/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   Chrome Dino Game - Main program body
  ******************************************************************************
  * 
  * CHROME DINO GAME IMPLEMENTATION
  * ================================
  * 
  * GAME CONTROLS:
  * - Connect a button to BUTTON_PORT/BUTTON_PIN (default: GPIOA, PIN_0)
  * - Press button to make the dino jump
  * - Avoid the cacti obstacles
  * - Game speeds up as score increases
  * - Press button after game over to restart
  * 
  * GAME FEATURES:
  * - Animated running dino
  * - Jump physics
  * - Multiple obstacle types (big/small cacti)
  * - Score display
  * - Automatic difficulty increase
  * - Collision detection
  * - Game over and restart
  * 
  * TO CUSTOMIZE:
  * - Change BUTTON_PIN and BUTTON_PORT in USER CODE BEGIN 0 section
  * - Adjust MAX_OBSTACLES for more/fewer obstacles
  * - Modify obstacle spawn rate in frameCount check
  * - Change game speed in GAME_SPEED_DELAY (function.h)
  * 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
#include "function.h"
#include "lcd.h"

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/** @addtogroup Templates
  * @{
  */


ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
void Error_Handler(void);
//static void MX_FSMC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// Game variables
#define MAX_OBSTACLES 3
#define BUTTON_PIN GPIO_PIN_0  // Change to your actual button pin
#define BUTTON_PORT GPIOA      // Change to your actual button port

// Timer-based frame control
extern volatile unsigned char gameTimerFlag;

Obstacle obstacles[MAX_OBSTACLES];
unsigned char obstacleSpawnCounter = 0;

/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  LCD_Init();
  LCD_Clear();
	
	/* Check TIM Init----------------------------------------------------------*/
	if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
  {
    /* Initialization Error */
    printf("Timer Broken");
  }
  
	/* Output a message on Hyperterminal using printf function */
  printf("\n\r UART Printf Example: retarget the C library printf function to the UART\n\r");
  printf("** Test finished successfully. ** \n\r");

	/* -------------------------------MAIN PROGRAM-----------------------------*/
  
  // Initialize game state
  DinoGameState game;
  initGameState(&game);
  
  // Initialize obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].active = 0;
  }
  
  // Draw initial game elements
  drawGroundLine(0);
  drawCloud(0, 20);   // Static cloud decoration at top
  drawCloud(0, 90);   // Another cloud at top
  
  unsigned int frameCount = 0;
  unsigned char gameOver = 0;

  /* Infinite loop */
  while (1)
  {
    if (!gameOver) {
      // Clear old dino position
      clearSprite(game.dinoX, game.dinoY, 2);
      
      // Check for button press (jump)
      GPIO_PinState buttonState = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
      if (buttonState == GPIO_PIN_SET) {
        if (!game.isJumping && game.jumpHeight == 0) {
          game.isJumping = 1;
        }
      }
      
      // Update dino physics
      handleJump(&game);
      
      // Update animation
      updateDinoAnimation(&game);
      
      // Draw dino at new position
      drawDino(&game);
      
      // Spawn obstacles periodically
      frameCount++;
      if (frameCount % 40 == 0) {  // Spawn every 40 frames
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (!obstacles[i].active) {
            obstacles[i].x = GROUND_PAGE - 2;
            obstacles[i].y = 120;  // Start from right side
            obstacles[i].type = (frameCount % 80 == 0) ? 1 : 0;  // Alternate obstacle types
            obstacles[i].active = 1;
            break;
          }
        }
      }
      
      // Update and draw obstacles
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
          // Clear old position
          clearSprite(obstacles[i].x, obstacles[i].y, 2);
          
          // Move obstacle left
          if (obstacles[i].y > 8) {
            obstacles[i].y -= 8;
            
            // Draw at new position
            drawCactus(obstacles[i].x, obstacles[i].y, obstacles[i].type);
            
            // Collision detection - only if dino is on ground or close to it
            // Check horizontal overlap first (Y axis = column/horizontal)
            unsigned char horizontalOverlap = (obstacles[i].y >= game.dinoY - 4 && 
                                               obstacles[i].y <= game.dinoY + 12);
            
            // Check vertical overlap (X axis = page/vertical, lower X = higher on screen)
            // Dino must be at same level or below obstacle top to collide
            unsigned char verticalOverlap = (game.dinoX >= obstacles[i].x - 1);
            
            if (horizontalOverlap && verticalOverlap) {
              // Collision! Game Over
              gameOver = 1;
            }
          } else {
            // Obstacle moved off screen
            obstacles[i].active = 0;
            // Increase score
            game.score++;
          }
        }
      }
      
      // Increase game difficulty over time
      if (game.score % 50 == 0 && game.gameSpeed > 30) {
        game.gameSpeed -= 5;
      }
      
      // Wait for timer interrupt to trigger next frame
      while (!gameTimerFlag) {
        // Wait for timer flag
      }
      gameTimerFlag = 0;  // Clear flag for next frame
      
    } else {
      // Game over state - wait for button to restart
      GPIO_PinState buttonState = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
      if (buttonState == GPIO_PIN_SET) {
        HAL_Delay(500);  // Debounce
        
        // Restart game
        LCD_Clear();
        initGameState(&game);
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          obstacles[i].active = 0;
        }
        drawGroundLine(0);
        drawCloud(1, 20);
        drawCloud(2, 90);
        frameCount = 0;
        gameOver = 0;
      }
    }
  /* USER CODE END 3 */
  }

}
/** System Clock Configuration
*/
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
	
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);

  return ch;
}
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_9B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);

}
void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_SlaveConfigTypeDef sSlaveConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7200;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000;  // ~100ms per frame (10 FPS) for slower gameplay
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&htim1);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);

  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  HAL_TIM_SlaveConfigSynchronization(&htim1, &sSlaveConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
	__GPIOA_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PF6 PF7 PF8 PF9 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* GPIO Config----------------------------------------------------------*/
	LED1_GPIO_CLK_ENABLE();
  LED2_GPIO_CLK_ENABLE();
  LED3_GPIO_CLK_ENABLE();
  LED4_GPIO_CLK_ENABLE();
	
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	GPIO_InitStruct.Pin = LED1_PIN;
  HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED2_PIN;
  HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED3_PIN;
  HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED4_PIN;
  HAL_GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStruct);
  /* Initialize BSP Led for LED3 */
  BSP_LED_Init(LED3);
	
	WAKEUP_BUTTON_GPIO_CLK_ENABLE();
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	TAMPER_BUTTON_GPIO_CLK_ENABLE();
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/