# Исправленные инструкции по запуску отладки STM32F411

## Описание проблемы на основе последних логов

### Новые ошибки в логах
```
ERROR: last sector must be <= 7
timed out while waiting for target halted
TARGET: stm32f4x.cpu - Not halted
```

### Анализ проблемы
1. **Неправильная нумерация секторов flash**: STM32F411 имеет только **8 секторов** (0-7), а не 128
2. **Цель не останавливается**: `timed out while waiting for target halted` - процессор не может быть корректно остановлен даже после `reset halt`
3. **HardFault persists**: Проблема с HardFault не решена предыдущими командами

### Структура flash STM32F411CE (512KB)
- Сектор 0: 16KB (0x08000000 - 0x08003FFF)
- Сектор 1: 16KB (0x08004000 - 0x08007FFF)
- Сектор 2: 16KB (0x08008000 - 0x0800BFFF)
- Сектор 3: 16KB (0x0800C000 - 0x0800FFFF)
- Сектор 4: 64KB (0x08010000 - 0x0801FFFF)
- Сектор 5: 128KB (0x08020000 - 0x0803FFFF)
- Сектор 6: 128KB (0x08040000 - 0x0805FFFF)
- Сектор 7: 128KB (0x08060000 - 0x0807FFFF)

## Исправленная последовательность команд

### Шаг 1: Запустить OpenOCD
```bash
cd /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411
openocd -f openocd.cfg
```

### Шаг 2: Подключиться к telnet консоли
```bash
telnet localhost 4444
```

### Шаг 3: Попробовать принудительный сброс (в telnet)
```
> halt
> reset
> halt
```

Если не помогает, попробуйте:
```
> reset_config none
> reset halt
```

### Шаг 4: Очистить flash секторами (в telnet)
```
> flash erase_sector 0 0 7
```

**Важно:** Команда стирает все сектора 0-7, что очистит всю flash память.

### Шаг 5: Проверить статус (в telnet)
```
> halt
> info
```

### Шаг 6: Выйти из telnet
```
> exit
```

### Шаг 7: Остановить OpenOCD (Ctrl+C в первом терминале)

### Шаг 8: Перезапустить OpenOCD
```bash
openocd -f openocd.cfg
```

### Шаг 9: Загрузить прошивку через GDB
```bash
arm-none-eabi-gdb build/ILI9341_stm32f411.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "load" -ex "monitor reset halt" -ex "break main" -ex "continue"
```

## Альтернативный способ: Использование st-flash

Да, вы можете очистить память через st-flash. Это простой и надежный способ, особенно когда OpenOCD имеет проблемы с коммуникацией.

### Установка st-flash
```bash
sudo apt update
sudo apt install stlink-tools
```

### Очистка flash памяти через st-flash
```bash
# Полная очистка всей flash памяти
st-flash erase
```

**Ожидаемый вывод:**
```
st-flash 1.7.0-136-g2fc4
2023-10-05T15:57:20 INFO common.c: Flash erased.
```

### Полная последовательность с st-flash
```bash
# 1. Очистить flash
st-flash erase

# 2. Записать прошивку (если ELF, сначала конвертировать в BIN)
arm-none-eabi-objcopy -O binary build/ILI9341_stm32f411.elf build/ILI9341_stm32f411.bin
st-flash write build/ILI9341_stm32f411.bin 0x8000000

# 3. Запустить OpenOCD для отладки
openocd -f openocd.cfg &
sleep 2

# 4. Подключить GDB
arm-none-eabi-gdb build/ILI9341_stm32f411.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "break main" -ex "continue"
```

### Преимущества st-flash
- **Надежность**: Простой протокол, меньше проблем с RST линией
- **Скорость**: Быстрее для операций стирания/записи
- **Совместимость**: Хорошо работает даже при проблемах с OpenOCD

### Недостатки st-flash
- **Только запись/стирание**: Нет интерактивной отладки (используйте с OpenOCD для отладки)
- **Меньше диагностики**: Меньше информации об ошибках по сравнению с OpenOCD

## Диагностика проблем с RST линией

Если `reset halt` не работает, проверьте:

1. **Подключение RST**: ST-Link pin 3 ↔ STM32F411 pin 7 (NRST)
2. **Альтернативная конфигурация сброса** в openocd.cfg:
   ```cfg
   reset_config trst_and_srst separate connect_assert_srst
   ```
   Или:
   ```cfg
   reset_config none separate
   ```

3. **Тестирование RST**:
   - Отключите RST линию
   - Добавьте в openocd.cfg: `reset_config none`
   - Попробуйте отладку без аппаратного сброса

## Ожидаемые результаты после исправления

```
Info : [stm32f4x.cpu] halted due to debug-request, current mode: Thread
Info : flash erase finished
Info : Padding image section 1 at 0x08000000 with 0xff
Info : Padding image section 1 at 0x08000000 with 0xff
Info : flash write algorithm 0 (256 bytes) sent
Info : flash write algorithm 1 (256 bytes) sent
Info : flash write algorithm 2 (256 bytes) sent
Info : flash write algorithm 3 (256 bytes) sent
Info : flash write finished
```

## Следующие шаги при неудаче

1. **Циклическое питание**: Полностью отключить питание STM32F411 на 10-15 секунд
2. **Проверка SWD линий**:
   - SWCLK: ST-Link pin 2 ↔ STM32F411 PA14
   - SWDIO: ST-Link pin 4 ↔ STM32F411 PA13
   - GND: ST-Link pin 6 ↔ STM32F411 GND
3. **Тестирование на другом STM32** (если возможно)
4. **Обновление OpenOCD**: `sudo apt update && sudo apt upgrade openocd`
