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
 ******************************************************************************
 */

#include "function.h"
#include "lcd.h"
#include "string.h"

// Initialize game state
void initGameState(DinoGameState *state) {
    state->dinoX = GROUND_PAGE - GROUND_OFFSET; // Start 2 page above ground (page 5)
    state->dinoY = 8;  // Leftmost position
    state->dinoState = 0;  // Running
    state->animFrame = 0;
    state->jumpHeight = 0;
    state->isJumping = 0;
    state->isCrouching = 0;
    state->jumpVelocity = 0;
    state->jumpFrameCounter = 0;
    state->buttonHeld = 0;  // Button not held initially
    state->lives = 1;  // Default 1 life
    state->score = 0;
    state->currentSpeed = OBSTACLE_SPEED_INIT;  // Start with initial speed
    state->speedTimer = 0;  // Reset speed timer
}

// Draw the dino at current state position
void drawDino(DinoGameState *state) {
    unsigned char sprite[2];  // Array for 16x16 sprite (2 chars wide)
    
    // Select sprite based on state
    // 16x16 sprites use 2 consecutive indices (e.g., 125 and 126)
    if (state->isCrouching) {
        // Alternate between crouch frames for crouching animation
        if (state->animFrame % 8 < 4) {
            sprite[0] = SPRITE_DINO_CROUCH;      // Index 144
            sprite[1] = SPRITE_DINO_CROUCH + 1;  // Index 145
        } else {
            sprite[0] = SPRITE_DINO_CROUCH_2;      // Index 146
            sprite[1] = SPRITE_DINO_CROUCH_2 + 1;  // Index 147
        }
    } else if (state->isJumping) {
        sprite[0] = SPRITE_DINO_STAND;      // Index 125
        sprite[1] = SPRITE_DINO_STAND + 1;  // Index 126
    } else {
        // Alternate between run frames for running animation
        if (state->animFrame % 8 < 4) {
            sprite[0] = SPRITE_DINO_RUN;      // Index 127
            sprite[1] = SPRITE_DINO_RUN + 1;  // Index 128
        } else {
            sprite[0] = SPRITE_DINO_RUN_2;      // Index 129
            sprite[1] = SPRITE_DINO_RUN_2 + 1;  // Index 130
        }
    }
    
    // Draw the dino (16x16 sprite using 2 consecutive 8x16 chars)
    LCD_DrawString(state->dinoX, state->dinoY, sprite, 2);
}

// Draw dead dino sprite at current position
void drawDinoDead(DinoGameState *state) {
    unsigned char sprite[2];
    sprite[0] = SPRITE_DINO_DEAD;      // Index 131
    sprite[1] = SPRITE_DINO_DEAD + 1;  // Index 132
    LCD_DrawString(state->dinoX, state->dinoY, sprite, 2);
}

// Draw dino hit sprite at current position (when losing a life but not dead)
void drawDinoHit(DinoGameState *state) {
    unsigned char sprite[2];
    sprite[0] = SPRITE_DINO_HIT;      // Index 142
    sprite[1] = SPRITE_DINO_HIT + 1;  // Index 143
    LCD_DrawString(state->dinoX, state->dinoY, sprite, 2);
}

// Update dino animation frame
void updateDinoAnimation(DinoGameState *state) {
    state->animFrame++;
    if (state->animFrame > 100) {
        state->animFrame = 0;  // Reset to prevent overflow
    }
}

// Handle jump mechanics with smooth velocity-based animation
// Uses velocity that decreases going up (deceleration) and increases going down (gravity)
// Pressing crouch during jump will cancel and fall immediately (fast-fall)
void handleJump(DinoGameState *state) {
    // Check if crouch button pressed during jump - fast-fall immediately
    if (state->isCrouching && (state->isJumping || state->jumpHeight > 0)) {
        state->isJumping = 0;
        state->jumpVelocity = 0;
        state->jumpFrameCounter = 0;
        // Fast-fall: drop all remaining height at once
        while (state->jumpHeight > 0) {
            state->jumpHeight--;
            state->dinoX++;  // Move down one page
        }
        return;
    }
    
    // Start jump with initial velocity
    if (state->isJumping && state->jumpVelocity == 0 && state->jumpHeight == 0) {
        state->jumpVelocity = JUMP_INITIAL_VELOCITY;
        state->jumpFrameCounter = 0;
    }
    
    // Process jump physics
    if (state->jumpVelocity > 0) {
        // Going up - move every (5 - velocity) frames for variable speed
        // Higher velocity = move more frequently (faster)
        state->jumpFrameCounter++;
        unsigned char framesPerMove = 6 - state->jumpVelocity;
        if (framesPerMove < 1) framesPerMove = 1;
        
        if (state->jumpFrameCounter >= framesPerMove) {
            state->jumpFrameCounter = 0;
            if (state->jumpHeight < JUMP_MAX_HEIGHT) {
                state->jumpHeight++;
                state->dinoX--;  // Move up one page
            }
            // Apply gravity - reduce upward velocity
            state->jumpVelocity -= JUMP_GRAVITY;
            
            // If velocity becomes 0 or negative, start falling
            if (state->jumpVelocity <= 0) {
                state->isJumping = 0;
                state->jumpVelocity = -1;  // Start falling slowly
            }
        }
    } else if (state->jumpVelocity < 0 || (state->jumpHeight > 0 && !state->isJumping)) {
        // Falling down - accelerate with gravity
        if (state->jumpVelocity == 0) state->jumpVelocity = -1;
        
        state->jumpFrameCounter++;
        // Falling speed increases (velocity becomes more negative)
        unsigned char framesPerMove = 5 + state->jumpVelocity;  // velocity is negative, so this decreases
        if (framesPerMove < 1) framesPerMove = 1;
        if (framesPerMove > 4) framesPerMove = 4;  // Cap slowest fall speed
        
        if (state->jumpFrameCounter >= framesPerMove) {
            state->jumpFrameCounter = 0;
            if (state->jumpHeight > 0) {
                state->jumpHeight--;
                state->dinoX++;  // Move down one page
                // Accelerate falling (make velocity more negative)
                if (state->jumpVelocity > -JUMP_INITIAL_VELOCITY) {
                    state->jumpVelocity -= JUMP_GRAVITY;
                }
            } else {
                // Landed
                state->jumpVelocity = 0;
                state->jumpFrameCounter = 0;
            }
        }
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

// Draw a flying bird with animation
void drawBird(unsigned char x, unsigned char y, unsigned char animFrame) {
    unsigned char sprite[2];
    // Alternate between two bird frames for flapping animation
    if (animFrame % 8 < 4) {
        sprite[0] = SPRITE_BIRD_FLY_1;
        sprite[1] = SPRITE_BIRD_FLY_1 + 1;
    } else {
        sprite[0] = SPRITE_BIRD_FLY_2;
        sprite[1] = SPRITE_BIRD_FLY_2 + 1;
    }
    LCD_DrawString(x, y, sprite, 2);
}

// Draw a star decoration
void drawStar(unsigned char x, unsigned char y) {
    unsigned char sprite[2] = {SPRITE_STAR, SPRITE_STAR + 1};
    LCD_DrawString(x, y, sprite, 2);
}

// Draw a moon decoration
void drawMoon(unsigned char x, unsigned char y) {
    unsigned char sprite[2] = {SPRITE_MOON, SPRITE_MOON + 1};
    LCD_DrawString(x, y, sprite, 2);
}

// Draw ground line (full width)
void drawGroundLine(unsigned char page) {
    // Draw a continuous line across the entire width at GROUND_PAGE
    // Use SPRITE_GROUND_LINE (135) which has the line in the bottom byte
    unsigned char sprite[1] = {SPRITE_GROUND_LINE};
    for (unsigned char i = 0; i < 16; i++) {  // 128 pixels / 8 = 16 sprites
        LCD_DrawString(page, i * 8, sprite, 1);
    }
}

// Draw ground line from startCol to endCol (for partial drawing / animation)
// startCol and endCol are in pixels (0-127)
void drawGroundLinePartial(unsigned char page, unsigned char startCol, unsigned char endCol) {
    unsigned char sprite[1] = {SPRITE_GROUND_LINE};
    unsigned char blank[1] = {22};  // Blank sprite
    
    // Calculate which 8-pixel blocks to draw
    unsigned char startBlock = startCol / 8;
    unsigned char endBlock = endCol / 8;
    if (endBlock > 15) endBlock = 15;  // Clamp to 16 blocks max
    
    // Draw ground line sprites in the specified range
    for (unsigned char i = startBlock; i <= endBlock; i++) {
        LCD_DrawString(page, i * 8, sprite, 1);
    }
}

// Clear the ground line on a page
void clearGroundLine(unsigned char page) {
    unsigned char blank[1] = {22};  // Blank sprite
    for (unsigned char i = 0; i < 16; i++) {
        LCD_DrawString(page, i * 8, blank, 1);
    }
}

// Animate ground line entry from right to left (blocking animation for start screen)
// This creates a cool starting effect where the ground "rolls in" from the right
void animateGroundLineEntry(unsigned char page) {
    unsigned char sprite[1] = {SPRITE_GROUND_LINE};
    
    // Start from rightmost position and draw progressively to the left
    // Each step adds one more 8-pixel block from the right
    for (int col = 15; col >= 0; col--) {
        // Draw from current column to the right edge
        for (int i = col; i < 16; i++) {
            LCD_DrawString(page, i * 8, sprite, 1);
        }
        HAL_Delay(30);  // Animation delay between frames
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

// Draw game score in upper right corner of LCD
// LCD is 128 pixels wide, score at page 0 (top), right-aligned
void drawGameScore(unsigned int score) {
    // First clear the score area (up to 5 digits = 40 pixels)
    unsigned char blank[5] = {22, 22, 22, 22, 22};  // Index 22 is blank
    LCD_DrawString(0, 88, blank, 5);  // Clear from column 88 to 128
    
    // Draw score right-aligned at upper right
    // Calculate starting position based on number of digits
    unsigned char numDigits = 1;
    unsigned int temp = score;
    while (temp >= 10 && numDigits < 5) {
        temp /= 10;
        numDigits++;
    }
    
    // Position: 128 - (numDigits * 8) for right alignment
    unsigned char startY = 128 - (numDigits * 8);
    drawScore(score, 0, startY);
}

// Draw "START" text in the middle of the LCD
// ChineseTable indices: S=74, T=75, A=56, R=73, T=75
void drawStartScreen(void) {
    // "START" = 5 characters, each 8 pixels wide = 40 pixels
    // LCD is 128 pixels wide, center at (128-40)/2 = 44
    // Middle page is 3 or 4 (LCD has pages 0-7)
    unsigned char startText[5] = {74, 75, 56, 73, 75};  // S, T, A, R, T
    LCD_DrawString(3, 44, startText, 5);
}

// Clear the START text from the screen
void clearStartScreen(void) {
    unsigned char blank[5] = {22, 22, 22, 22, 22};  // Index 22 is blank
    LCD_DrawString(3, 44, blank, 5);
}

// Draw "END" text in the middle of the LCD
// ChineseTable indices: E=60, N=69, D=59
void drawEndScreen(void) {
    // "END" = 3 characters, each 8 pixels wide = 24 pixels
    // LCD is 128 pixels wide, center at (128-24)/2 = 52
    unsigned char endText[3] = {60, 69, 59};  // E, N, D
    LCD_DrawString(3, 52, endText, 3);
}

// Clear the END text from the screen
void clearEndScreen(void) {
    unsigned char blank[3] = {22, 22, 22};  // Index 22 is blank
    LCD_DrawString(3, 52, blank, 3);
}

// Update LEDs to show number of lives (1-4)
void updateLivesLED(unsigned char lives) {
    // LED1 = life 1, LED2 = life 2, etc.
    // Turn ON LEDs for each life, OFF for the rest
    HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, (lives >= 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, (lives >= 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, (lives >= 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, (lives >= 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Update game speed - gradually increases pace over time
// Uses frame-based control: currentSpeed = frames between obstacle moves
// This function should be called every frame
void updateGameSpeed(DinoGameState *state) {
    state->speedTimer++;
    
    // Check if it's time to increase speed
    if (state->speedTimer >= SPEED_INCREASE_RATE) {
        state->speedTimer = 0;
        
        // Decrease obstacle speed (lower = faster movement)
        if (state->currentSpeed > OBSTACLE_SPEED_MIN) {
            state->currentSpeed--;
        }
    }
}
