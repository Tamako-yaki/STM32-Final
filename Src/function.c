/**
 ******************************************************************************
 * @file    function.c
 * @brief   Chrome Dino Game - Game mechanics and sprite rendering
 * @author  Your Name
 * @date    November 20, 2025
 ******************************************************************************
 * 
 * CHROME DINO GAME IMPLEMENTATION GUIDE
 * ======================================
 * 
 * This file contains all the game mechanics and sprite rendering functions
 * for a Chrome Dino-style endless runner game on the STM32 EK-STM3210E LCD.
 * 
 * AVAILABLE SPRITES (defined in lcd.c ChineseTable):
 * --------------------------------------------------
 * - SPRITE_DINO_STAND    (125-126): Dino standing/jumping pose
 * - SPRITE_DINO_RUN_1    (127-128): Dino running animation frame 1
 * - SPRITE_DINO_RUN_2    (129-130): Dino running animation frame 2
 * - SPRITE_DINO_DUCK_1   (131-132): Dino ducking pose
 * - SPRITE_CACTUS_BIG    (120-121): Large cactus obstacle
 * - SPRITE_CACTUS_SMALL  (122):     Small cactus obstacle
 * - SPRITE_CLOUD         (123-124): Cloud decoration
 * - SPRITE_BIRD_FLY_1    (134-135): Flying bird animation frame 1
 * - SPRITE_BIRD_FLY_2    (136-137): Flying bird animation frame 2
 * - SPRITE_GROUND_LINE   (133):     Ground line decoration
 * 
 * EXAMPLE USAGE IN main.c:
 * ------------------------
 * 
 *   // Initialize game
 *   DinoGameState gameState;
 *   initGameState(&gameState);
 *   
 *   // Draw initial ground
 *   drawGroundLine(0);
 *   
 *   // Game loop
 *   while(1) {
 *       // Clear old dino position
 *       clearSprite(gameState.dinoX, gameState.dinoY, 2);
 *       
 *       // Handle jump (trigger with button press)
 *       if (buttonPressed && !gameState.isJumping && gameState.jumpHeight == 0) {
 *           gameState.isJumping = 1;
 *       }
 *       handleJump(&gameState);
 *       
 *       // Update animation
 *       updateDinoAnimation(&gameState);
 *       
 *       // Draw dino at new position
 *       drawDino(&gameState);
 *       
 *       // Update and draw obstacles
 *       // (create Obstacle structs and call updateObstacle)
 *       
 *       // Delay for game speed
 *       HAL_Delay(gameState.gameSpeed);
 *       
 *       // Increment score
 *       gameState.score++;
 *       drawScore(gameState.score, 0, 100);
 *   }
 * 
 ******************************************************************************
 */

#include "function.h"
#include "lcd.h"
#include "string.h"

// Original jump function
void jump(unsigned char *a, unsigned char *b) {
    // Move from position 5 to 1 (decreasing)
    for (int j = 5; j >= 1; j--) {
        LCD_DrawString(j, 10, a, strlen(a));  // Draw string a at position j
        HAL_Delay(50);  // Delay for animation effect
        LCD_DrawString(j, 10, b, strlen(b));  // Draw string b at position j
    }
    
    // Move from position 1 to 5 (increasing)
    for (int j = 1; j <= 5; j++) {
        LCD_DrawString(j, 10, a, strlen(a));  // Draw string a at position j
        HAL_Delay(50);  // Delay for animation effect
        LCD_DrawString(j, 10, b, strlen(b));  // Draw string b at position j
    }
    
    // Final position at 5 for string a
    LCD_DrawString(5, 10, a, strlen(a));  
}

// Initialize game state
void initGameState(DinoGameState *state) {
    state->dinoX = GROUND_PAGE - 1;  // Start 1 page above ground (page 6)
    state->dinoY = 8;  // Leftmost position
    state->dinoState = 0;  // Running
    state->animFrame = 0;
    state->jumpHeight = 0;
    state->isJumping = 0;
    state->jumpHangCounter = 0;
    state->score = 0;
}

// Draw the dino at current state position
void drawDino(DinoGameState *state) {
    unsigned char sprite[2];  // Array for 16x16 sprite (2 chars wide)
    
    // Select sprite based on state
    if (state->isJumping) {
        sprite[0] = SPRITE_DINO_STAND;      // First 8x16 part
        sprite[1] = SPRITE_DINO_STAND + 1;  // Second 8x16 part
    } else {
        // Alternate between run frames for running animation
        if (state->animFrame % 20 < 10) {
            sprite[0] = SPRITE_DINO_RUN;
            sprite[1] = SPRITE_DINO_RUN + 1;
        } else {
            sprite[0] = SPRITE_DINO_RUN_2;
            sprite[1] = SPRITE_DINO_RUN_2 + 1;
        }
    }
    
    // Draw the dino (16x16 sprite using 2 consecutive 8x16 chars)
    LCD_DrawString(state->dinoX, state->dinoY, sprite, 2);
}

// Update dino animation frame
void updateDinoAnimation(DinoGameState *state) {
    state->animFrame++;
    if (state->animFrame > 100) {
        state->animFrame = 0;  // Reset to prevent overflow
    }
}

// Handle jump mechanics
void handleJump(DinoGameState *state) {
    if (state->isJumping) {
        // Going up
        if (state->jumpHeight < JUMP_MAX_HEIGHT) {
            state->jumpHeight++;
            state->dinoX--;  // Move up one page
        } else {
            // At peak - hang in the air for a few frames
            state->jumpHangCounter++;
            if (state->jumpHangCounter >= JUMP_HANG_TIME) {
                state->isJumping = 0;
                state->jumpHangCounter = 0;
            }
        }
    } else if (state->jumpHeight > 0) {
        // Coming down
        state->jumpHeight--;
        state->dinoX++;  // Move down one page
    }
}

// Draw a cactus obstacle
void drawCactus(unsigned char x, unsigned char y, unsigned char type) {
    unsigned char sprite[2];
    if (type == 0) {
        // Big cactus (16x16)
        sprite[0] = SPRITE_CACTUS_BIG;
        sprite[1] = SPRITE_CACTUS_BIG + 1;
        LCD_DrawString(x, y, sprite, 2);
    } else {
        // Small cactus (8x16)
        sprite[0] = SPRITE_CACTUS_SMALL;
        LCD_DrawString(x, y, sprite, 1);
    }
}

// Draw a cloud decoration
void drawCloud(unsigned char x, unsigned char y) {
    unsigned char sprite[2] = {SPRITE_CLOUD, SPRITE_CLOUD + 1};
    LCD_DrawString(x, y, sprite, 2);
}

// Draw ground line
void drawGroundLine(unsigned char y) {
    // Draw a continuous line across the entire width at GROUND_PAGE
    unsigned char sprite[1] = {SPRITE_GROUND_LINE};
    for (unsigned char i = 0; i < 16; i++) {  // 128 pixels / 8 = 16 sprites
        LCD_DrawString(GROUND_PAGE, i * 8, sprite, 1);
    }
}

// Clear a sprite area by drawing blank characters
void clearSprite(unsigned char x, unsigned char y, unsigned char width) {
    // Draw blank characters to clear the area
    unsigned char blank[1] = {22};  // Index 22 is blank in ChineseTable
    for (unsigned char i = 0; i < width; i++) {
        LCD_DrawString(x, y + (i * 8), blank, 1);
    }
}

// Update obstacle position (move left)
void updateObstacle(Obstacle *obs) {
    if (obs->active) {
        if (obs->y > 0) {
            // Clear old position
            clearSprite(obs->x, obs->y, 2);
            
            // Move left
            obs->y -= 8;
            
            // Draw at new position
            if (obs->type == 0 || obs->type == 1) {
                drawCactus(obs->x, obs->y, obs->type);
            }
        } else {
            // Obstacle has moved off screen
            obs->active = 0;
        }
    }
}

// Draw score using number sprites
void drawScore(unsigned int score, unsigned char x, unsigned char y) {
    // Convert score to digits and draw
    unsigned char digits[5];
    unsigned char numDigits = 0;
    unsigned int temp = score;
    
    // Extract digits
    if (temp == 0) {
        digits[0] = 0;
        numDigits = 1;
    } else {
        while (temp > 0 && numDigits < 5) {
            digits[numDigits++] = temp % 10;
            temp /= 10;
        }
    }
    
    // Draw digits (reversed order)
    for (int i = numDigits - 1; i >= 0; i--) {
        unsigned char digitSprite[1] = {digits[i]};
        LCD_DrawString(x, y + ((numDigits - 1 - i) * 8), digitSprite, 1);
    }
}

