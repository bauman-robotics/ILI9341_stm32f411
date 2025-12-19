# 21. Проблема с запуском других задач при ENABLE_LIVE_PACKET_TASK = 1

## Описание проблемы

При включении `#define ENABLE_LIVE_PACKET_TASK 1` в config.h другие задачи, такие как бегущая строка (TASK_SCROLLING_HELLO), перестают работать корректно.

## Анализ кода

### Структура задач в freertos.c

1. **StartDefaultTask** (главная задача):
   - Инициализирует USB, GPIO, дисплей
   - Выполняет одну из задач в зависимости от конфигурации:
     - TASK_ALPHABET_DISPLAY
     - TASK_SCROLLING_HELLO (бегущая строка)
     - TASK_QWERTY_KEYBOARD

2. **LivePacketTask** (при ENABLE_LIVE_PACKET_TASK = 1):
   - Создается параллельно с главной задачей
   - Выводит "live packet" каждую секунду через LOG_SendString
   - Приоритет: osPriorityNormal (такой же, как у главной задачи)

### Возможные причины проблемы

1. **Конфликт приоритетов**: Обе задачи имеют одинаковый приоритет (osPriorityNormal), что может приводить к starvation (голоданию) задач.

2. **Перегрузка системы логирования**: LivePacketTask постоянно использует LOG_SendString, что может влиять на USB CDC передачу и косвенно влиять на производительность других задач.

3. **Недостаток ресурсов**: STM32F411 имеет ограниченные ресурсы. Две задачи с одинаковым приоритетом могут конкурировать за процессорное время.

4. **Проблемы с USB**: LivePacketTask использует USB для вывода логов, что может конфликтовать с инициализацией USB в главной задаче.

## Текущее состояние конфигурации

```c
#define TASK_SCROLLING_HELLO 1         // Бегущая строка включена
#define ENABLE_LIVE_PACKET_TASK 1      // LivePacketTask включена
```

## Возможные решения

1. **Изменить приоритет LivePacketTask**:
   ```c
   osThreadDef(livePacketTask, LivePacketTask, osPriorityBelowNormal, 0, 128);
   ```

2. **Увеличить задержку в LivePacketTask**:
   ```c
   osDelay(2000); // Вместо 1000ms
   ```

3. **Отключить LivePacketTask для тестирования**:
   ```c
   #define ENABLE_LIVE_PACKET_TASK 0
   ```

4. **Добавить taskYIELD() в критических местах** главной задачи для предотвращения starvation.

## Рекомендация

Для диагностики проблемы временно установите:
```c
#define ENABLE_LIVE_PACKET_TASK 0
```

И проверьте, работает ли бегущая строка. Если да, то проблема в конфликте с LivePacketTask.
