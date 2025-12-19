# Проблема: HardFault предотвращает операции с flash

## В чем проблема

На основе предоставленных логов OpenOCD:

### Симптомы
```
[stm32f4x.cpu] halted due to debug-request, current mode: Handler HardFault
xPSR: 0x41000003 pc: 0x08000d16 msp: 0x2001ffa8, semihosting
Error: timed out while waiting for target halted
Error: Target not halted
Error: failed erasing sectors 0 to 1
Error: flash_erase returned -304
```

### Корень проблемы
STM32F411 находится в состоянии **HardFault** - это критическое исключение процессора, которое возникает при:
- Доступе к недопустимым адресам памяти
- Делении на ноль
- Выполнении невалидных инструкций
- Переполнении стека
- Других аппаратных или программных ошибках

Пока процессор в HardFault, OpenOCD не может выполнить:
- Корректную остановку цели
- Операции стирания flash
- Загрузку новой прошивки

## Решение: Ручная очистка flash

### Консольные команды для правильного запуска

**Шаг 1: Запустить OpenOCD**
```bash
cd /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411
openocd -f openocd.cfg
```
*Ожидайте сообщения о подключении GDB (это нормально, даже если будут ошибки flash erase)*

**Шаг 2: Подключиться к консоли OpenOCD (в новом терминале)**
```bash
telnet localhost 4444
```

**Шаг 3: Выполнить команды сброса и очистки в telnet**
```
> reset halt
> flash erase_sector 0 0 127
> reset halt
> exit
```

**Пояснения:**
- `reset halt` - сбросить процессор и остановить его
- `flash erase_sector 0 0 127` - стереть все сектора flash (STM32F411 имеет 128 секторов по 4KB каждый)
- Второй `reset halt` - подтвердить остановку после стирания

**Шаг 4: Остановить OpenOCD (Ctrl+C в первом терминале)**

**Шаг 5: Перезапустить OpenOCD**
```bash
openocd -f openocd.cfg
```

**Шаг 6: Подключить GDB для загрузки прошивки**
```bash
arm-none-eabi-gdb build/ILI9341_stm32f411.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "load" -ex "monitor reset halt" -ex "break main" -ex "continue"
```

## Альтернативный способ через GDB

Если telnet недоступен:

**Шаг 1: Запустить OpenOCD**
```bash
openocd -f openocd.cfg
```

**Шаг 2: Подключить GDB**
```bash
arm-none-eabi-gdb build/ILI9341_stm32f411.elf
```

**Шаг 3: Выполнить команды в GDB**
```gdb
(gdb) target extended-remote localhost:3333
(gdb) monitor reset
(gdb) monitor halt
(gdb) monitor flash erase_sector 0 0 127
(gdb) monitor reset
(gdb) monitor halt
(gdb) load
(gdb) monitor reset
(gdb) monitor halt
(gdb) break main
(gdb) continue
```

## Проверка успешности

После выполнения команд OpenOCD должен показать:
```
Info : flash erase finished
Info : [stm32f4x.cpu] halted due to debug-request, current mode: Thread
xPSR: 0x01000000 pc: 0xfffffffe msp: 0x20020000
```

Обратите внимание на `current mode: Thread` вместо `Handler HardFault`.

## Профилактика

- Всегда выполняйте `reset halt` перед загрузкой прошивки
- При появлении HardFault анализируйте код на предмет ошибок доступа к памяти
- Регулярно проверяйте подключение RST линии для стабильной работы отладки
