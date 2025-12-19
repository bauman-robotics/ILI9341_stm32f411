# Ручной запуск отладки STM32F411

## Предварительные требования
- ST-Link подключен с линиями SWCLK, SWDIO, GND, RST
- STM32F411 получает питание 3.3V
- OpenOCD установлен
- ARM GCC toolchain установлен (arm-none-eabi-gdb)

## Последовательность запуска

### Терминал 1: Запуск OpenOCD
```bash
cd /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411
openocd -f openocd.cfg
```

**Ожидаемый вывод:**
```
Info : Listening on port 3333 for gdb connections
Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
```

### Терминал 2: Подключение GDB
```bash
cd /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411
arm-none-eabi-gdb build/ILI9341_stm32f411.elf
```

**В GDB выполнить команды:**
```gdb
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) break main
(gdb) continue
```

## Полная последовательность команд

```bash
# Терминал 1
openocd -f /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411/openocd.cfg

# Терминал 2
cd /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411
arm-none-eabi-gdb build/ILI9341_stm32f411.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "load" -ex "break main" -ex "continue"
```

## Остановка отладки
- В GDB: `Ctrl+C` затем `quit`
- В терминале OpenOCD: `Ctrl+C`

## Возможные проблемы
- Если порт 3333 занят: проверить и завершить другие экземпляры OpenOCD
- Если "Connection refused": убедиться, что OpenOCD запущен и слушает порт
- Если проблемы с RST: проверить подключение RST линии
