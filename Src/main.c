/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2021 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include "buffer.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  char ch = 'a';
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF); // just transmit an 'a'


  printf("\r\n ** I2C Sniffer **\r\n");
  printf("Key: \r\n");
  printf("\e[32m Left [\e[39m: (re)START \r\n");
  printf("\e[31mRight ]\e[39m:     STOP \r\n");
  printf("\r\n");
  printf("\e[36mCyan\e[39m: Address (hex) \r\n");
  printf("\e[33m   R\e[39m:   Read from master \r\n");
  printf("\e[33m   W\e[39m:  Write   to slave \r\n");
  printf("\r\n");
  printf("\e[39mWhite\e[39m: Data (hex) \r\n");
  printf("\e[33m    A\e[39m:   ACK (acknowledged) \r\n");
  printf("\e[33m    N\e[39m:  NACK (not acknowledged -- note: this is not an error) \r\n");
  printf("\r\n Enjoy! \r\n\r\n");

  // Too many variables!
  int16_t dataLeft = 0; // The number of I2C bits left to process
  uint8_t pendingData = 0; // The current byte being processed
  uint8_t pendingACK = 0; // Whether we have received ACK for the current byte
  uint8_t pendingRW = 0; // Whether a register is being Read
  uint8_t pendingExists = 0; // Whether we need to print the data
  uint8_t waitingForRegister = 0; // Whether we have received a (Re)start condition and are expecting a register address
  uint8_t dump = 0; // Whether we are dumping data due to a TIMEOUT

  uint32_t oldTimer = HAL_GetTick();

  setbuf(stdout, NULL); // Disable flushing; This might make the code slower, but makes sure everything is sent without
  	  	  	  	  	  	// having to wait for a newline
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if ((bufferPos - bufferStart + I2C_BUFFER_SIZE) % I2C_BUFFER_SIZE >= 1) { // positive modulo - distance left to cover
		  if (buffer[bufferStart] == 'A') {
			  // Start condition!!!
			  printf("\e[32m[\e[39m");
			  bufferStart = (bufferStart + 1) % I2C_BUFFER_SIZE;

			  if (dataLeft > 0 && dataLeft < 8) {
				  printf("ERROR! Not enough dat %d.\r\n", dataLeft);
			  }

			  dataLeft = 9;
			  pendingData = 0;
			  waitingForRegister = 1;
		  } else if (buffer[bufferStart] == 'B') {
			  // Stop condition!!!
			  printf("\e[31m]\e[39m\r\n");
			  bufferStart = (bufferStart + 1) % I2C_BUFFER_SIZE;

			  if (dataLeft > 0 && dataLeft < 8) {
				  printf("ERROR! Not got enough dat.\r\n");
				  dataLeft = 0;
			  }
			  pendingData = 0;
		  } else {
			  if (dataLeft <= 0) {
				  printf("ERROR! Got data without start condition.\r\n");
			  }
			  if (dataLeft > 9) {
				  printf("ERROR! Too much data expected\r\n");
			  }

			  dataLeft--;

			  if (dataLeft == 0) {
				  // Read ACK byte
				  pendingACK = !buffer[bufferStart];
				  pendingExists = 1;
			  } else if (dataLeft == 1 && waitingForRegister) {
				  // Read RW byte
				  pendingRW = buffer[bufferStart];
			  } else {
				  // Read regular byte
				  pendingData = pendingData << 1 | buffer[bufferStart];
			  }

			  // Increase the circular buffer position
			  bufferStart = (bufferStart + 1) % I2C_BUFFER_SIZE;
		  }
	  }

	  if (pendingExists) {
		  // Print received data
		  if (waitingForRegister) {
			  printf("\e[36m%2x", pendingData);
		  } else {
			  printf("\e[39m%2x", pendingData);
		  }
		  printf("\e[33m");
		  if (waitingForRegister) {
			  putchar(pendingRW ? 'R' : 'W');
			  waitingForRegister = 0;
		  }
		  putchar(pendingACK ? 'A' : 'N');

		  // Reset all the values
		  pendingExists = 0;
		  pendingData = 0;
		  dataLeft = 9;

		  oldTimer = HAL_GetTick();

		  printf("\e[39m");

		  if (dump) {
			  printf("\r\n");
			  dump = 0;
		  }
	  }

	  if (HAL_GetTick() - oldTimer > 1500) {
		  printf("\r\nNo data found (TIMEOUT), dumping information...\r\n");
		  printf("Received data: 0x%X, %d bits\r\n", pendingData, 9 - dataLeft);

		  printf("SCL line: %s\e[39m\r\n", HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin) ? "\e[32mHI" : "\e[31mLO");
		  printf("SDA line: %s\e[39m\r\n", HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin) ? "\e[32mHI" : "\e[31mLO");

		  pendingExists = dump = 1;
	  }

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 3000000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SCL_IT_Pin */
  GPIO_InitStruct.Pin = SCL_IT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SCL_IT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDA_IT_Pin */
  GPIO_InitStruct.Pin = SDA_IT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SDA_IT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	uint8_t datum;

	if (GPIO_Pin == SCL_IT_Pin) {
		// Clock triggered; bit received
		datum = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);
	} else if (HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin)) { // SDA pin necessarily
		// START or STOP condition
		// A for START, B for STOP
		datum = HAL_GPIO_ReadPin(SDA_IT_GPIO_Port, SDA_IT_Pin) ? 'B' : 'A';
	} else {
		// Nothing interesting here...
		return;
	}

	buffer[bufferPos] = datum; // Store the received bit in the buffer

	bufferPos = (bufferPos + 1) % I2C_BUFFER_SIZE;
	if (bufferPos == bufferStart) printf("ERROR! I2C buffer too small!\r\n"); // Buffer overflow!
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
