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
  * - Change game speed with OBSTACLE_SPEED in function.h
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
#include <string.h>

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
#define MAX_OBSTACLES 3  // Allow multiple obstacles on screen simultaneously
#define BUTTON_PIN GPIO_PIN_0  // Jump button (WAKEUP)
#define BUTTON_PORT GPIOA      // Jump button port
#define CROUCH_BUTTON_PIN GPIO_PIN_10  // Crouch button (USER)
#define CROUCH_BUTTON_PORT GPIOB       // Crouch button port

// Timer-based frame control
extern volatile unsigned char gameTimerFlag;

Obstacle obstacles[MAX_OBSTACLES];
unsigned char obstacleSpawnCounter = 0;
unsigned int nextObstacleSpawn = 100;  // Frame count for next spawn

// Simple pseudo-random number generator
unsigned int randomSeed = 12345;
unsigned int getRandomSpawnInterval(void) {
  // Linear congruential generator for pseudo-random numbers
  randomSeed = (randomSeed * 1103515245 + 12345) & 0x7FFFFFFF;
  // Map to range [OBSTACLE_SPAWN_MIN, OBSTACLE_SPAWN_MAX]
  return OBSTACLE_SPAWN_MIN + (randomSeed % (OBSTACLE_SPAWN_MAX - OBSTACLE_SPAWN_MIN + 1));
}

unsigned char getRandomObstacleType(void) {
  // Update random seed with additional entropy from system tick
  randomSeed = (randomSeed * 1103515245 + 12345 + HAL_GetTick()) & 0x7FFFFFFF;
  // 25% chance for each: big cactus (0), small cactus (1), high bird (2), low bird (3)
  // Use higher bits which have better randomness in LCG
  return ((randomSeed >> 16) % 4);
}

// Simple UART send functions using HAL directly (no printf dependency)
void UART_SendString(const char *str) {
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 1000);
}

void UART_SendNumber(int num) {
  char buffer[12];
  int i = 0;
  int isNegative = 0;
  
  if (num < 0) {
    isNegative = 1;
    num = -num;
  }
  
  // Handle 0 case
  if (num == 0) {
    buffer[i++] = '0';
  } else {
    // Extract digits in reverse
    while (num > 0) {
      buffer[i++] = '0' + (num % 10);
      num /= 10;
    }
  }
  
  if (isNegative) {
    buffer[i++] = '-';
  }
  
  // Reverse the buffer
  for (int j = 0; j < i / 2; j++) {
    char temp = buffer[j];
    buffer[j] = buffer[i - 1 - j];
    buffer[i - 1 - j] = temp;
  }
  
  HAL_UART_Transmit(&huart1, (uint8_t *)buffer, i, 1000);
}

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
    UART_SendString("Timer Broken\r\n");
  }
  
	/* Output a message on Hyperterminal using direct UART */
  UART_SendString("\r\n UART Direct Test - HAL_UART_Transmit\r\n");
  UART_SendString("** Test finished successfully. **\r\n");

	/* -------------------------------MAIN PROGRAM-----------------------------*/
  
  // Initialize game state
  DinoGameState game;
  initGameState(&game);
  
  // Initialize obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].active = 0;
  }
  
  // ===== START SCREEN: Select lives using ADC =====
  drawStartScreen();
  unsigned char selectedLives = 1;
  updateLivesLED(selectedLives);
  
  // Wait for button press while reading ADC to select lives
  while (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) != GPIO_PIN_SET) {
    // Read ADC value from variable resistor
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint32_t adcValue = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // Map ADC value (0-4095) to lives (1-4)
    // 0-1023 = 1 life, 1024-2047 = 2 lives, 2048-3071 = 3 lives, 3072-4095 = 4 lives
    if (adcValue < 1024) {
      selectedLives = 1;
    } else if (adcValue < 2048) {
      selectedLives = 2;
    } else if (adcValue < 3072) {
      selectedLives = 3;
    } else {
      selectedLives = 4;
    }
    
    // Update LEDs to show selected lives
    updateLivesLED(selectedLives);
    
    HAL_Delay(50);  // Small delay to avoid flickering
  }
  
  // Button pressed - start the game
  HAL_Delay(200);  // Debounce
  game.lives = selectedLives;
  
  // Seed random generator with ADC value for varied gameplay
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);
  randomSeed = HAL_ADC_GetValue(&hadc1) + HAL_GetTick();
  HAL_ADC_Stop(&hadc1);
  nextObstacleSpawn = 10;  // First obstacle spawns quickly after game start
  
  UART_SendString("\r\n=== GAME START ===\r\n");
  UART_SendString("Lives: ");
  UART_SendNumber(game.lives);
  UART_SendString("\r\n");
  
  // Clear start screen and draw game elements
  clearStartScreen();
  LCD_Clear();
  drawGroundLine(0);
  drawStar(0, 20);   // Static star decoration at top
  drawMoon(0, 90);   // Moon decoration at top
  
  unsigned int frameCount = 0;
  unsigned int obstacleFrameCounter = 0;
  unsigned char gameOver = 0;

  /* Infinite loop */
  while (1)
  {
    if (!gameOver) {
      // Clear old dino position
      clearSprite(game.dinoX, game.dinoY, 2);
      
      // Check for button press (jump) - level triggered
      GPIO_PinState buttonState = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
      if (buttonState == GPIO_PIN_SET) {
        game.buttonHeld = 1;  // Track button is being held
        if (!game.isJumping && game.jumpHeight == 0 && !game.isCrouching) {
          game.isJumping = 1;
        }
      } else {
        game.buttonHeld = 0;  // Button released
      }
      
      // Check for crouch button (TAMPER button - active LOW with pull-up)
      GPIO_PinState crouchState = HAL_GPIO_ReadPin(CROUCH_BUTTON_PORT, CROUCH_BUTTON_PIN);
      if (crouchState == GPIO_PIN_RESET && !game.isJumping && game.jumpHeight == 0) {
        game.isCrouching = 1;
      } else {
        game.isCrouching = 0;
      }
      
      // Update dino physics
      handleJump(&game);
      
      // Update animation
      updateDinoAnimation(&game);
      
      // Draw dino at new position
      drawDino(&game);
      
      // Spawn obstacles with random spacing
      frameCount++;
      if (frameCount >= nextObstacleSpawn) {
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (!obstacles[i].active) {
            obstacles[i].type = getRandomObstacleType();  // Random: 0=big, 1=small, 2=high bird, 3=low bird
            obstacles[i].y = 120;  // Start from right side
            obstacles[i].animFrame = 0;  // Reset animation frame
            
            // Set height based on obstacle type
            if (obstacles[i].type == 2) {
              // High bird - flies above dino, must NOT jump
              obstacles[i].x = BIRD_FLIGHT_PAGE;
            } else if (obstacles[i].type == 3) {
              // Low bird - flies at head level, must CROUCH
              obstacles[i].x = BIRD_LOW_FLIGHT_PAGE;
            } else {
              // Cactus on ground
              obstacles[i].x = GROUND_PAGE - 2;  // 2 pages above ground
            }
            
            obstacles[i].active = 1;
            // Set next spawn time with random interval
            nextObstacleSpawn = frameCount + getRandomSpawnInterval();
            break;
          }
        }
      }
      
      // Update and draw obstacles at dynamic speed
      obstacleFrameCounter++;
      if (obstacleFrameCounter >= game.currentSpeed) {
        obstacleFrameCounter = 0;
        
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (obstacles[i].active) {
            // Clear old position
            clearSprite(obstacles[i].x, obstacles[i].y, 2);
            
            // Move obstacle left
            if (obstacles[i].y > 8) {
              obstacles[i].y -= 8;
              obstacles[i].animFrame++;  // Update animation frame
            
              // Draw at new position based on type
              if (obstacles[i].type == 2 || obstacles[i].type == 3) {
                // Bird with animation (both high and low birds)
                drawBird(obstacles[i].x, obstacles[i].y, obstacles[i].animFrame);
              } else {
                // Cactus
                drawCactus(obstacles[i].x, obstacles[i].y, obstacles[i].type);
              }
            } else {
              // Obstacle moved off screen
              obstacles[i].active = 0;
              // Increase score and print to UART
              game.score++;
              UART_SendString("Score: ");
              UART_SendNumber(game.score);
              UART_SendString("\r\n");
            }
          }
        }
      }
      
      // Collision detection (check every frame)
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
          // Check horizontal overlap first (Y axis = column/horizontal)
          unsigned char horizontalOverlap = (obstacles[i].y >= game.dinoY - 4 && 
                                             obstacles[i].y <= game.dinoY + 12);
          
          unsigned char collision = 0;
          
          if (obstacles[i].type == 2) {
            // High bird collision: only hits dino if dino is jumping (in the air)
            // Stay on ground to avoid!
            if (horizontalOverlap && game.dinoX <= BIRD_FLIGHT_PAGE + 1) {
              collision = 1;
            }
          } else if (obstacles[i].type == 3) {
            // Low bird collision: hits dino unless crouching
            // Crouch to avoid!
            if (horizontalOverlap && !game.isCrouching && game.dinoX >= BIRD_LOW_FLIGHT_PAGE - 1) {
              collision = 1;
            }
          } else {
            // Cactus collision: only hits dino if dino is on ground (not jumping high enough)
            // Jump to avoid!
            unsigned char verticalOverlap = (game.dinoX >= obstacles[i].x - 1);
            if (horizontalOverlap && verticalOverlap) {
              collision = 1;
            }
          }
          
          if (collision) {
            // Collision! Lose a life
            game.lives--;
            UART_SendString("Hit! Lives remaining: ");
            UART_SendNumber(game.lives);
            UART_SendString("\r\n");
            updateLivesLED(game.lives);
            
            // Draw hit sprite to show collision
            clearSprite(game.dinoX, game.dinoY, 2);
            drawDinoHit(&game);
            HAL_Delay(300);  // Brief pause to show hit sprite
            clearSprite(game.dinoX, game.dinoY, 2);
            
            // Deactivate the obstacle that hit us
            obstacles[i].active = 0;
            clearSprite(obstacles[i].x, obstacles[i].y, 2);
            
            if (game.lives == 0) {
              // No more lives - Game Over
              gameOver = 1;
              UART_SendString("\r\n=== GAME OVER ===\r\n");
              UART_SendString("Final Score: ");
              UART_SendNumber(game.score);
              UART_SendString("\r\n");
              
              // Draw dead dino sprite at collision position
              drawDinoDead(&game);
              
              drawEndScreen();  // Show END text
            }
            break;
          }
        }
      }
      
      // Update lives display on LEDs
      updateLivesLED(game.lives);
      
      // Increase game difficulty over time using PWM
      updateGameSpeed(&game);
      
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
        
        // Restart game - go back to start screen
        clearEndScreen();  // Clear END text
        LCD_Clear();
        initGameState(&game);
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          obstacles[i].active = 0;
        }
        
        // Show start screen again to select lives
        drawStartScreen();
        unsigned char selectedLives = 1;
        updateLivesLED(selectedLives);
        
        // Wait for button press while reading ADC to select lives
        HAL_Delay(300);  // Wait for button release
        while (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) != GPIO_PIN_SET) {
          HAL_ADC_Start(&hadc1);
          HAL_ADC_PollForConversion(&hadc1, 100);
          uint32_t adcValue = HAL_ADC_GetValue(&hadc1);
          HAL_ADC_Stop(&hadc1);
          
          if (adcValue < 1024) {
            selectedLives = 1;
          } else if (adcValue < 2048) {
            selectedLives = 2;
          } else if (adcValue < 3072) {
            selectedLives = 3;
          } else {
            selectedLives = 4;
          }
          updateLivesLED(selectedLives);
          HAL_Delay(50);
        }
        
        HAL_Delay(200);  // Debounce
        game.lives = selectedLives;
        UART_SendString("\r\n=== GAME RESTART ===\r\n");
        UART_SendString("Lives: ");
        UART_SendNumber(game.lives);
        UART_SendString("\r\n");
        
        clearStartScreen();
        LCD_Clear();
        drawGroundLine(0);
        drawStar(0, 20);
        drawMoon(0, 90);
        frameCount = 0;
        nextObstacleSpawn = 10;  // First obstacle spawns quickly after restart
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
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
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
  htim1.Init.Period = TIMER_PERIOD_FIXED;  // Fixed fast frame rate (~4ms per frame)
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

  __HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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