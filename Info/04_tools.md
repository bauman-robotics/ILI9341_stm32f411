# Необходимое программное обеспечение для компиляции и отладки кода STM32F411

## Основные инструменты

### 1. Компилятор и toolchain
- **arm-none-eabi-gcc** - кросс-компилятор для ARM Cortex-M
- **arm-none-eabi-gdb** - отладчик
- **make** - система сборки

### 2. STM32CubeIDE
- Интегрированная среда разработки от STMicroelectronics
- Включает в себя компилятор, отладчик и инструменты генерации кода

### 3. OpenOCD
- Инструмент для отладки и программирования через JTAG/SWD

### 4. ST-Link
- Программатор и отладчик для STM32 микроконтроллеров

### 5. STM32CubeMX
- Инструмент для генерации кода и конфигурации периферии

## Команды для проверки наличия инструментов в Linux

### Проверка arm-none-eabi-gcc
```bash
arm-none-eabi-gcc --version
# или
which arm-none-eabi-gcc
```

### Проверка make
```bash
make --version
# или
which make
```

### Проверка OpenOCD
```bash
openocd --version
# или
which openocd
```

### Проверка gdb
```bash
arm-none-eabi-gdb --version
# или
which arm-none-eabi-gdb
```

### Проверка Python (для некоторых скриптов)
```bash
python3 --version
# или
which python3
```

### Проверка git (для загрузки репозиториев)
```bash
git --version
# или
which git
```

## Установка инструментов

### Ubuntu/Debian
```bash
# Установка toolchain
sudo apt update
sudo apt install gcc-arm-none-eabi gdb-multiarch make

# Установка OpenOCD
sudo apt install openocd

# Установка STM32CubeIDE (скачать с сайта ST)
# STM32CubeMX (скачать с сайта ST)
```

### Arch Linux
```bash
# Установка toolchain
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-gdb make

# OpenOCD
sudo pacman -S openocd
```

## Альтернативные инструменты
- **PlatformIO** - платформа для разработки embedded проектов
- **Visual Studio Code** с расширениями для ARM development
- **Eclipse** с плагинами для STM32

## Проверка конфигурации
После установки проверить версии всех инструментов и убедиться, что они находятся в PATH.
