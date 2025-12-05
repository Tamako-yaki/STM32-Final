/**
 ******************************************************************************
 * @file    function.h
 * @brief   Chrome Dino Game - Header file with sprite definitions and API
 ******************************************************************************
 * 
 * SPRITE DRAWING NOTES:
 * --------------------
 * - All sprites are 8x16 format (each sprite index represents 8x16 pixels)
 * - 16x16 sprites use TWO consecutive indices (e.g., 125-126)
 * - Use LCD_DrawChar(page, column, sprite_index) to draw individual sprites
 * - Page: vertical position (0-7), Column: horizontal position (0-127)
 * - Each sprite is 8 pixels wide, so spacing is typically multiples of 8
 * 
 * QUICK START:
 * -----------
 * 1. Create a DinoGameState: DinoGameState game;
 * 2. Initialize it: initGameState(&game);
 * 3. In game loop:
 *    - Clear old position: clearSprite(game.dinoX, game.dinoY, 2);
 *    - Update game logic: handleJump(&game); updateDinoAnimation(&game);
 *    - Draw new position: drawDino(&game);
 * 
 ******************************************************************************
 */

#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "main.h"
#include "lcd.h"

// Game sprite indices in ChineseTable (8x16 format)
// 16x16 sprites use 32 bytes (first 16 = left half, next 16 = right half)
#define SPRITE_CACTUS_BIG    120  // Big cactus (16x16) - indices 120-121
#define SPRITE_CACTUS_SMALL  122  // Small cactus (8x16)
#define SPRITE_STAR          123  // Star decoration (16x16) - indices 123-124
#define SPRITE_DINO_STAND    125  // Dino standing/jumping (16x16) - indices 125-126
#define SPRITE_DINO_RUN      127  // Dino running frame 1 (16x16) - indices 127-128
#define SPRITE_DINO_RUN_2    129  // Dino running frame 2 (16x16) - indices 129-130
#define SPRITE_DINO_DEAD     131  // Dino dead sprite (16x16) - indices 131-132
#define SPRITE_CLEAR         133  // Clear sprite (16x16) - indices 133-134
#define SPRITE_GROUND_LINE   136  // Ground line (8x16) - index 136 has the line pixels
#define SPRITE_MOON          139  // Moon decoration (16x16) - indices 139-140

// Game constants
#define GROUND_PAGE          7    // The page/row where ground is drawn (bottom of LCD)
#define DINO_GROUND_Y        64   // Dino's Y position when on ground
#define JUMP_MAX_HEIGHT      3    // Maximum jump height in pages
#define JUMP_HANG_TIME_MIN   8    // Normal hang time at jump peak (when button released)
#define JUMP_HANG_TIME_MAX   20   // Maximum hang time at jump peak (when button held)
#define OBSTACLE_SPEED_INIT  5    // Initial frames between obstacle movements (higher = slower)
#define OBSTACLE_SPEED_MIN   1    // Minimum obstacle speed (fastest)
#define SPEED_INCREASE_RATE  150  // Frames between speed increases

// PWM Timer period constant (fixed fast frame rate)
#define TIMER_PERIOD_FIXED   40   // Fixed timer period (~4ms per frame, ~250 FPS)

// Obstacle spawn interval constants (frames between spawns)
#define OBSTACLE_SPAWN_MIN   40   // Minimum frames between obstacle spawns
#define OBSTACLE_SPAWN_MAX   100  // Maximum frames between obstacle spawns

// Game state and animation variables
typedef struct {
    unsigned char dinoX;          // Dino X position (page)
    unsigned char dinoY;          // Dino Y position (column)
    unsigned char dinoState;      // 0=running, 1=jumping, 2=ducking
    unsigned char animFrame;      // Animation frame counter
    unsigned char jumpHeight;     // Current jump height
    unsigned char isJumping;      // Jump state flag
    unsigned char jumpHangCounter; // Counter for hang time at peak
    unsigned char buttonHeld;     // Whether jump button is being held
    unsigned char lives;          // Number of lives (1-4)
    unsigned int score;           // Current game score
    unsigned char currentSpeed;   // Current obstacle speed (frames between moves)
    unsigned int speedTimer;      // Timer for speed increases
} DinoGameState;

// Obstacle structure
typedef struct {
    unsigned char x;              // X position (page)
    unsigned char y;              // Y position (column)
    unsigned char type;           // 0=cactus big, 1=cactus small, 2=bird
    unsigned char active;         // Is obstacle active
} Obstacle;

// Game functions
void drawDino(DinoGameState *state);
void drawDinoDead(DinoGameState *state);  // Draw dead dino sprite
void updateDinoAnimation(DinoGameState *state);
void drawCactus(unsigned char x, unsigned char y, unsigned char type);
void drawStar(unsigned char x, unsigned char y);
void drawMoon(unsigned char x, unsigned char y);
void drawGroundLine(unsigned char y);
void clearSprite(unsigned char x, unsigned char y, unsigned char width);
void initGameState(DinoGameState *state);
void handleJump(DinoGameState *state);
void updateObstacle(Obstacle *obs);
void drawScore(unsigned int score, unsigned char x, unsigned char y);
void drawStartScreen(void);
void clearStartScreen(void);
void drawEndScreen(void);
void clearEndScreen(void);
void updateLivesLED(unsigned char lives);
void updateGameSpeed(DinoGameState *state);  // PWM-based speed control

#endif /* __FUNCTION_H */