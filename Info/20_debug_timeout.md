# 20. Проблема с таймаутом отладки и остановкой цели

## Описание проблемы

Во время сеанса отладки GDB успешно подключается к OpenOCD на localhost:3333, но сталкивается с несколькими проблемами:

1. **Несоответствие скорости**: Запрошено 2000 кГц, но доступно только 1800 кГц
2. **Цель не останавливается**: "timed out while waiting for target halted" и "TARGET: stm32f4x.cpu - Not halted"
3. **Ошибка протокола**: "Protocol error with Rcmd"
4. **Проблемы с точками останова**: Точки останова установлены, но управление целью проблематично

## Вывод GDB

```
Reading symbols from build/ILI9341_stm32f411.elf...
Remote debugging using localhost:3333
HAL_GetTick () at Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c:325
325       return uwTick;
Unable to match requested speed 2000 kHz, using 1800 kHz
Unable to match requested speed 2000 kHz, using 1800 kHz
timed out while waiting for target halted
TARGET: stm32f4x.cpu - Not halted
Protocol error with Rcmd
Breakpoint 1 at 0x80003d0: file Core/Src/main.c, line 69.
Note: automatically using hardware breakpoints for read-only addresses.
```

## Возможные причины

1. **Аппаратное подключение**: Проблемы с подключением ST-Link
2. **Питание**: Недостаточное питание платы STM32
3. **Конфигурация OpenOCD**: Проблемы с openocd.cfg
4. **Состояние цели**: МК застрял в состоянии, предотвращающем остановку
5. **Конфигурация тактовой частоты**: Проблемы со скоростью SWD

## Возможные решения

1. Проверить подключение ST-Link и питание
2. Сбросить цель вручную
3. Настроить конфигурацию OpenOCD
4. Использовать задачу "Free OpenOCD Ports" для завершения зависших процессов
5. Проверить пины SWD и подключения
