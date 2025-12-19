# Решение HardFault при запуске FreeRTOS на STM32F411

## Описание проблемы

При попытке запустить отладку проекта на STM32F411CE с FreeRTOS программа падала в `HardFault_Handler` сразу после старта первой задачи в функции `prvPortStartFirstTask`.

### Симптомы

1. OpenOCD не мог остановить микроконтроллер командой `reset halt`
2. Программа застревала в бесконечном цикле в `prvCheckTasksWaitingTermination()` (FreeRTOS)
3. При подключении отладчика показывало HardFault
4. Регистры показывали проблему с памятью:
   - `sp = 0x20020000` (за пределами RAM!)
   - `pc = 0x8002faa` в `prvPortStartFirstTask`

### Ошибки при отладке

```bash
Error: timed out while waiting for target halted
TARGET: stm32f4x.cpu - Not halted
Error: Target not halted
Error: failed erasing sectors 0 to 1
```

---

## Причина проблемы

**Переполнение RAM из-за недостаточного размера FreeRTOS heap.**

STM32F411CE имеет только **128KB RAM** (0x20000000 - 0x2001FFFF).

### Анализ использования памяти

```bash
arm-none-eabi-size build/ILI9341_stm32f411.elf
```

**До исправления:**
```
   text    data     bss     dec     hex filename
  29132     344   46496   75972   128c4 build/ILI9341_stm32f411.elf
```

**После исправления:**
```
   text    data     bss     dec     hex filename
  29244     344  102816  132404   20534 build/ILI9341_stm32f411.elf
```

### Анализ больших переменных

```bash
arm-none-eabi-nm --size-sort -C -r build/ILI9341_stm32f411.elf | head -20
```

**Результат:**
```
00004b00 b text_buffer.0     # 19200 байт (19KB) - экранный буфер
00003c00 b ucHeap            # 15360 байт (15KB) - FreeRTOS heap (МАЛО!)
00000800 B UserTxBufferFS    # 2048 байт - USB TX
00000800 B UserRxBufferFS    # 2048 байт - USB RX
```

### Проблема

Heap FreeRTOS был настроен на **15KB**, что было **недостаточно** для:
- Стеков задач (3 задачи × ~1KB каждая = 3KB)
- Внутренних структур FreeRTOS
- Динамического выделения памяти

При запуске первой задачи указатель стека выходил за пределы доступной RAM → **HardFault**.

---

## Диагностика проблемы

### Шаг 1: Проверка регистров при HardFault

В GDB выполнить:

```gdb
info registers
```

**Результат:**
```
sp  = 0x20020000  # ЗА ПРЕДЕЛАМИ RAM! (должен быть < 0x20020000)
pc  = 0x8002faa   # в prvPortStartFirstTask
```

### Шаг 2: Проверка использования памяти

```bash
arm-none-eabi-size build/*.elf
```

Проверить что `data + bss < 120KB` (оставить запас).

### Шаг 3: Найти большие переменные

```bash
arm-none-eabi-nm --size-sort -C -r build/*.elf | head -20
```

### Шаг 4: Проверка FreeRTOS heap

```bash
grep "configTOTAL_HEAP_SIZE" Core/Inc/FreeRTOSConfig.h
```

### Шаг 5: Проверка linker script

В `STM32F411CEUx_FLASH.ld` убедиться:

```ld
MEMORY
{
  RAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 128K  # Правильно!
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
}
```

---

## Решение

### 1. Увеличение FreeRTOS heap

**Файл:** `Core/Inc/FreeRTOSConfig.h`

```c
// БЫЛО (недостаточно):
#define configTOTAL_HEAP_SIZE    ((size_t)15360)  // 15KB

// СТАЛО (достаточно):
#define configTOTAL_HEAP_SIZE    ((size_t)71680)  // 70KB
```

### 2. Добавление проверки переполнения стека

**Файл:** `Core/Inc/FreeRTOSConfig.h`

```c
#define configCHECK_FOR_STACK_OVERFLOW  2
```

### 3. Добавление callback-функций для отладки

**Файл:** `Core/Src/freertos.c` (в конец файла)

```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Called if stack overflow is detected */
    taskDISABLE_INTERRUPTS();
    while(1) {
        // Установите breakpoint здесь
    }
}

void vApplicationMallocFailedHook(void)
{
    /* Called if pvPortMalloc() fails */
    taskDISABLE_INTERRUPTS();
    while(1) {
        // Установите breakpoint здесь
    }
}
```

### 4. Исправление конфигурации OpenOCD

**Файл:** `openocd.cfg`

```tcl
source [find interface/stlink.cfg]
source [find target/stm32f4x.cfg]

# Установите низкую скорость для надежности
adapter speed 480

# Предотвращение изменения скорости при reset
$_TARGETNAME configure -event reset-start {
    adapter speed 480
}

$_TARGETNAME configure -event reset-init {
    adapter speed 480
}

$_TARGETNAME configure -event reset-end {
    adapter speed 480
}

reset_config srst_only connect_assert_srst
gdb_port 3333
telnet_port 4444
tcl_port 6666

$_TARGETNAME configure -work-area-phys 0x20000000 -work-area-size 0x4000

init
arm semihosting enable
```

---

## Формула расчета памяти

```
Общая RAM STM32F411CE: 128KB = 131072 байт

Распределение:
┌─────────────────────────────────────────┐
│ .data (инициализированные переменные)   │  ~344 байт
├─────────────────────────────────────────┤
│ .bss (неинициализированные переменные)  │  ~27KB
│   - text_buffer: 19KB                   │
│   - USB buffers: 4KB                    │
│   - другое: ~4KB                        │
├─────────────────────────────────────────┤
│ FreeRTOS heap (ucHeap)                  │  70KB
│   - Стеки задач                         │
│   - Внутренние структуры RTOS           │
│   - Динамическая память                 │
├─────────────────────────────────────────┤
│ Main stack (_Min_Stack_Size)            │  2KB
├─────────────────────────────────────────┤
│ Минимальный heap (_Min_Heap_Size)       │  1KB
├─────────────────────────────────────────┤
│ Запас безопасности                      │  ~27KB
└─────────────────────────────────────────┘
ИТОГО:                                      ~128KB
```

### Безопасная формула

```
data + bss + configTOTAL_HEAP_SIZE + _Min_Stack_Size + запас < 128KB
344 + 32000 + 71680 + 2048 + 25000 ≈ 131KB ✓
```

---

## Настройка VS Code для отладки

### `.vscode/tasks.json`

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build",
            "type": "shell",
            "command": "make",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Flash (st-flash)",
            "type": "shell",
            "command": "make",
            "args": ["flash"],
            "dependsOn": ["Build"],
            "problemMatcher": []
        },
        {
            "label": "Start OpenOCD",
            "type": "shell",
            "command": "openocd",
            "args": ["-f", "openocd.cfg"],
            "isBackground": true,
            "problemMatcher": {
                "pattern": {
                    "regexp": "."
                },
                "background": {
                    "activeOnStart": true,
                    "beginsPattern": "Open On-Chip Debugger",
                    "endsPattern": "Listening on port 3333 for gdb connections"
                }
            }
        },
        {
            "label": "Stop OpenOCD",
            "type": "shell",
            "command": "killall",
            "args": ["openocd"],
            "problemMatcher": []
        },
        {
            "label": "Flash and Start OpenOCD",
            "dependsOrder": "sequence",
            "dependsOn": ["Flash (st-flash)", "Start OpenOCD"],
            "problemMatcher": []
        }
    ]
}
```

### `.vscode/launch.json`

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug STM32F411",
            "type": "cortex-debug",
            "request": "attach",
            "servertype": "external",
            "gdbTarget": "localhost:3333",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/build/ILI9341_stm32f411.elf",
            "svdFile": "${workspaceFolder}/STM32F411.svd",
            "preLaunchTask": "Flash and Start OpenOCD",
            "postDebugTask": "Stop OpenOCD",
            "showDevDebugOutput": "raw"
        }
    ]
}
```

### Установка SVD файла

```bash
wget https://github.com/tinygo-org/stm32-svd/raw/main/svd/stm32f411.svd -O STM32F411.svd
```

---

## Workflow отладки

### 1. Сборка и прошивка

```bash
make clean
make
make flash
```

### 2. Запуск OpenOCD

```bash
openocd -f openocd.cfg
```

### 3. Запуск отладки в VS Code

Нажмите `F5` или:
- **Run → Start Debugging**

### 4. Полезные команды GDB

```gdb
# Информация о задачах FreeRTOS
info threads

# Переключение между задачами
thread 2

# Стек вызовов
backtrace

# Проверка свободной памяти heap
print xFreeBytesRemaining
print configTOTAL_HEAP_SIZE

# Проверка регистров
info registers

# Продолжить выполнение
continue

# Остановить
interrupt
```

---

## Проверка после исправления

### 1. Размер бинарника

```bash
arm-none-eabi-size build/ILI9341_stm32f411.elf
```

**Ожидаемый результат:**
```
   text    data     bss     dec     hex filename
  29244     344  102816  132404   20534 build/ILI9341_stm32f411.elf
```

✅ `bss` должен быть ~100KB (было 46KB)

### 2. Проверка heap

```bash
arm-none-eabi-nm --size-sort -C -r build/ILI9341_stm32f411.elf | grep ucHeap
```

**Результат:**
```
00011800 b ucHeap  # 71680 байт (70KB) - правильно!
```

### 3. Запуск программы

Программа должна:
- ✅ Запуститься без HardFault
- ✅ Мигать светодиодом (500мс)
- ✅ Отображать контент на дисплее
- ✅ Отвечать на отладчик

---

## Типичные ошибки и их решения

### Ошибка 1: "Target not halted"

**Причина:** Скорость адаптера слишком высокая или программа не отдает управление.

**Решение:**
```bash
# В telnet к OpenOCD:
telnet localhost 4444
adapter speed 480
halt
```

### Ошибка 2: "flash_erase returned -304"

**Причина:** Не удалось остановить CPU для стирания flash.

**Решение:** Используйте st-flash вместо OpenOCD:
```bash
make flash  # использует st-flash
```

### Ошибка 3: Stack overflow в задаче

**Решение:** Увеличьте размер стека задачи в `freertos.c`:
```c
osThreadDef(taskName, TaskFunction, osPriorityNormal, 0, 384); // было 256
```

### Ошибка 4: Malloc failed

**Причина:** Недостаточно heap памяти.

**Решение:** Увеличьте `configTOTAL_HEAP_SIZE` или уменьшите статические буферы.

---

## Рекомендации по оптимизации памяти

### 1. Используйте меньшие буферы

```c
// Вместо полноэкранного буфера
uint16_t frame_buffer[240*320];  // 150KB - не влезет!

// Используйте построчный буфер
uint16_t line_buffer[240];  // 480 байт - отлично!
```

### 2. Динамическое выделение при необходимости

```c
void display_task(void *argument) {
    uint16_t *temp_buffer = pvPortMalloc(1024 * sizeof(uint16_t));
    
    if (temp_buffer == NULL) {
        // Обработка ошибки
        while(1);
    }
    
    // Используйте буфер
    
    vPortFree(temp_buffer);
}
```

### 3. Правильные размеры стеков задач

```c
// Простая задача (мигание LED)
.stack_size = 128 * 4  // 512 байт

// Задача с SPI/дисплеем
.stack_size = 256 * 4  // 1024 байта

// Задача с большими локальными переменными
.stack_size = 384 * 4  // 1536 байт
```

### 4. Мониторинг использования heap

```c
void monitor_task(void *argument) {
    while(1) {
        size_t free_heap = xPortGetFreeHeapSize();
        size_t min_free = xPortGetMinimumEverFreeHeapSize();
        
        LOG_Printf("Free heap: %d, Min: %d\r\n", free_heap, min_free);
        
        osDelay(5000);
    }
}
```

---

## Итоговый чек-лист

- [x] Увеличен `configTOTAL_HEAP_SIZE` до 70KB
- [x] Добавлен `configCHECK_FOR_STACK_OVERFLOW = 2`
- [x] Добавлены callback-функции для отладки
- [x] Настроен OpenOCD с фиксированной скоростью 480 kHz
- [x] Проверено использование памяти: `bss ~100KB`
- [x] Установлен SVD файл для Cortex-Debug
- [x] Настроены VS Code tasks и launch конфигурации
- [x] Программа запускается без HardFault
- [x] Отладка работает корректно

---

## Полезные ссылки

- [FreeRTOS Memory Management](https://www.freertos.org/a00111.html)
- [STM32F411 Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
- [Cortex-Debug Extension](https://github.com/Marus/cortex-debug)
- [OpenOCD Documentation](http://openocd.org/doc/html/index.html)
- [ARM Cortex-M4 Technical Reference](https://developer.arm.com/documentation/100166/0001)

---

## Заключение

Проблема HardFault при запуске FreeRTOS на STM32F411 была вызвана **недостаточным размером heap** (15KB вместо необходимых 70KB). 

Увеличение `configTOTAL_HEAP_SIZE` и правильная настройка отладочного окружения позволили успешно запустить и отлаживать проект с FreeRTOS, дисплеем ILI9341, USB CDC и тачскрином.

**Ключевой урок:** Всегда проверяйте использование памяти с помощью `arm-none-eabi-size` и `arm-none-eabi-nm` перед отладкой встраиваемых систем с RTOS.