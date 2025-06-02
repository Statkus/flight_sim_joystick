/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_hid.h"

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

uint16_t Low_Pass_Filter_ADC(uint32_t new_value, uint16_t previous_value);
uint16_t Temporal_Filter_ADC(uint32_t new_value, uint16_t* value_buffer, uint8_t* value_buffer_index);

uint16_t Read_Shift_Register(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t new_ADC_data = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USB_DEVICE_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  Joystick_HID_TypeDef joystick_HID          = {0};
  Joystick_HID_TypeDef previous_joystick_HID = {0};

  uint32_t ADC_DMA_buffer[5] = {0};

  uint16_t ADC_value_buffer[5][TEMPORAL_FILTER_WINDOW_SIZE] = {0};
  uint8_t ADC_value_buffer_index[5]                         = {0};

  HAL_ADC_Start_DMA(&hadc1, ADC_DMA_buffer, 5);

  HAL_TIM_Base_Start_IT(&htim1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* Handle joystick ---------------------------------------------------------------------------*/
    if (new_ADC_data == 1)
    {
      new_ADC_data = 0;

      //joystick_HID.rx     = Low_Pass_Filter_ADC(ADC_DMA_buffer[0], joystick_HID.rx);
      //joystick_HID.ry     = Low_Pass_Filter_ADC(ADC_DMA_buffer[1], joystick_HID.ry);
      //joystick_HID.rz     = Low_Pass_Filter_ADC(ADC_DMA_buffer[2], joystick_HID.rz);
      //joystick_HID.slider = Low_Pass_Filter_ADC(ADC_DMA_buffer[3], joystick_HID.slider);
      //joystick_HID.dial   = Low_Pass_Filter_ADC(ADC_DMA_buffer[4], joystick_HID.dial);

      joystick_HID.rx     = Temporal_Filter_ADC(ADC_DMA_buffer[0], ADC_value_buffer[0], &ADC_value_buffer_index[0]);
      joystick_HID.ry     = Temporal_Filter_ADC(ADC_DMA_buffer[1], ADC_value_buffer[1], &ADC_value_buffer_index[1]);
      joystick_HID.rz     = Temporal_Filter_ADC(ADC_DMA_buffer[2], ADC_value_buffer[2], &ADC_value_buffer_index[2]);
      joystick_HID.slider = Temporal_Filter_ADC(ADC_DMA_buffer[3], ADC_value_buffer[3], &ADC_value_buffer_index[3]);
      joystick_HID.dial   = Temporal_Filter_ADC(ADC_DMA_buffer[4], ADC_value_buffer[4], &ADC_value_buffer_index[4]);

      HAL_ADC_Start_DMA(&hadc1, ADC_DMA_buffer, 5);

      joystick_HID.buttons = (uint64_t)(Read_Shift_Register());
    }

    if (joystick_HID.buttons != previous_joystick_HID.buttons
     || joystick_HID.rx      != previous_joystick_HID.rx
     || joystick_HID.ry      != previous_joystick_HID.ry
     || joystick_HID.rz      != previous_joystick_HID.rz
     || joystick_HID.slider  != previous_joystick_HID.slider
     || joystick_HID.dial    != previous_joystick_HID.dial)
    {
      USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&joystick_HID, sizeof(Joystick_HID_TypeDef));
      HAL_Delay(20);
    }

    previous_joystick_HID.buttons = joystick_HID.buttons;
    previous_joystick_HID.rx      = joystick_HID.rx;
    previous_joystick_HID.ry      = joystick_HID.ry;
    previous_joystick_HID.rz      = joystick_HID.rz;
    previous_joystick_HID.slider  = joystick_HID.slider;
    previous_joystick_HID.dial    = joystick_HID.dial;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 95;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SR_LATCH_Pin|SR_CLOCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SR_LATCH_Pin SR_CLOCK_Pin */
  GPIO_InitStruct.Pin = SR_LATCH_Pin|SR_CLOCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SR_DATA_Pin */
  GPIO_InitStruct.Pin = SR_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SR_DATA_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    new_ADC_data = 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  // Heartbeat LED at 1Hz
  static uint16_t LED_counter = 0;

  if (LED_counter > 500)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    LED_counter = 0;
  }
  else
  {
    LED_counter++;
  }
}

uint16_t Low_Pass_Filter_ADC(uint32_t new_value, uint16_t previous_value)
{
  return (uint16_t)((float)previous_value + (((float)(MIN(new_value, 4096)) - (float)previous_value) * LOW_PASS_FILTER_COEF));
}

uint16_t Temporal_Filter_ADC(uint32_t new_value, uint16_t* value_buffer, uint8_t* value_buffer_index)
{
  value_buffer[*value_buffer_index] = (uint16_t)(MIN(new_value, 4096));

  if (*value_buffer_index < TEMPORAL_FILTER_WINDOW_SIZE - 1)
  {
    ++*value_buffer_index;
  }
  else
  {
    *value_buffer_index = 0;
  }

  uint32_t sum_value = 0;

  for (int i = 0; i < TEMPORAL_FILTER_WINDOW_SIZE; i++)
  {
    sum_value += value_buffer[i];
  }

  return (uint16_t)(sum_value / TEMPORAL_FILTER_WINDOW_SIZE);
}

uint16_t Read_Shift_Register(void)
{
  // Set clock low
  HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_RESET);

  // Latch impulse
  HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_SET);
  for (int i = 0; i < SHIFT_REGISTER_TICK_DELAY; i++) __NOP();
  HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_RESET);

  uint16_t data = 0;

  for (int bit_index = 0; bit_index < 16; bit_index++)
  {
    HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_RESET);
    for (int i = 0; i < SHIFT_REGISTER_TICK_DELAY; i++) __NOP();

    if (HAL_GPIO_ReadPin(SR_DATA_GPIO_Port, SR_DATA_Pin) == GPIO_PIN_RESET)
    {
      data |= 0x8000 >> bit_index;
    }

    HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_SET);
    for (int i = 0; i < SHIFT_REGISTER_TICK_DELAY; i++) __NOP();
  }

  return data;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
