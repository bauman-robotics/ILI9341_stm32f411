# Анализ конфигурации памяти FreeRTOS и рекомендации

## Введение

Этот документ содержит полный анализ текущей конфигурации памяти в проекте STM32F411 с FreeRTOS, включая анализ использования стека задач, размера кучи и общих рекомендаций по оптимизации.

## Текущая конфигурация FreeRTOS

### Основные параметры (FreeRTOSConfig.h)

```c
#define configTOTAL_HEAP_SIZE                    ((size_t)15360)     // 15 КБ
#define configMINIMAL_STACK_SIZE                 ((uint16_t)128)     // 128 слов = 512 байт
#define configMAX_PRIORITIES                     (7)
#define configUSE_STATIC_ALLOCATION              1
#define configUSE_DYNAMIC_ALLOCATION             1
```

### Аппаратные ограничения

- **Общий RAM**: 128 КБ (STM32F411CEU6)
- **Linker script**: `_Min_Heap_Size = 0x400` (1024 байт), `_Min_Stack_Size = 0x800` (2048 байт)

## Анализ созданных задач

### Статическое выделение памяти для задач

#### 1. Idle Task (системная задача простоя)
```c
// Статическое выделение в freertos.c
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE]; // 128 слов = 512 байт

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
```

#### 2. Default Task (основная задача приложения)
```c
osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
```
- **Стек**: 128 слов = 512 байт
- **Приоритет**: `osPriorityNormal`
- **Функциональность**: Инициализация USB, дисплея, управление подсветкой, основной цикл приложения

#### 3. Touch Task (обработка touchscreen)
```c
osThreadDef(touchTask, TouchTask, osPriorityNormal, 0, 512);
```
- **Стек**: 512 слов = 2048 байт
- **Приоритет**: `osPriorityNormal`
- **Функциональность**: Инициализация touchscreen, чтение данных касания, обработка калибровки

#### 4. Calibration Task (калибровка touchscreen)
```c
osThreadDef(calibrationTask, CalibrationTask, CALIBRATION_TASK_PRIORITY, 0, CALIBRATION_TASK_STACK_SIZE);
#define CALIBRATION_TASK_STACK_SIZE 512  // 512 слов = 2048 байт
#define CALIBRATION_TASK_PRIORITY osPriorityNormal
```
- **Стек**: 512 слов = 2048 байт
- **Приоритет**: `osPriorityNormal`
- **Функциональность**: Процесс калибровки touchscreen, завершается после выполнения

#### 5. Live Packet Task (вывод пакетов) - ОТКЛЮЧЕНА
```c
#define ENABLE_LIVE_PACKET_TASK 0  // Отключена в config.h
```

## Расчет текущего использования памяти

### Стек задач
- Idle Task: 512 байт
- Default Task: 512 байт
- Touch Task: 2048 байт
- Calibration Task: 2048 байт
- **Итого стек задач**: 5120 байт

### Куча (Heap)
- Размер: 15360 байт (15 КБ)
- Используется для: динамических аллокаций FreeRTOS (очереди, семафоры, etc.)

### Системный стек (MSP)
- Linker: `_Min_Stack_Size = 0x800` (2048 байт)
- Используется для: прерываний, системных вызовов

### Общее использование
- **Выделенная память**: 5120 + 15360 = 20480 байт
- **Свободный RAM**: 128 КБ - 20480 байт = ~107 КБ
- **Запас**: Достаточный для расширений

## Анализ потенциальных проблем

### 1. Недостаточный стек Idle Task
**Проблема**: 512 байт может быть недостаточно для некоторых операций HAL.
**Риск**: Stack overflow в системной задаче.

### 2. Минимальный стек Default Task
**Проблема**: 512 байт для основной задачи с инициализацией USB и дисплея.
**Риск**: Переполнение при вложенных вызовах функций.

### 3. Размер кучи
**Проблема**: 15 КБ может быть недостаточно при активном использовании USB и touchscreen.
**Риск**: Ошибка `pvPortMalloc()` при нехватке памяти.

### 4. Отсутствие мониторинга
**Проблема**: Нет контроля за использованием стека и кучи в runtime.
**Риск**: Скрытые проблемы с памятью.

## Рекомендации по оптимизации

### 1. Увеличение размера кучи
```c
// FreeRTOSConfig.h - изменить на 20 КБ
#define configTOTAL_HEAP_SIZE ((size_t)20480)
```
**Обоснование**: Дополнительный запас для динамических аллокаций USB, touchscreen и будущих расширений.

### 2. Увеличение минимального размера стека
```c
// FreeRTOSConfig.h - изменить на 256 слов (1 КБ)
#define configMINIMAL_STACK_SIZE ((uint16_t)256)
```
**Обоснование**: Idle task требует больше стека для HAL функций.

### 3. Оптимизация стеков задач
```c
// freertos.c - MX_FREERTOS_Init()

// Default Task: увеличить до 256 слов
osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);

// Touch Task: оставить 512 слов (достаточно)
// Calibration Task: оставить 512 слов (достаточно)
```

### 4. Добавление мониторинга памяти
```c
// В каждую задачу добавить проверку стека
UBaseType_t stackWaterMark = uxTaskGetStackHighWaterMark(NULL);
LOG_Printf("Task started, stack high water mark: %d words\n", stackWaterMark);

// Периодическая проверка
void vTaskMonitorMemory(void *pvParameters) {
    for (;;) {
        // Проверка кучи
        size_t freeHeap = xPortGetFreeHeapSize();
        size_t minFreeHeap = xPortGetMinimumEverFreeHeapSize();

        // Проверка стеков задач
        // ... код для проверки всех задач

        vTaskDelay(pdMS_TO_TICKS(5000)); // Каждые 5 секунд
    }
}
```

### 5. Оптимизация Linker Script
```ld
// STM32F411CEUx_FLASH.ld
_Min_Heap_Size = 0x800;      /* 2048 байт вместо 1024 */
_Min_Stack_Size = 0x1000;    /* 4096 байт вместо 2048 для MSP */
```

### 6. Hooks для отладки памяти
```c
// freertos.c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    LOG_Printf("STACK OVERFLOW in task: %s\n", pcTaskName);
    // Дополнительная обработка
}

void vApplicationMallocFailedHook(void) {
    LOG_Printf("MALLOC FAILED - Out of heap memory!\n");
    // Дополнительная обработка
}
```

## Таблица рекомендуемых изменений

| Параметр | Текущее значение | Рекомендуемое значение | Обоснование |
|----------|------------------|------------------------|-------------|
| `configTOTAL_HEAP_SIZE` | 15360 байт | 20480 байт | Запас для динамических аллокаций |
| `configMINIMAL_STACK_SIZE` | 128 слов | 256 слов | Безопасность для Idle task |
| Default Task stack | 128 слов | 256 слов | Инициализация USB/дисплея |
| `_Min_Heap_Size` | 0x400 | 0x800 | Linker check |
| `_Min_Stack_Size` | 0x800 | 0x1000 | MSP stack |

## Мониторинг после изменений

### 1. Проверка стека задач
```c
UBaseType_t waterMark = uxTaskGetStackHighWaterMark(xTaskHandle);
LOG_Printf("Stack usage: %d/%d words (%d%%)\n",
           configSTACK_SIZE - waterMark, configSTACK_SIZE,
           ((configSTACK_SIZE - waterMark) * 100) / configSTACK_SIZE);
```

### 2. Проверка кучи
```c
size_t free = xPortGetFreeHeapSize();
size_t minEver = xPortGetMinimumEverFreeHeapSize();
LOG_Printf("Heap: free=%d, minEver=%d, used=%d\n",
           free, minEver, configTOTAL_HEAP_SIZE - free);
```

### 3. Анализ fragmentation
Регулярно проверять, не фрагментирована ли куча из-за частых аллокаций/деаллокаций.

## Заключение

Текущая конфигурация памяти близка к оптимальной, но имеет потенциальные проблемы с минимальными размерами стеков. Рекомендуемые изменения повысят надежность системы и предотвратят HardFault из-за переполнения стека или нехватки кучи.

**Критические изменения:**
1. Увеличить `configTOTAL_HEAP_SIZE` до 20480
2. Увеличить `configMINIMAL_STACK_SIZE` до 256
3. Увеличить стек Default Task до 256 слов

**Мониторинг:**
Добавить функции отслеживания использования памяти для своевременного обнаружения проблем.
