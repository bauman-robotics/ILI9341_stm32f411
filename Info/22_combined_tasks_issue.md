# 22. Анализ проблемы одновременного запуска TASK_ALPHABET_DISPLAY и ENABLE_LIVE_PACKET_TASK

## Описание проблемы

При одновременном включении `TASK_ALPHABET_DISPLAY = 1` и `ENABLE_LIVE_PACKET_TASK = 1` проект не запускается корректно. Это происходит несмотря на то, что каждая задача по отдельности работает нормально.

## Текущая конфигурация

```c
#define TASK_ALPHABET_DISPLAY 1        // Включена задача отображения алфавита
#define ENABLE_LIVE_PACKET_TASK 1      // Включена задача вывода live packets
```

## Анализ возможных причин

### 1. **Конфликт ресурсов**
- Обе задачи используют USB для логирования
- LivePacketTask начинает работу через 5 секунд, но может конфликтовать с основной задачей
- Возможное переполнение буферов USB CDC

### 2. **Проблемы с памятью**
- STM32F411 имеет ограниченную RAM (192KB)
- Каждая задача занимает стек (128 байт для LivePacketTask, 128 байт для основной задачи)
- Дополнительная нагрузка на heap для динамических операций

### 3. **Временные конфликты**
- LivePacketTask ждёт 5 секунд, но основная задача уже работает
- Возможны race conditions при доступе к ресурсам

### 4. **Проблемы с приоритетами**
- LivePacketTask имеет `osPriorityBelowNormal`
- Основная задача имеет `osPriorityNormal`
- Но LivePacketTask может всё равно влиять на производительность

### 5. **USB CDC инициализация**
- Двойная нагрузка на USB стек
- Возможные проблемы с дескрипторами или конфигурацией USB

## Анализ кода StartDefaultTask

При `TASK_ALPHABET_DISPLAY = 1` задача выполняет статическое отображение текста:

```c
// Font1 (5x7) - Uppercase A-Z
ILI9341_DrawString(10, 10, upper_az, ILI9341_WHITE, ILI9341_BLACK, 1, Font1);

// Font1 (5x7) - Lowercase a-z
ILI9341_DrawString(10, 25, lower_az, ILI9341_CYAN, ILI9341_BLACK, 1, Font1);

// Font1_2x (10x14) - Uppercase A-Z
ILI9341_DrawStringLarge(10, 45, upper_az, ILI9341_YELLOW, ILI9341_BLACK);

// Font1_2x (10x14) - Lowercase a-z
ILI9341_DrawStringLarge(10, 75, lower_az, ILI9341_RED, ILI9341_BLACK);
```

После этого задача входит в бесконечный цикл с морганием LED.

## Анализ LivePacketTask

```c
void LivePacketTask(void const * argument) {
    // Ждёт 5 секунд
    osDelay(5000);

    // Начинает вывод каждые 3 секунды
    while (1) {
        LOG_SendString("live packet\r\n");
        osDelay(3000);
    }
}
```

## Возможные решения

### 1. **Увеличить задержку LivePacketTask**
```c
osDelay(10000); // 10 секунд вместо 5
```

### 2. **Уменьшить частоту вывода**
```c
osDelay(10000); // Каждые 10 секунд вместо 3
```

### 3. **Использовать условную компиляцию**
```c
#if !TASK_ALPHABET_DISPLAY
// Запуск LivePacketTask только если не включена задача алфавита
#endif
```

### 4. **Добавить мьютекс для USB**
Создать мьютекс для синхронизации доступа к USB логированию.

### 5. **Отключить логирование в LivePacketTask**
```c
// Вместо LOG_SendString использовать что-то другое
```

## Рекомендации для диагностики

1. **Тестировать по отдельности**: Каждая задача работает по отдельности
2. **Проверить логи**: Если проект запускается частично, проверить USB логи
3. **Мониторить ресурсы**: Проверить использование RAM и CPU
4. **Тестировать задержки**: Попробовать разные значения задержек

## Финальное решение

**Коренная причина:** Логирование при создании задачи в `MX_FREERTOS_Init()` конфликтовало с USB инициализацией.

**Решение:** Убрать все `LOG_SendString()` вызовы из функции `MX_FREERTOS_Init()` при создании задач.

**Изменения в коде:**
```c
// ДО (вызывало проблемы):
LOG_SendString("FREERTOS: Creating LivePacketTask...\r\n");
osThreadDef(livePacketTask, LivePacketTask, osPriorityBelowNormal, 0, 256);
osThreadId livePacketTaskHandle = osThreadCreate(osThread(livePacketTask), NULL);

// ПОСЛЕ (работает корректно):
osThreadDef(livePacketTask, LivePacketTask, osPriorityBelowNormal, 0, 256);
osThreadId livePacketTaskHandle = osThreadCreate(osThread(livePacketTask), NULL);
```

**Дополнительные улучшения:**
- Увеличен размер стека задач до 256 байт
- LivePacketTask запускается через 30 секунд после старта
- Частота вывода "live packet" - каждые 30 секунд
- Приоритет LivePacketTask понижен до `osPriorityBelowNormal`

## Результат

Теперь обе задачи работают стабильно:
- `TASK_ALPHABET_DISPLAY = 1` отображает алфавит
- `ENABLE_LIVE_PACKET_TASK = 1` выводит пакеты без конфликтов
