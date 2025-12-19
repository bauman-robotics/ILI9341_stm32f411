# Анализ проблемы: HardFault в FreeRTOS

## Описание ситуации

После успешной очистки flash через `st-flash erase` и записи прошивки, при запуске отладки наблюдается следующее:

### Логи GDB
```
Remote debugging using localhost:3333
0x0800327e in prvPortStartFirstTask () at Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c:267
timed out while waiting for target halted
TARGET: stm32f4x.cpu - Not halted
Protocol error with Rcmd
Breakpoint 1 at 0x80003d0: file Core/Src/main.c, line 69.
```

### Логи OpenOCD (повторный запуск)
```
[stm32f4x.cpu] halted due to debug-request, current mode: Handler HardFault
xPSR: 0x61000003 pc: 0x0800327e msp: 0x20020000, semihosting
Error: timed out while waiting for target halted
```

## В чем проблема

### 1. HardFault во время выполнения FreeRTOS
Процессор находится в состоянии **HardFault** по адресу `0x0800327e`, который соответствует функции `prvPortStartFirstTask()` в коде FreeRTOS (port.c:267).

### 2. Проблемы с остановкой цели
- `timed out while waiting for target halted` - OpenOCD/GDB не могут корректно остановить процессор
- `Protocol error with Rcmd` - ошибки протокола коммуникации
- Процессор продолжает выполняться несмотря на команды остановки

### 3. FreeRTOS не запускается корректно
Функция `prvPortStartFirstTask()` вызывается при запуске первой задачи FreeRTOS, но происходит HardFault, что указывает на:
- Повреждение стека или памяти
- Ошибки в конфигурации FreeRTOS
- Проблемы с аппаратной инициализацией

## Возможные причины HardFault в prvPortStartFirstTask()

HardFault в `prvPortStartFirstTask()` обычно связан с проблемами в момент переключения контекста при запуске первой задачи FreeRTOS. Эта функция выполняет критические операции:

### 1. Настройка MSP/PSP (Stack Pointers)
```c
// В port.c
__asm volatile(
    "ldr r0, =0xE000ED08 \n"  /* Вектор таблицы */
    "ldr r0, [r0] \n"
    "ldr r0, [r0] \n"
    "msr msp, r0 \n"  /* Установка MSP */
    "cpsie i \n"       /* Включение прерываний */
    "svc 0 \n"         /* Переход в задачу */
);
```

**Возможные проблемы:**
- **Некорректная векторная таблица** - VTOR указывает на неправильный адрес
- **Поврежденный стек** - MSP получает недопустимый адрес
- **Переполнение стека** - недостаточный размер стека для задачи

### 2. Конфигурация прерываний (NVIC)
- **SysTick не настроен** - FreeRTOS требует SysTick для планировщика
- **PendSV не настроен** - используется для переключения задач
- **SVC не обрабатывается** - системный вызов для запуска задач

### 3. Конфигурация памяти
- **Неправильный heap** - FreeRTOS heap поврежден
- **Переполнение памяти** - выход за границы выделенной памяти
- **Конфликт областей памяти** - пересечение стеков или heap

### Аппаратные проблемы
1. **Нестабильное питание** - недостаточное или шумное питание 3.3V
2. **Проблемы с RST линией** - нестабильный сброс приводит к некорректному запуску
3. **Повреждение STM32** - аппаратный брак микроконтроллера

### Программные проблемы
1. **Ошибки в коде инициализации** - неправильная настройка периферии перед запуском FreeRTOS
2. **Конфликт прерываний** - NVIC не настроен корректно
3. **Проблемы с памятью** - heap/stack overflow
4. **Некорректная clock конфигурация** - неправильные настройки RCC

### Проблемы с отладкой
1. **OpenOCD конфигурация** - неправильные настройки reset_config
2. **ST-Link проблемы** - нестабильная связь
3. **Конфликт процессов** - несколько экземпляров OpenOCD

## Использование CDC порта для логов

Да, подключение **USB CDC порта** крайне рекомендуется для получения отладочных логов! Это позволит увидеть, что происходит в прошивке до HardFault.

### Настройка логирования через USB CDC

1. **Подключить USB разъем STM32 к компьютеру**
   - STM32F411 должен enumerаться как виртуальный COM порт

2. **Найти COM порт:**
   ```bash
   # Linux
   dmesg | grep ttyACM
   # Или
   ls /dev/ttyACM*
   ```

3. **Открыть serial monitor:**
   ```bash
   # Использовать screen, minicom или другой терминал
   screen /dev/ttyACM0 115200

   # Или с помощью Python
   python3 -c "import serial; s=serial.Serial('/dev/ttyACM0', 115200); [print(s.readline().decode('utf-8', errors='ignore'), end='') for _ in range(100)]"
   ```

4. **Добавить логи в код:**
   ```c
   #include "usbd_cdc_if.h"  // Если используется USB CDC

   int main(void) {
       HAL_Init();
       SystemClock_Config();

       // Инициализация USB (должна быть в коде)
       MX_USB_DEVICE_Init();

       printf("System initialized\n");
       printf("Starting FreeRTOS configuration...\n");

       // Конфигурация FreeRTOS
       printf("Creating tasks...\n");

       printf("Starting scheduler...\n");
       osKernelStart();

       while (1);
   }
   ```

### Преимущества CDC логов
- **Видеть выполнение до HardFault** - логи покажут, на каком этапе происходит сбой
- **Мониторить FreeRTOS** - сообщения о создании задач, очереди и т.д.
- **Диагностировать инициализацию** - проверка корректности настройки периферии

### Если CDC не работает
1. **Проверить USB дескрипторы** - правильная конфигурация USB_DEVICE
2. **Время инициализации** - USB может требовать задержку перед работой
3. **Альтернативный вывод** - использовать UART для логов (USART1/2/3)

## Что делать: Пошаговая диагностика

### Шаг 1: Проверить аппаратную часть
```bash
# Проверить напряжение питания
# Отключить/подключить RST линию
# Попробовать другой USB порт для ST-Link
# Проверить качество соединений SWD
```

### Шаг 2: Изменить конфигурацию OpenOCD
Временно изменить `openocd.cfg`:
```cfg
# Изменить на:
reset_config none separate
# Вместо:
reset_config srst_only connect_assert_srst
```

### Шаг 3: Запустить без FreeRTOS (тестовый код)
Создать минимальную прошивку без FreeRTOS для проверки:
```c
int main(void) {
    // Минимальная инициализация
    HAL_Init();

    while (1) {
        // Пустой цикл
    }
}
```

### Шаг 4: Использовать отладку с немедленной остановкой
```bash
# Запустить OpenOCD
openocd -f openocd.cfg

# В другом терминале
arm-none-eabi-gdb build/ILI9341_stm32f411.elf -ex "target extended-remote localhost:3333" -ex "monitor halt"
```

### Шаг 5: Анализ HardFault причины
В GDB при HardFault:
```gdb
(gdb) info registers
(gdb) x/16xw $sp  # Посмотреть стек
(gdb) disassemble  # Дизассемблировать вокруг PC
```

### Шаг 6: Логирование перед FreeRTOS
Добавить отладочный вывод перед запуском FreeRTOS:
```c
int main(void) {
    // Инициализация HAL
    HAL_Init();

    // Инициализация clock
    SystemClock_Config();

    // Отладочный вывод
    printf("Starting FreeRTOS...\n");

    // Запуск FreeRTOS
    osKernelStart();

    // Сюда никогда не дойдет
    while (1);
}
```

## Рекомендации

1. **Начать с простого**: Убрать FreeRTOS временно, запустить голый HAL
2. **Проверить инициализацию**: Убедиться, что все периферия настроена до запуска ОС
3. **Мониторить HardFault**: Использовать GDB для анализа причины
4. **Попробовать JTAG вместо SWD**: Изменить интерфейс в openocd.cfg на jtag
5. **Обновить toolchain**: Убедиться, что все компоненты свежие

## Следующие шаги

1. Создать тестовую прошивку без FreeRTOS
2. Проверить работу с минимальным кодом
3. Постепенно добавлять функциональность до выявления проблемы
4. Проанализировать конфигурацию FreeRTOS

HardFault в `prvPortStartFirstTask()` часто связан с неправильной настройкой MSP/PSP или проблемами с прерываниями.
