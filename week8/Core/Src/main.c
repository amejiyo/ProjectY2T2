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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
int32_t PWMOut = 5000;
uint64_t _micros = 0;
uint8_t cP = 0;
float outputVel = 0;
float EncoderVel = 0;
float sumVel = 0;
float preVel = 0;
float velocity = 0;
float position = 0;
float alpha = 0;
float tim = 0;
uint8_t check = 0;
_Bool SetHome = 0;
uint64_t setTime = 0;
float startAngle = 0;
float stopTime = 0;
uint8_t storeAngle = 0;
float finalAngle = 30;
float currentPosition = 0;
uint16_t rawPosition[2] = {0};
float accerelation = 0;
uint8_t state[2] = {0};
_Bool start = 0;
uint16_t dt = 10000;
uint64_t TimeOutputLoop = 0;
uint64_t Timestamp = 0;
float vMax = 10;
uint16_t Kp = 250;
float Ki = 2.5;
uint16_t Kd = 200;
uint16_t k = 5000;
float R = 0.2;
float Gl = 10;
static float tF = 0;
float omegaPredict = 0;
float omegaPredictPre = 0;
float P11predict = 0;
float P12predict = 0;
float P21predict = 0;
float P22predict = 0;
float P11predictPRE = 0;
float P12predictPRE = 0;
float P21predictPRE = 0;
float P22predictPRE = 0;
float errorVel = 0;
float errorPos = 0;
float delt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
uint64_t micros();
#define  HTIM_ENCODER htim1
float EncoderVelocity_Update();
void pidVelocity();
void trajectory(uint64_t);
void kalman();
void gotoSethome();
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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	//start micros
	HAL_TIM_Base_Start_IT(&htim2);
	//Encoder start
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
	//PWM start AIN1
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	htim3.Instance->CCR1 = 5000;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		gotoSethome();
		if (micros() - Timestamp >= dt){
			Timestamp = micros();
			trajectory(Timestamp);
			kalman();
			pidVelocity();
			if(velocity < 0){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			}
			else if (velocity > 0){
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, SET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, RESET);
			}
			htim3.Instance->CCR1 = abs(PWMOut);
			state[1] = state[0];
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
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
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
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

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

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
#define  MAX_SUBPOSITION_OVERFLOW 1536
#define  MAX_ENCODER_PERIOD 2048

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_7){
		check ++;
		cP = 0;
		velocity = 0;
		SetHome = 0;
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	}
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
	return (EncoderPositionDiff * 1000000*60) / (float) (EncoderTimeDiff *2048*4);

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
	if(abs(finalAngle-startAngle)<10){
		vMax = 5;
		k = 1000;
	}
	else if (abs(finalAngle-startAngle)<50){
		vMax = 5;
		k = 2000;
	}
	else{
		vMax = 8;
		k = 2000;
		if(abs(finalAngle-startAngle)>200){
			vMax = 10;
			k = 5000;
		}
	}
	if(state[0] == 1){
		if(state[0] != state[1]){
			setTime = Timestamp;
			startAngle = currentPosition;
//			if(abs(finalAngle-startAngle) > 200 ){
//				storeAngle = 10;
//			}
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
		}
	}
	rawPosition[1] = rawPosition[0];
}

void pidVelocity(){
	static float error = 0;
	static float integral = 0;
	static float derivative = 0;
	error = abs(velocity) - abs(EncoderVel);
	integral = integral+error;
	PWMOut = k + Kp*error + Ki*integral +Kd*(error-derivative);
	derivative = error;
	if (abs(PWMOut) > 10000){
		PWMOut = 10000;
	}

	if (velocity == 0){
		PWMOut = 0;
		error = 0;
		integral = 0;
		derivative = 0;
	}
}

void gotoSethome(){
	if (SetHome == 1){
		velocity = 3;
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	}
}
void kalman(){
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

	EncoderVel = (float) omegaPredict + (P22predict*errorVel)/(pow(R,2)+P22predict);
	P11predictPRE = (float) P11predict - (P12predict*P21predict)/(pow(R,2)+P22predict);
	P12predictPRE = (float) P12predict - (P12predict*P22predict)/(pow(R,2)+P22predict);
	P21predictPRE = (float) P21predict*(P22predict/(pow(R,2)+P22predict)-1);
	P22predictPRE = (float) P22predict*(P22predict/(pow(R,2)+P22predict)-1);
	omegaPredictPre = EncoderVel;
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
