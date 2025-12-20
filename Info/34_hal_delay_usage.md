# 34. Анализ использования HAL_Delay в проекте

## Все использования HAL_Delay

### Пользовательский код:
- **Core/Src/ili9341.c**: `HAL_Delay(ms);` — задержка в инициализации дисплея.
- **Core/Src/logger.c**: `HAL_Delay(10);`, `HAL_Delay(5);` — задержки в логировании.
- **USB_DEVICE/Target/usbd_conf.c**: `HAL_Delay(Delay);` — задержка в USB конфигурации.

### Драйверы HAL (не рекомендуется изменять):
- **stm32f4xx_hal_sd.c**: `HAL_Delay(2);` — задержка в SD карте.
- **stm32f4xx_hal_mmc.c**: `HAL_Delay(2);` — задержка в MMC.
- **stm32f4xx_hal_pcd_ex.c**: `HAL_Delay(300U);`, `HAL_Delay(50U);` — задержки в USB.
- **stm32f4xx_hal_eth.c**: Множество `HAL_Delay(ETH_REG_WRITE_DELAY);` — задержки в Ethernet.
- **stm32f4xx_ll_usb.c**: `HAL_Delay(10U);`, `HAL_Delay(100U);` — задержки в USB low-level.

## Вывод
- **В пользовательском коде**: 4 использования (ili9341.c, logger.c, usbd_conf.c) — можно заменить на `osDelay()`.
- **В драйверах HAL**: 20+ использований — не изменять, переопределить `HAL_Delay()` глобально.
- **Рекомендация**: Переопределить `HAL_Delay()` в `freertos.c` через `osDelay()`, чтобы все вызовы работали корректно в FreeRTOS. Это безопасно и не требует изменения драйверов.
