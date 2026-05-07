/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "Statechart.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SHT31_ADDR (0x44 << 1)
#define SHT31_READY_TRIES 3
#define SHT31_I2C_TIMEOUT_MS 100
#define SHT31_MEASUREMENT_DELAY_MS 20
static const uint8_t CMD_MEASURE_TEMP[] = {0x24, 0x00};

#define RTC_WAKEUP_TIME_SECONDS 2
#define UART_BAUD_RATE 115200
#define UART_TIMEOUT_MS 100

#define DISPLAY_LEFT_COLUMN 0
#define DISPLAY_TEMP_ROW 11
#define DISPLAY_HUMIDITY_ROW 22
#define DISPLAY_TEXT_BUFFER_SIZE 20
#define UART_TEXT_BUFFER_SIZE 48

#define SHT31_DATA_SIZE 6
#define SHT31_TEMPERATURE_OFFSET_TENTHS -450
#define SHT31_TEMPERATURE_SCALE_TENTHS 1750
#define SHT31_HUMIDITY_SCALE_TENTHS 1000
#define SHT31_CONVERSION_ROUNDING 32767
#define SHT31_CONVERSION_MAX 65535

#define TEMPERATURE_ALARM_TENTHS 350
#define HUMIDITY_ALARM_TENTHS 800
#define STARTUP_LED_TIME_MS 2000
#define LED_OFF 0
#define LED_ON 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim11;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
static int16_t temperature_tenths = 0;
static uint16_t humidity_tenths = 0;
static volatile uint8_t rtc_alarm_flag = 0;
static Statechart sc_handle;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM11_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
static uint8_t SHT31_IsReady(void);
static uint8_t SHT31_ReadTempHumidity(void);
static int GetTemperatureDecimal(void);
static void ProcessMeasurement(void);
static void DisplayMeasurement(void);
static void UartTransmitString(const char *message);
static void UartOutputMeasurement(void);
static void SetLed(GPIO_TypeDef *port, uint16_t pin, uint8_t on);
static void UpdateAlarmLed(void);
static void EnterStopMode(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
  (void)hrtc;
  rtc_alarm_flag = 1;
}

static uint8_t SHT31_IsReady(void)
{
  if (HAL_I2C_IsDeviceReady(&hi2c1, SHT31_ADDR, SHT31_READY_TRIES,
                            SHT31_I2C_TIMEOUT_MS) == HAL_OK)
  {
    return 1;
  }

  return 0;
}

static uint8_t SHT31_ReadTempHumidity(void)
{
  uint8_t data[SHT31_DATA_SIZE];
  uint16_t temp_raw;
  uint16_t humidity_raw;
  int32_t temperature_scaled;
  int32_t humidity_scaled;

  if (HAL_I2C_Master_Transmit(&hi2c1, SHT31_ADDR,
                              (uint8_t *)CMD_MEASURE_TEMP,
                              sizeof(CMD_MEASURE_TEMP),
                              SHT31_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return 0;
  }

  HAL_Delay(SHT31_MEASUREMENT_DELAY_MS);

  if (HAL_I2C_Master_Receive(&hi2c1, SHT31_ADDR, data, sizeof(data),
                             SHT31_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return 0;
  }

  temp_raw = ((uint16_t)data[0] << 8) | data[1];
  humidity_raw = ((uint16_t)data[3] << 8) | data[4];

  temperature_scaled = (SHT31_TEMPERATURE_SCALE_TENTHS * (int32_t)temp_raw) +
                       SHT31_CONVERSION_ROUNDING;
  temperature_tenths = (int16_t)(SHT31_TEMPERATURE_OFFSET_TENTHS +
                                 (temperature_scaled / SHT31_CONVERSION_MAX));

  humidity_scaled = (SHT31_HUMIDITY_SCALE_TENTHS * (int32_t)humidity_raw) +
                    SHT31_CONVERSION_ROUNDING;
  humidity_tenths = (uint16_t)(humidity_scaled / SHT31_CONVERSION_MAX);

  return 1;
}

static int GetTemperatureDecimal(void)
{
  int decimal = temperature_tenths % 10;

  if (decimal < 0)
  {
    decimal = -decimal;
  }

  return decimal;
}

static void ProcessMeasurement(void)
{
  SetLed(LED2_GPIO_Port, LED2_Pin, LED_ON);

  if (SHT31_IsReady() == 0)
  {
    SetLed(LED3_GPIO_Port, LED3_Pin, LED_OFF);
    UartTransmitString("SHT31 not ready\r\n");
  }
  else if (SHT31_ReadTempHumidity() == 0)
  {
    SetLed(LED3_GPIO_Port, LED3_Pin, LED_OFF);
    UartTransmitString("SHT31 read failed\r\n");
  }
  else
  {
    UpdateAlarmLed();
    UartOutputMeasurement();
  }

  SetLed(LED2_GPIO_Port, LED2_Pin, LED_OFF);
}

static void UartTransmitString(const char *message)
{
  size_t length = strlen(message);

  (void)HAL_UART_Transmit(&huart1, (uint8_t *)message, (uint16_t)length,
                          UART_TIMEOUT_MS);
}

static void UartOutputMeasurement(void)
{
  char buffer[UART_TEXT_BUFFER_SIZE];
  int length;

  length = snprintf(buffer, sizeof(buffer), "Temperature: %d.%d C, Humidity: %u.%u %%\r\n",
                    temperature_tenths / 10,
                    GetTemperatureDecimal(),
                    humidity_tenths / 10,
                    humidity_tenths % 10);

  if (length > 0)
  {
    if (length >= (int)sizeof(buffer))
    {
      length = (int)sizeof(buffer) - 1;
    }
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)length,
                            UART_TIMEOUT_MS);
  }
}

static void DisplayMeasurement(void)
{
  char buffer[DISPLAY_TEXT_BUFFER_SIZE];

  ssd1306_Fill(Black);

  ssd1306_SetCursor(DISPLAY_LEFT_COLUMN, DISPLAY_TEMP_ROW);
  snprintf(buffer, sizeof(buffer), "Temp: %d.%d C",
           temperature_tenths / 10,
           GetTemperatureDecimal());
  ssd1306_WriteString(buffer, Font_6x8, White);

  ssd1306_SetCursor(DISPLAY_LEFT_COLUMN, DISPLAY_HUMIDITY_ROW);
  snprintf(buffer, sizeof(buffer), "Hum:  %u.%u %%",
           humidity_tenths / 10,
           humidity_tenths % 10);
  ssd1306_WriteString(buffer, Font_6x8, White);

  ssd1306_UpdateScreen();
}

static void SetLed(GPIO_TypeDef *port, uint16_t pin, uint8_t on)
{
  GPIO_PinState pin_state;

  if (on != 0)
  {
    pin_state = GPIO_PIN_SET;
  }
  else
  {
    pin_state = GPIO_PIN_RESET;
  }

  HAL_GPIO_WritePin(port, pin, pin_state);
}

static void UpdateAlarmLed(void)
{
  uint8_t alarm_active = LED_OFF;

  if ((temperature_tenths > TEMPERATURE_ALARM_TENTHS) ||
      (humidity_tenths > HUMIDITY_ALARM_TENTHS))
  {
    alarm_active = LED_ON;
  }

  SetLed(LED3_GPIO_Port, LED3_Pin, alarm_active);
}

static void EnterStopMode(void)
{
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  HAL_ResumeTick();

  SystemClock_Config();
}



void statechart_goSleep(Statechart *handle)
{
  (void)handle;
  EnterStopMode();
}

void statechart_readI2CSensor(Statechart *handle)
{
  (void)handle;
  ProcessMeasurement();
}

void statechart_processData(Statechart *handle)
{
  (void)handle;
}

void statechart_displayInfo(Statechart *handle)
{
  (void)handle;
  DisplayMeasurement();
}

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
  MX_I2C1_Init();
  MX_TIM11_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  SetLed(LED1_GPIO_Port, LED1_Pin, LED_ON);
  HAL_Delay(STARTUP_LED_TIME_MS);
  SetLed(LED1_GPIO_Port, LED1_Pin, LED_OFF);

  UartTransmitString("STM-401 UART ready\r\n");
  ssd1306_Init();
  statechart_init(&sc_handle);
  statechart_enter(&sc_handle);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (rtc_alarm_flag != 0)
    {
      rtc_alarm_flag = 0;
      statechart_raise_ev_RTC_Alarm(&sc_handle);
    }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 80;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, RTC_WAKEUP_TIME_SECONDS,
                                  RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 640;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = UART_BAUD_RATE;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_GPIO_WritePin(GPIOC, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
