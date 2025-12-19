# Отладка STM32F411 с OpenOCD

## Настройка

### 1. Установка OpenOCD
```bash
# Ubuntu/Debian
sudo apt install openocd

# Arch Linux
sudo pacman -S openocd

# macOS
brew install openocd
```

### 2. Подключение ST-Link
- Подключите ST-Link к USB
- Подключите SWD пины к STM32F411:
  - SWDIO → PA13
  - SWCLK → PA14
  - GND → GND
  - 3.3V → 3.3V (опционально)

## Запуск отладки

### 1. Сборка проекта
```bash
make clean && make
```

### 2. Запуск OpenOCD
```bash
make debug
# или
openocd -f openocd.cfg
```

OpenOCD запустится на порту 3333 (GDB) и 4444 (Telnet).

### 3. Подключение GDB
```bash
# В другом терминале
arm-none-eabi-gdb build/ILI9341_stm32f411.elf
```

В GDB:
```gdb
target remote localhost:3333
monitor reset halt
break main
continue
```

## Диагностика зависания

### Если STM32 зависает в TOUCH_Init():

1. **Установить breakpoint в начале TOUCH_Init():**
```gdb
break TOUCH_Init
continue
```

2. **Посмотреть, доходит ли до TOUCH_Init():**
```gdb
info breakpoints
```

3. **Если доходит - пошагово выполнить:**
```gdb
step
```

4. **Найти строку, где зависает:**
```gdb
info registers
```

### Команды OpenOCD

```bash
# Подключиться к OpenOCD telnet
telnet localhost 4444

# Команды в telnet:
reset halt          # Сброс и остановка
flash probe 0       # Проверить flash
mdw 0x08000000 10   # Прочитать память
```

### Распространенные проблемы

1. **"Error: open failed"**
   - ST-Link не подключен
   - Неправильные права доступа

2. **"Target not examined"**
   - Неправильная конфигурация target
   - STM32 не запитано

3. **GDB не подключается**
   - Проверить, что OpenOCD запущен
   - Проверить порт 3333

## Логи отладки

Если система работает, вы увидите логи через USB CDC. Если зависает - используйте OpenOCD для анализа состояния процессора.
