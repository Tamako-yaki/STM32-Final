# STM32 Chrome Dino Game

A Chrome dinosaur-inspired endless runner game for STM32F1 microcontrollers with LCD display, featuring animated sprites, multiple obstacle types, and UART debugging output.

## Features

- 🦖 **Animated Dino** - Running, jumping, crouching, and death animations
- 🌵 **Multiple Obstacles** - Big/small cacti and flying birds at different heights
- 🦅 **Bird Enemies** - High birds (stay grounded) and low birds (crouch to avoid)
- ⬇️ **Fast-Fall Mechanic** - Press crouch while jumping for immediate landing
- ❤️ **Lives System** - Select 1-4 lives using ADC potentiometer before game start
- 💡 **LED Indicators** - LEDs show remaining lives
- 📊 **Score Display** - Real-time score on LCD (max 999)
- 🎯 **Progressive Difficulty** - Game speed increases over time
- 🖥️ **UART Output** - Debug messages and score updates via serial terminal
- 🌄 **Animated Ground** - Scrolling terrain with varied patterns

## Hardware Requirements

- STM32F103xG microcontroller (EK-STM3210E board)
- 128x64 LCD display (ST7920 compatible)
- WAKEUP button (PA0) - Jump control
- KEY button (PB10) - Crouch control
- Potentiometer connected to ADC1 - Life selection
- 4 LEDs for life display
- UART connection for debug output (9600 baud, 8N1)

## Controls

| Button | Action |
|--------|--------|
| **WAKEUP (PA0)** | Jump / Start game / Restart after game over |
| **KEY (PB10)** | Crouch / Fast-fall (when pressed during jump) |
| **Potentiometer** | Select lives (1-4) on start screen |

## Game Mechanics

### Obstacles
- **Big Cactus** - Jump to avoid (16x16 sprite)
- **Small Cactus** - Jump to avoid (8x16 sprite)
- **High Bird** - Stay on ground! Jumping into it causes damage
- **Low Bird** - Crouch to avoid! Flies at head height

### Tips
- Jump over cacti with the WAKEUP button
- Crouch under low birds with the KEY button
- Stay grounded when high birds approach - don't jump!
- Use fast-fall (crouch while jumping) for quick landings

## Project Structure

```
Inc/
  ├── function.h          # Game constants, sprites, and API declarations
  ├── lcd.h               # LCD driver interface
  ├── main.h              # Hardware configuration and pin definitions
  ├── stm32f1xx_hal_conf.h # HAL configuration
  └── stm32f1xx_it.h      # Interrupt handlers
Src/
  ├── function.c          # Game mechanics and sprite rendering
  ├── lcd.c               # LCD driver and sprite data (ChineseTable)
  ├── main.c              # Main game loop and initialization
  ├── stm32f1xx_hal_msp.c # HAL MSP initialization
  ├── stm32f1xx_it.c      # Timer interrupt for frame timing
  └── system_stm32f1xx.c  # System clock configuration
```

## Sprite Reference

| Index | Sprite | Size |
|-------|--------|------|
| 120-121 | Big Cactus | 16x16 |
| 122 | Small Cactus | 8x16 |
| 123-124 | Star | 16x16 |
| 125-126 | Dino Standing | 16x16 |
| 127-128 | Dino Run Frame 1 | 16x16 |
| 129-130 | Dino Run Frame 2 | 16x16 |
| 131-132 | Dino Dead | 16x16 |
| 133-134 | Moon | 16x16 |
| 135-136 | Bird Frame 1 | 16x16 |
| 137-138 | Bird Frame 2 | 16x16 |
| 139-140 | Dino Hit | 16x16 |
| 141-142 | Dino Crouch Frame 1 | 16x16 |
| 143-144 | Dino Crouch Frame 2 | 16x16 |
| 145-148 | Ground Line Variations | 8x16 |

## Build & Flash

This project is designed for STM32 development environments:
- **STM32CubeIDE** (recommended)
- **Keil MDK-ARM**
- **IAR Embedded Workbench**

Configure your toolchain for STM32F103xG and flash to your board.

## UART Debug Output

Connect a serial terminal (9600 baud) to see:
- Welcome screen with control instructions
- Real-time score updates
- Hit notifications with remaining lives
- Game over summary with final score

## Customization

| Constant | File | Description |
|----------|------|-------------|
| `MAX_OBSTACLES` | main.c | Max simultaneous obstacles (default: 3) |
| `OBSTACLE_SPEED_INIT` | function.h | Initial game speed (higher = slower) |
| `OBSTACLE_SPEED_MIN` | function.h | Maximum game speed (lower = faster) |
| `JUMP_MAX_HEIGHT` | function.h | Maximum jump height in pages |
| `JUMP_INITIAL_VELOCITY` | function.h | Jump power (higher = faster start) |
| `SPEED_INCREASE_RATE` | function.h | Frames between speed increases |
| `TIMER_PERIOD_FIXED` | function.h | Frame timing (~40 = 4ms/frame) |

---

*Classic Chrome dino game reimagined for embedded systems!* 🎮🦖
