# Проблема с подключением STM32F411: JTAG/SWD communication failure

## Описание проблемы

После устранения конфликта портов OpenOCD запускается и слушает на портах 6666 (TCL) и 4444 (Telnet), но возникает ошибка коммуникации с целевым устройством:

```
Error: jtag status contains invalid mode value - communication failure
Polling target stm32f4x.cpu failed, trying to reexamine
Examination failed, GDB will be halted. Polling again in 100ms
```

OpenOCD пытается переподключиться, но безуспешно, с увеличивающимся интервалом опроса (100ms → 300ms → 700ms → 1500ms → 3100ms → 6300ms).

## Анализ причины

### Конфигурация сброса в openocd.cfg
```cfg
reset_config srst_only connect_assert_srst
```

Эта настройка означает:
- `srst_only`: Использовать только системный сброс (SRST)
- `connect_assert_srst`: При подключении активировать SRST

### Проблема
ST-Link не может выполнить сброс процессора, потому что **линия сброса (RST/NRST) не подключена** между программатором и STM32F411.

Без подключения RST OpenOCD не может:
- Корректно инициализировать коммуникацию
- Перевести процессор в известное состояние
- Выполнить необходимые процедуры подключения

## Решение: Подключение линии сброса

### Вариант 1: Подключить RST пин (Рекомендуется)
Подключить пин RST (Reset) ST-Link к пину NRST STM32F411:

**Распиновка ST-Link V2:**
- Pin 3: RST (System Reset)

**Распиновка STM32F411 Black Pill:**
- Pin 7: NRST (Reset)

**Подключение:**
```
ST-Link Pin 3 (RST) → STM32F411 Pin 7 (NRST)
```

### Вариант 2: Изменить конфигурацию сброса
Если подключение RST невозможно, изменить `reset_config` в `openocd.cfg`:

```cfg
# Вместо:
reset_config srst_only connect_assert_srst

# Использовать:
reset_config none separate connect_assert_srst
# или
reset_config trst_only separate connect_assert_srst
```

**Внимание:** Этот вариант может привести к нестабильной работе отладки и рекомендуется только как временное решение.

### Вариант 3: Использовать STM32CubeProgrammer для проверки
Установить STM32CubeProgrammer и проверить подключение:
1. Подключить ST-Link
2. Запустить STM32CubeProgrammer
3. Выбрать ST-LINK в качестве программатора
4. Проверить соединение с STM32F411

## Проверка исправления

1. **Подключить RST линию** (ST-Link Pin 3 → STM32 NRST)
2. **Перезагрузить ST-Link** (отключить/подключить USB)
3. **Запустить OpenOCD:**
   ```bash
   openocd -f /home/ypc/projects/stm32/ILI9341_project/ILI9341_stm32f411/openocd.cfg
   ```
4. **Ожидаемый результат:**
   ```
   Info : Target voltage: X.XXX
   Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
   Info : [stm32f4x.cpu] target has 6 breakpoints, 4 watchpoints
   Info : starting gdb server for stm32f4x.cpu on 3333
   Info : Listening on port 3333 for gdb connections
   ```

## Дополнительные рекомендации

- **Проверить питание:** Убедиться, что STM32F411 получает стабильное питание 3.3V
- **Проверить SWD линии:**
  - SWCLK (ST-Link Pin 2 → STM32 PA14)
  - SWDIO (ST-Link Pin 4 → STM32 PA13)
  - GND (ST-Link Pin 6 → STM32 GND)
- **Попробовать другой ST-Link** или USB-порт
- **Обновить драйверы ST-Link** если проблема persists

После подключения RST отладка в VS Code должна заработать корректно.
