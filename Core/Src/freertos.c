/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "ili9341.h"
#include "fonts.h"
#include "logger.h"
#include "config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */

  LOG_Init();
  HAL_Delay(1000); // Give USB CDC time to initialize
  LOG_Printf("System: Starting application\n");

  // Turn on backlight
  LOG_Printf("GPIO: Turning on backlight\n");
  HAL_GPIO_WritePin(BACK_LIGHT_GPIO_Port, BACK_LIGHT_Pin, GPIO_PIN_SET);

  // Initialize display
  ILI9341_Init();

  // Set rotation to landscape
  LOG_Printf("Setting display to landscape mode");
  ILI9341_SetRotation(0x28);  // Landscape mode

  // Fill screen with black
  ILI9341_FillScreen(ILI9341_BLACK);

#if TASK_ALPHABET_DISPLAY
  // Task 1: Display complete alphabet with two fonts
  LOG_Printf("Task: Alphabet Display");
  LOG_Printf("Font1: 5x7 pixels, Font1_2x: 10x14 pixels");

  // Font1 (5x7) - Uppercase A-Z
  const char *upper_az = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  LOG_Printf("Font1 Uppercase: %s", upper_az);
  ILI9341_DrawString(10, 10, upper_az, ILI9341_WHITE, ILI9341_BLACK, 1, Font1);

  // Font1 (5x7) - Lowercase a-z
  const char *lower_az = "abcdefghijklmnopqrstuvwxyz";
  LOG_Printf("Font1 Lowercase: %s", lower_az);
  ILI9341_DrawString(10, 25, lower_az, ILI9341_CYAN, ILI9341_BLACK, 1, Font1);

  // Font1_2x (10x14) - Uppercase A-Z
  LOG_Printf("Font1_2x Uppercase: %s", upper_az);
  ILI9341_DrawStringLarge(10, 45, upper_az, ILI9341_YELLOW, ILI9341_BLACK);

  // Font1_2x (10x14) - Lowercase a-z
  LOG_Printf("Font1_2x Lowercase: %s", lower_az);
  ILI9341_DrawStringLarge(10, 75, lower_az, ILI9341_RED, ILI9341_BLACK);

#elif TASK_SCROLLING_HELLO
  // Task 2: Cyclic scrolling "Hello World!" text in large font
  LOG_Printf("Task: Scrolling Hello World!");
  LOG_Printf("Cyclic scrolling text with Font1_2x");

  static int scroll_pos = 320; // Start from right edge
  const char *hello_text = "Hello World! ";
  const int text_width = 12 * 13; // Approximate width of text in large font

  int prev_scroll_pos = scroll_pos;

  while (1) {
    // Calculate text boundaries
    int current_end = scroll_pos + text_width;
    int prev_end = prev_scroll_pos + text_width;

    // Clear only the area that needs to be cleared (partial update)
    // Clear the trailing area of previous text position
    if (prev_end > 0 && prev_end < 320) {
      ILI9341_FillRectangle(prev_end, 100, 320 - prev_end, 40, ILI9341_BLACK);
    }

    // Clear the leading area if text is entering from right
    if (scroll_pos > 0 && scroll_pos < 320) {
      ILI9341_FillRectangle(0, 100, scroll_pos, 40, ILI9341_BLACK);
    }

    // Draw the scrolling text
    ILI9341_DrawStringLarge(scroll_pos, 110, hello_text, ILI9341_GREEN, ILI9341_BLACK);

    // Update positions
    prev_scroll_pos = scroll_pos;
    scroll_pos -= 2; // Move left by 2 pixels

    // Reset position when text goes off screen
    if (scroll_pos < -text_width) {
      scroll_pos = 320;
      prev_scroll_pos = scroll_pos;
    }

    // Optimized delay for smoother scrolling
    osDelay(30); // Reduced from 50ms for smoother animation
  }

#endif

  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
