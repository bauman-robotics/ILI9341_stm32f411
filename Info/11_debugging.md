# Ошибка отладки STM32F411: Connection timeout на порту 50000

## Описание ситуации

При попытке запуска отладки в VS Code с использованием расширения Cortex-Debug (версия 1.12.1) возникает ошибка подключения к GDB-серверу. Ошибка проявляется следующим образом:

- Нажатие F5 для запуска отладки
- GDB пытается подключиться к `localhost:50000`
- Получается таймаут соединения: `localhost:50000: Connection timed out`
- OpenOCD-сервер не запускается или не отвечает на указанном порту

### Конфигурация отладки
```json
{
    "name": "Debug STM32F411 (OpenOCD)",
    "type": "cortex-debug",
    "request": "launch",
    "servertype": "openocd",
    "configFiles": [
        "/home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411/openocd.cfg"
    ],
    "cwd": "/home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411",
    "executable": "/home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411/build/ILI9341_stm32f411.elf",
    "device": "STM32F411CE",
    "interface": "swd",
    "serialNumber": "",
    "runToEntryPoint": "main",
    "preLaunchTask": "Build"
}
```

### Лог ошибки
```
Launching gdb-server: openocd -c "gdb_port 50000" -c "tcl_port 50001" -c "telnet_port 50002" -s /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411 -f /home/ypc/.vscode/extensions/marus25.cortex-debug-1.12.1/support/openocd-helpers.tcl -f /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411/openocd.cfg
-> 11^error,msg="localhost:50000: Connection timed out."
Failed to launch GDB: localhost:50000: Connection timed out.
```

## Возможные причины ошибки

1. **Проблемы с OpenOCD конфигурацией**: Файл `openocd.cfg` может содержать некорректные настройки
2. **Отсутствие или неправильное подключение отладчика**: ST-Link, J-Link или другой программатор не подключен или не распознан
3. **Конфликт портов**: Порт 50000 занят другим процессом
4. **Проблемы с USB**: Недостаточно прав доступа к USB-устройствам или драйверы не установлены
5. **Питание микроконтроллера**: STM32F411 не получает питание или находится в неправильном режиме

## Методы диагностики ошибки

### 1. Проверка конфигурации OpenOCD
- Проверить содержимое файла `openocd.cfg`
- Убедиться, что указан правильный интерфейс (swd) и устройство (STM32F411CE)
- Попробовать запустить OpenOCD вручную из терминала:
  ```bash
  openocd -f /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411/openocd.cfg
  ```

### 2. Проверка подключения отладчика
- Подключить ST-Link к USB-порту
- Проверить распознавание устройства:
  ```bash
  lsusb | grep ST-Link
  ```
- Проверить права доступа к USB-устройствам (добавить пользователя в группу dialout или plugdev)

### 3. Проверка портов
- Проверить, занят ли порт 50000:
  ```bash
  netstat -tulpn | grep 50000
  ```
- Если порт занят, завершить процесс или изменить порт в конфигурации

### 4. Ручной запуск OpenOCD
- Попробовать запустить OpenOCD с минимальной конфигурацией:
  ```bash
  openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
  ```
- Проверить вывод на наличие ошибок

### 5. Проверка питания и подключения
- Убедиться, что STM32F411 получает питание
- Проверить правильность подключения SWD интерфейса (SWCLK, SWDIO, GND, VCC)
- Попробовать сброс микроконтроллера

### 6. Логи отладки
- Включить подробный вывод отладки в VS Code настройках Cortex-Debug
- Проверить логи OpenOCD в терминале gdb-server
- Искать сообщения об ошибках в выводе

### 7. Альтернативные методы отладки
- Попробовать использовать другой GDB порт (изменить в launch.json)
- Проверить работу с STM32CubeIDE или другими IDE
- Использовать telnet для подключения к OpenOCD: `telnet localhost 50002`

## Рекомендации по исправлению

1. **Базовая проверка**:
   - Перезагрузить компьютер и отладчик
   - Проверить все физические подключения
   - Убедиться в наличии драйверов ST-Link

2. **Конфигурация**:
   - Проверить и скорректировать `openocd.cfg`
   - Обновить Cortex-Debug до последней версии

3. **USB права**:
   ```bash
   sudo usermod -a -G dialout $USER
   sudo usermod -a -G plugdev $USER
   ```

4. **Тестирование**:
   - Создать простой тестовый проект для проверки отладки
   - Попробовать отладку на другом компьютере

Если проблема persists, рекомендуется обратиться к документации OpenOCD и форумам разработчиков STM32.
