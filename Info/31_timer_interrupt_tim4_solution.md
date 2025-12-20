# 31. Решение проблемы с TIM4: Переопределение HAL_Delay в FreeRTOS

## Проблема
Отключение `HAL_TIM_Base_Start_IT(&htim4);` в `HAL_InitTick()` остановит прерывания TIM4, но `HAL_Delay()` перестанет работать, так как `uwTick` не обновляется.

## Решение: Переопределение HAL_Delay через FreeRTOS
В FreeRTOS **не рекомендуется** использовать `HAL_Delay()`, так как он блокирует и не учитывает переключение задач. Вместо этого используйте `osDelay()` или `vTaskDelay()`.

### Если HAL_Delay() всё же нужен:
Добавьте в `Core/Src/freertos.c` (в раздел `/* USER CODE BEGIN Application */`):

```c
void HAL_Delay(uint32_t Delay)
{
  osDelay(Delay);
}
```

Это переопределит weak функцию из HAL и сделает `HAL_Delay()` совместимым с FreeRTOS.

### Рекомендация:
- Замените все `HAL_Delay()` в коде на `osDelay()`.
- Отключите прерывания TIM4 и используйте `osDelay()` для задержек.

### Шаги для тестирования:
1. Добавьте переопределение `HAL_Delay()` в `freertos.c`.
2. Закомментируйте `HAL_TIM_Base_Start_IT(&htim4);` в `stm32f4xx_hal_timebase_tim.c`.
3. Перекомпилируйте и проверьте, что `HAL_Delay(1000)` работает как 1 секунда задержки.

## Заключение
В FreeRTOS используйте `osDelay()` вместо `HAL_Delay()`. Если нужно сохранить `HAL_Delay()`, переопределите его через `osDelay()`.
