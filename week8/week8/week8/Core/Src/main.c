/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_PACKET_LEN 255
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
typedef struct _UartStructure
{
	UART_HandleTypeDef *huart;
	uint16_t TxLen, RxLen;
	uint8_t *TxBuffer;
	uint16_t TxTail, TxHead;
	uint8_t *RxBuffer;
	uint16_t RxTail; //RXHeadUseDMA

} UARTStucrture;
UARTStucrture UART2 =
{ 0 };

uint8_t MainMemory[255] =
{ 0 };
_Bool checkI2C = 0;
static uint8_t CHECKSUM;
static uint8_t CHECK_SUM1 = 0;
static uint8_t CHECK_SUM3 = 0;
static uint8_t CHECK_SEND = 0;
static uint16_t START = 0;
static uint16_t MODE = 0;
static uint8_t DATAFRAME [256] = { 0 };
static uint8_t DATAFRAME_4 [256] = { 0 };
static int DATAFRAME_5[256] = { 0 };
static uint16_t JUMP = 0;
static uint16_t STATEJUMP = 0;
static uint16_t DATA = 0;
static uint16_t DATA_Byte = 0;
static uint8_t STATION = 0;
static uint8_t DATA_N_SUM = 0;
static uint8_t GO = 0;
static uint16_t Endeffecter = 0;
static uint16_t S = 0;
static uint8_t x = 0;
static uint16_t ACK = 0;
static uint16_t A = 0;
static uint16_t B = 0;
uint16_t XX = 0;
static float Vel_Data = 0; //--------------> Velocity from UI
static uint16_t Pos_Data = 0; //--------------> Position from UI
static uint16_t C_Station = 0; //--------------> Current Station from UI
static uint16_t Gripper = 0; //--------------> Gripper on/off , 1/0
static uint16_t Connect = 0; //--------------> Connect / Disconnect , 1/0
uint16_t store [20] = {0};

//order of station refers to angular position as follow
uint8_t stations[10] = {30,60,90,120,150,180,210,240,270,300};

typedef enum
{
	S_idle,
	S_Start,
	S_Mode,
	S_Mode_1,
	S_Mode_2,
	S_Mode_3,
	S_Mode_4,
	S_Mode_5,
	S_Mode_6,
	S_Mode_7,
	S_Mode_8,
	S_Mode_9,
	S_Mode_10,
	S_Mode_11,
	S_Mode_12,
	S_Mode_13,
	S_Mode_14,
	S_Frame2_DataFrame_1,
	S_Frame2_DataFrame_2,
	S_Frame2_DataFrame_Mode4_1,
	S_Frame2_DataFrame_Mode4_2,
	S_Frame2_DataFrame_Mode5_1,
	S_Frame2_DataFrame_Mode5_2,
	S_Frame3_Station,
	S_Frame3_DataFrame_1,
	S_Frame3_DataFrame_2,
	S_Checksum1,
	S_Checksum1_2,
	S_Checksum2,
	S_Checksum2_4,
	S_Checksum2_5,
	S_Checksum3,
	CheckACK1,
	CheckACK2,
	S_error,
	S_SendACK,
	S_Jump,
	S500,S600,S700,
	//////////////////////////////
	DNMXP_idle,
	DNMXP_1stHeader,
	DNMXP_2ndHeader,
	DNMXP_3rdHeader,
	DNMXP_Reserved,
	DNMXP_ID,
	DNMXP_LEN1,
	DNMXP_LEN2,
	DNMXP_Inst,
	DNMXP_ParameterCollect,
	DNMXP_CRCAndExecute

} DNMXPState;
static DNMXPState State = S_idle;

///////////////////////////////////////////////////////////////<--

int32_t PWMOut = 5000;
uint64_t _micros = 0;
uint8_t cP = 0;
int16_t a = 0;
float EncoderVel = 0;
float sumVel = 0;
float calculatedVelocity = 0;
float velocity = 0;
float position = 0;
float alpha = 0;
float stopError = 0.1;
float tim = 0;
uint8_t check = 0;
_Bool SetHome = 0; //--------------------->> sethome
uint64_t setTime = 0;
float startAngle = 0;
float stopTime = 0;
uint8_t storeAngle = 0;
float finalAngle = 30; //----------------->> Pos_Data
float currentPosition = 0;
float rawPosition[2] = {0};
float accerelation = 0;
uint8_t state[2] = {0};
_Bool start = 0; //---------------------->> go
uint16_t dt = 2000;
uint64_t TimeOutputLoop = 0;
uint64_t Timestamp = 0;
float vMax = 8; //--------------------->> Vel_Data
float Kp = 200;
float Ki = 0.1;
float Kd = 0;
float Kp_p = 1;
float Ki_p = 0.05;
float Kd_p = 0;
float K = 1000;
float R = 0.2;
float Gl = 10;
static float tF = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void UARTInit(UARTStucrture *uart);

void UARTResetStart(UARTStucrture *uart);

uint32_t UARTGetRxHead(UARTStucrture *uart);

int16_t UARTReadChar(UARTStucrture *uart);

void UARTTxDumpBuffer(UARTStucrture *uart);

void UARTTxWrite(UARTStucrture *uart, uint8_t *pData, uint16_t len);

unsigned short update_crc(unsigned short crc_accum, unsigned char *data_blk_ptr,
		unsigned short data_blk_size);
void DynamixelProtocal2(uint8_t *Memory, uint8_t MotorID, int16_t dataIn,
		UARTStucrture *uart);

uint64_t micros();
#define  HTIM_ENCODER htim1
float EncoderVelocity_Update();
void pidPosition();
void piVelocity();
void trajectory(uint64_t);
void kalman();
void gotoSethome();
void I2Con();
void I2CprepareRead();
uint8_t findingPosition();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	UART2.huart = &huart2;
	UART2.RxLen = 255;
	UART2.TxLen = 255;
	UARTInit(&UART2);
	UARTResetStart(&UART2);
	//start micros
	HAL_TIM_Base_Start_IT(&htim2);
	//Encoder start
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
	//PWM start AIN1
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	htim3.Instance->CCR1 = 5000;
	I2Con();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		int16_t inputChar = UARTReadChar(&UART2);
		a = inputChar;
		if (inputChar != -1)
		{
			DynamixelProtocal2(MainMemory, 1, inputChar, &UART2);
		}
		if(Connect == 1){
			findingPosition();
			gotoSethome();
			if (micros() - Timestamp >= dt){
				Timestamp = micros();
				trajectory(Timestamp);
				kalman();
			}
			UARTTxDumpBuffer(&UART2);
		}
		else{
			DynamixelProtocal2(MainMemory, 1,999, &UART2);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 2047;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 3;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 5;
  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 99;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 99;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 512000;
  huart2.Init.WordLength = UART_WORDLENGTH_9B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_EVEN;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
#define  MAX_SUBPOSITION_OVERFLOW 1536
#define  MAX_ENCODER_PERIOD 2048

void UARTInit(UARTStucrture *uart)
{
	//dynamic memory allocate
	uart->RxBuffer = (uint8_t*) calloc(sizeof(uint8_t), UART2.RxLen);
	uart->TxBuffer = (uint8_t*) calloc(sizeof(uint8_t), UART2.TxLen);
	uart->RxTail = 0;
	uart->TxTail = 0;
	uart->TxHead = 0;

}

void UARTResetStart(UARTStucrture *uart)
{
	HAL_UART_Receive_DMA(uart->huart, uart->RxBuffer, uart->RxLen);
}
uint32_t UARTGetRxHead(UARTStucrture *uart)
{
	return uart->RxLen - __HAL_DMA_GET_COUNTER(uart->huart->hdmarx);
}
int16_t UARTReadChar(UARTStucrture *uart)
{
	int16_t Result = -1; // -1 Mean no new data
	static uint8_t order = 0;
	//check Buffer Position
	if (uart->RxTail != UARTGetRxHead(uart))
	{
		//get data from buffer
		Result = uart->RxBuffer[uart->RxTail];
		uart->RxTail = (uart->RxTail + 1) % uart->RxLen;
		store[order] = Result;
		order += 1;
		order %= 20;

	}
	return Result;

}
void UARTTxDumpBuffer(UARTStucrture *uart)
{
	static uint8_t MultiProcessBlocker = 0;

	if (uart->huart->gState == HAL_UART_STATE_READY && !MultiProcessBlocker)
	{
		MultiProcessBlocker = 1;

		if (uart->TxHead != uart->TxTail)
		{
			//find len of data in buffer (Circular buffer but do in one way)
			uint16_t sentingLen =
					uart->TxHead > uart->TxTail ?
							uart->TxHead - uart->TxTail :
							uart->TxLen - uart->TxTail;

			//sent data via DMA
			HAL_UART_Transmit_DMA(uart->huart, &(uart->TxBuffer[uart->TxTail]),
					sentingLen);
			//move tail to new position
			uart->TxTail = (uart->TxTail + sentingLen) % uart->TxLen;

		}
		MultiProcessBlocker = 0;
	}
}
void UARTTxWrite(UARTStucrture *uart, uint8_t *pData, uint16_t len)
{
	//check data len is more than buffur?
	uint16_t lenAddBuffer = (len <= uart->TxLen) ? len : uart->TxLen;
	// find number of data before end of ring buffer
	uint16_t numberOfdataCanCopy =
			lenAddBuffer <= uart->TxLen - uart->TxHead ?
					lenAddBuffer : uart->TxLen - uart->TxHead;
	//copy data to the buffer
	memcpy(&(uart->TxBuffer[uart->TxHead]), pData, numberOfdataCanCopy);

	//Move Head to new position

	uart->TxHead = (uart->TxHead + lenAddBuffer) % uart->TxLen;
	//Check that we copy all data That We can?
	if (lenAddBuffer != numberOfdataCanCopy)
	{
		memcpy(uart->TxBuffer, &(pData[numberOfdataCanCopy]),
				lenAddBuffer - numberOfdataCanCopy);
	}
	UARTTxDumpBuffer(uart);

}

void DynamixelProtocal2(uint8_t *Memory, uint8_t MotorID, int16_t dataIn,
		UARTStucrture *uart)
{
	//all Static Variable
	//	static DNMXPState State = DNMXP_idle;
	static uint16_t datalen = 0;
	static uint16_t CollectedData = 0;
	static uint8_t inst = 0;
	static uint8_t parameter[256] =
	{ 0 };
	static uint16_t CRCCheck = 0;
	static uint16_t packetSize = 0;
	static uint16_t CRC_accum;


	//Pj. Var.
	A = dataIn ;


	//	Pj.State Machine
	if(dataIn == 999)
	{
		B = 25;
		uint8_t temp[] = {0x46,0x6E};
		UARTTxWrite(uart, temp,2);
	}
	else
	{
		switch (State)
		{
		case CheckACK1:
			if ((dataIn &0xFF) == 0x58)
				State = CheckACK2 ;
		case CheckACK2:
			if ((dataIn &0xFF) == 0x75)
				ACK = 0;
			State = S_idle ;

		case S_idle:
			if (ACK == 1)
			{
				State = CheckACK1;

			}
			else
			{
				if (((dataIn >> 4) & 0xff) == 0x09)
				{
					START = dataIn;
					if ((dataIn &0x0F) == 0x02 || (dataIn &0x0F) == 0x03 || (dataIn &0x0F) >= 0x08) //case 2,3,8-14 Frame#1
					{
						if((dataIn &0x0F) == 0x09 ||(dataIn &0x0F) == 0x0A || (dataIn &0x0F) == 0x0B)
						{
							MODE = dataIn &0x0F ;
							State = S_Checksum1_2;
						}
						else
						{
							MODE = dataIn &0x0F ;
							State = S_Checksum1;
						}

					}
					else if ((dataIn &0x0F) == 0x01 || ((dataIn &0x0F) == 0x06)) //case 1,6 Frame#2
					{
						MODE = dataIn &0x0F ;
						State = S_Frame2_DataFrame_1;
					}
					else if ((dataIn &0x0F) == 0x04) //case 4 Frame#2
					{
						MODE = dataIn &0x0F ;
						State = S_Frame2_DataFrame_1;
					}
					else if ((dataIn &0x0F) == 0x05) //case 5 Frame#2
					{
						MODE = dataIn &0x0F ;
						State = S_Frame2_DataFrame_Mode5_1;
					}
					else if ((dataIn &0x0F) == 0x07) //case 7 Frame#3
					{
						MODE = dataIn &0x0F ;
						State = S_Frame3_Station;
					}
					else
					{
						State = S_idle ;
					}
				}
				else
				{
					State = S_idle ;
				}
			}
			break;

		case S_Frame2_DataFrame_1 :
			DATAFRAME[CollectedData] = dataIn &0xff;
			CollectedData++;
			State = S_Frame2_DataFrame_2;
			break;

		case S_Frame2_DataFrame_2:
			DATAFRAME[CollectedData] = dataIn &0xff;
			vMax = DATAFRAME[CollectedData];
			CollectedData++;
			State = S_Checksum2 ;
			break;

			//	case S_Frame2_DataFrame_Mode4_1 :
			//		DATAFRAME[CollectedData] = dataIn &0xff;
			//		CollectedData++;
			//		State = S_Frame2_DataFrame_Mode4_2;
			//		break;
			//
			//	case S_Frame2_DataFrame_Mode4_2:
			//		DATAFRAME[CollectedData] = dataIn &0xff;
			//		CollectedData++;
			//		State = S_Checksum2_4 ;
			//		break;

		case S_Frame2_DataFrame_Mode5_1 :
			DATAFRAME[CollectedData] = dataIn &0xff;
			finalAngle = DATAFRAME[CollectedData];
			CollectedData++;
			State = S_Frame2_DataFrame_Mode5_2;
			break;

		case S_Frame2_DataFrame_Mode5_2:
			DATAFRAME[CollectedData] = dataIn &0xff;
			finalAngle = (Pos_Data << 8) | DATAFRAME[CollectedData];
			CollectedData++;
			State = S_Checksum2 ;
			break;

		case S_Frame3_Station:
			STATION = dataIn &0xff;
			C_Station = STATION;
			DATA = (STATION) &0xff;
			if(DATA % 2 == 0) 				//EVEN
			{
				DATA_Byte = (DATA/2);
			}
			else							//odd
			{
				DATA_Byte = (DATA+1)/2;
			}
			State = S_Frame3_DataFrame_2;
			break;


		case S500 :
			DATAFRAME[CollectedData] = dataIn &0xff;
			CollectedData++;
			State = S600;
			break;
		case S600 :
			DATAFRAME[CollectedData] = dataIn &0xff;
			CollectedData++;
			State = S700;
			break;
		case S700 :
			DATAFRAME[CollectedData] = dataIn &0xff;
			CollectedData++;
			State = S_Checksum3;
			break;

		case S_Jump :
			State = S_Checksum3;
			break;

		case  S_Frame3_DataFrame_2:
		{
			if (x < DATA_Byte)
			{
				x++;
				S = dataIn &0xff;
				DATA_N_SUM += S;
				DATAFRAME[CollectedData] = S;
				CollectedData++;

			}
			else
			{
				B+=1;
			}
			State = S_Checksum3;
			break;
		}

		case S_Checksum1_2:
			CHECKSUM = dataIn & 0xff ;
			CHECK_SUM1 = ~((0x9 << 4) | MODE );
			if (CHECK_SUM1 == CHECKSUM)
			{
				switch (MODE)
				{
				case 0b1001: //9
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					CHECK_SEND = ~ (0x99 + (DATAFRAME[CollectedData-2]) + (DATAFRAME[CollectedData-1]));
					uint8_t FRAME2[] = {0x99,(DATAFRAME[CollectedData-2]),(DATAFRAME[CollectedData-1]),CHECK_SEND};
					UARTTxWrite(uart, FRAME2, 4);
					break;
				}
				case 0b1010: //10
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					CHECK_SEND = ~(0x9A + (DATAFRAME[CollectedData-2]) + (DATAFRAME[CollectedData-1]));
					uint8_t FRAME2[] = {0x9A,(DATAFRAME[CollectedData-2]),(DATAFRAME[CollectedData-1]),CHECK_SEND};
					UARTTxWrite(uart, FRAME2, 4);
					break;
				}
				case 0b1011: //11
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					CHECK_SEND = ~(0x9B + (DATAFRAME[CollectedData-2]) + (DATAFRAME[CollectedData-1]));
					uint8_t FRAME2[] = {0x9B,(DATAFRAME[CollectedData-2]),(DATAFRAME[CollectedData-1]),CHECK_SEND};
					UARTTxWrite(uart, FRAME2, 4);
					break;
				}
				}

			}
			else
			{
				uint8_t temp[] = {START,0x75,CHECKSUM};
				UARTTxWrite(uart, temp, 3);
			}
			ACK = 1;
			State = S_idle ;
			break;

		case S_Checksum1:
			CHECKSUM = dataIn & 0xff ;
			CHECK_SUM1 = ~((0x9 << 4) | MODE );
			if (CHECK_SUM1 == CHECKSUM)
			{
				switch (MODE)
				{
				case 0b0010: //2
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					Connect = 1;
					State = S_idle ;
					break;
				}
				case 0b0011: //3
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					Connect = 0;
					State = S_idle ;
					break;
				}
				case 0b1000: //8
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp,2);
					HAL_Delay(5000);
					start = 1;
					State = S_idle;
					break;
				}
				case 0b1100: //12
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					Gripper = 1;
					State = S_idle ;
					break;
				}
				case 0b1101: //13
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					Gripper = 0;
					State = S_idle ;
					break;
				}
				case 0b1110: //14
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					State = S_idle ;
					break;
				}
				}

			}
			else
			{
				uint8_t temp[] = {START,0x75,CHECKSUM};
				UARTTxWrite(uart, temp, 3);
				State = S_idle ;
			}
			break;

		case S_Checksum2:
			CHECKSUM = dataIn & 0xff ;
			CHECK_SUM3 = ~( ((0x9 << 4) | MODE) + STATION + DATA_N_SUM);
			CHECK_SUM1 = ~( ((0x9 << 4) | MODE) + ((DATAFRAME[CollectedData-1]) + (DATAFRAME[CollectedData-2])) );
			if (CHECK_SUM1 == CHECKSUM)
			{
				switch (MODE)
				{
				case 0b0001: //1
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					break;
				}
				case 0b0100: //4
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					break;
				}
				case 0b0101: //5
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					break;
				}
				case 0b0110: //6
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					break;
				}
				case 0b0111: //7
				{
					uint8_t temp[] = {0x58,0x75};
					UARTTxWrite(uart, temp, 2);
					break;
				}
				}
			}
			else
			{
				uint8_t temp[] = {START,0x75,CHECKSUM};
				UARTTxWrite(uart, temp, 3);
			}

			DATA_N_SUM = 0;
			x=0;
			State = S_idle;
			break;

		case S_Checksum3:
			x = 0;
			CHECK_SUM1 = ~( ((0x9 << 4) | MODE) + STATION + DATA_N_SUM);
			CHECKSUM = dataIn & 0xff ;
			if (CHECK_SUM1 == CHECKSUM)
			{
				switch (MODE)
				{
				case 0b0111: //7
				{
					uint8_t temp[] = {0x75};
					UARTTxWrite(uart, temp, 1);
					break;
				}
				}
			}
			else
			{
				uint8_t temp[] = {START,0x75,CHECKSUM};
				UARTTxWrite(uart, temp, 3);
			}
			State = S_idle;
			break;
		}

	}
}

void I2Con(){
	const uint8_t laserAddress = 0x23<<1;
	static uint8_t pdataStart[1] = {0x45};
	HAL_I2C_Master_Transmit_IT(&hi2c1, laserAddress, pdataStart, 1);
}

void I2CprepareRead(){
	const uint8_t laserAddress = 0x23<<1;
	static uint8_t pdataStart[1] = {0x23};
	static uint8_t status[1] = {0x00};
	HAL_I2C_Master_Transmit_IT(&hi2c1, laserAddress, pdataStart, 1);
	if(hi2c1.State == HAL_I2C_STATE_READY){
		HAL_I2C_Master_Receive_IT(&hi2c1, laserAddress, status, 1);
	}
	return status;
}

uint8_t findingPosition(){
	rawPosition[0] = (float)HTIM_ENCODER.Instance->CNT*90/2048;
	if((rawPosition[0] < 10 )&& (rawPosition[1] > 80)){
		cP += 1;
	}
	else if ((rawPosition[0] > 80 )&& (rawPosition[1] < 10)){
		cP -= 1;
	}
	if (cP > 3){
		cP = 0;
	}
	else if (cP < 0){
		cP = 0;
	}
	currentPosition = (float)rawPosition[0] + cP*90;
	rawPosition[1] = rawPosition[0];
}

float EncoderVelocity_Update()
{
	//Save Last state
	static uint32_t EncoderLastPosition = 0;
	static uint64_t EncoderLastTimestamp = 0;

	//read data
	uint32_t EncoderNowPosition = HTIM_ENCODER.Instance->CNT;
	uint64_t EncoderNowTimestamp = micros();

	int32_t EncoderPositionDiff;
	uint64_t EncoderTimeDiff;

	EncoderTimeDiff = EncoderNowTimestamp - EncoderLastTimestamp;
	EncoderPositionDiff = EncoderNowPosition - EncoderLastPosition;

	//compensate overflow and underflow
	if (EncoderPositionDiff >= MAX_SUBPOSITION_OVERFLOW)
	{
		EncoderPositionDiff -= MAX_ENCODER_PERIOD;
	}
	else if (-EncoderPositionDiff >= MAX_SUBPOSITION_OVERFLOW)
	{
		EncoderPositionDiff += MAX_ENCODER_PERIOD;
	}

	//Update Position and time
	EncoderLastPosition = EncoderNowPosition;
	EncoderLastTimestamp = EncoderNowTimestamp;

	//Calculate velocity
	//EncoderTimeDiff is in uS
	return (EncoderPositionDiff * 1000000*2*3.14) / (float) (EncoderTimeDiff *2048*4);

}

void trajectory(uint64_t Timestamp){
	static float a0 = 0;
	static float a1 = 0;
	static float a2 = 0;
	static float a3 = 0;
	static uint64_t setTime = 0;
	state[0] = start;
	rawPosition[0] = HTIM_ENCODER.Instance->CNT*90/2048;
	if((rawPosition[0] < 10 )&& (rawPosition[1] > 87)){
		cP += 1;
	}
	else if ((rawPosition[0] >87 )&& (rawPosition[1] < 10)){
		cP -= 1;
	}
	if (cP > 3){
		cP = 0;
	}
	else if (cP < 0){
		cP = 0;
	}
	currentPosition = rawPosition[0] + cP*90;
	if(state[0] == 1){
		if(state[0] != state[1]){
			setTime = Timestamp;
			startAngle = currentPosition;
		}
		tim = (float) (Timestamp-setTime)/1000000;
		tF = (float) (250*abs(finalAngle-startAngle-storeAngle)/(355*vMax));
		a0 = startAngle;
		a1 = 0;
		a2 = (float) (3/pow(tF,2))*(finalAngle-startAngle);
		a3 = (float) -(2/pow(tF,3))*(finalAngle-startAngle);
		position = (float) a0+ (a1*tim) +(a2*pow(tim,2)) +(a3*pow(tim,3));
		velocity = (float) (a1 +(2*a2*tim) +(3*a3*pow(tim,2)))/6;
		alpha = (float) ((2*a2) +(6*a3*tim))*2*3.14/360;
		if ((abs(currentPosition - finalAngle) < 2)||(tim >= tF)){
			start = 0;
			velocity = 0;
			stopTime = Timestamp;
			DynamixelProtocal2(MainMemory, 1,999, &UART2);
			if(Gripper == 1){
				I2Con();
			}
		}
	}
	rawPosition[1] = rawPosition[0];
}

void pidPosition(){
	static float errorP = 0;
	static float integralP = 0;
	static float derivativeP = 0;
	errorP = position - currentPosition;
	integralP = integralP+errorP;
	velocity = Kp_p*errorP + Ki_p*integralP +Kd_p*(errorP-derivativeP);
	derivativeP = errorP;
	if (velocity > vMax){
		velocity = vMax;
	}
	else if (velocity < -vMax){
		velocity = -vMax;
	}

	if (start == 0){
		velocity = 0;
		errorP = 0;
		integralP = 0;
		derivativeP = 0;
	}
}

void piVelocity(){
	static float error = 0;
	static float integral = 0;
	static float derivative = 0;
	if (velocity == 0){
		PWMOut = 0;
		error = 0;
		integral = 0;
		derivative = 0;
	}
	else{

		error = abs(velocity) - abs(EncoderVel);
		integral = integral+error;
		PWMOut = K + Kp*error + Ki*integral +Kd*(error-derivative);
		derivative = error;
	}
	if (abs(PWMOut) > 10000){
		PWMOut = 10000;
	}
	if(velocity < 0){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, RESET);
	}
	else if (velocity > 0){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, RESET);
	}
	htim3.Instance->CCR1 = abs(PWMOut);
	state[1] = state[0];
}
void gotoSethome(){
	if(SetHome == 1){
		if (currentPosition < 70){
			velocity = -1.5;
		}
		else if(currentPosition > 70){
			velocity = 1.5;
		}
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	}
	piVelocity();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_7){
		cP = 0;
		velocity = 0;
		SetHome = 0;
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	}
}

void kalman(){
	static float omega = 0;
	static float omegaPredict = 0;
	static float omegaPredictPre = 0;
	static float P11predict = 0;
	static float P12predict = 0;
	static float P21predict = 0;
	static float P22predict = 0;
	static float P11predictPRE = 0;
	static float P12predictPRE = 0;
	static float P21predictPRE = 0;
	static float P22predictPRE = 0;
	static float errorVel = 0;
	static float delt = 0;

	delt = (float) dt/1000000;
	omegaPredict =  (float) omegaPredictPre;
	errorVel = (float) EncoderVelocity_Update() - omegaPredict;

	P11predict = (float) P11predictPRE+delt*P21predictPRE+(pow(Gl,2)*pow(delt,4))/4+(pow(delt,2)*(P12predictPRE+delt*P22predictPRE))/delt;
	P12predict = (float) P12predictPRE+delt*P22predictPRE+(pow(Gl,2)*delt*pow(delt,2))/2;
	P21predict = (float) (2*delt*P21predictPRE+pow(Gl,2)*pow(delt,4)+2*P22predictPRE*pow(delt,2))/(2*delt);
	P22predict = (float) pow(Gl,2)*pow(delt,2) +P22predictPRE;

	omega = (float) omegaPredict + (P22predict*errorVel)/(pow(R,2)+P22predict);
	P11predictPRE = (float) P11predict - (P12predict*P21predict)/(pow(R,2)+P22predict);
	P12predictPRE = (float) P12predict - (P12predict*P22predict)/(pow(R,2)+P22predict);
	P21predictPRE = (float) P21predict*(P22predict/(pow(R,2)+P22predict)-1);
	P22predictPRE = (float) P22predict*(P22predict/(pow(R,2)+P22predict)-1);
	omegaPredictPre = omega;
	EncoderVel = omega/0.10472;
	if (velocity == 0){
		P11predict = 0;
		P12predict = 0;
		P21predict = 0;
		P22predict = 0;
		P11predictPRE = 0;
		P12predictPRE = 0;
		P21predictPRE = 0;
		P22predictPRE = 0;
		omegaPredict = 0;
		omegaPredictPre = 0;
		errorVel = 0;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim2)
	{
		_micros += 4294967295;
	}
}
uint64_t micros()
{
	return _micros + htim2.Instance->CNT;
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
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
