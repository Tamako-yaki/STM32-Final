# STM32 Chrome Dino Game

A Chrome dinosaur-inspired running game implementation for STM32F1 microcontrollers with LCD display.

## Features

- 🦖 Animated running dinosaur with jump mechanics
- 🌵 Randomly generated cactus obstacles
- 📊 Score tracking with automatic difficulty increase
- 🎮 Simple one-button control
- 💥 Collision detection and game over state

## Hardware Requirements

- STM32F1xx microcontroller (e.g., STM32F103)
- LCD display (ST7920 or compatible)
- Push button for jump control
- Basic power supply and connections

## Controls

- **Button Press**: Make the dino jump
- **After Game Over**: Press button to restart

## Game Mechanics

- Press the button to jump over cacti
- Score increases as you survive longer
- Game speed increases with higher scores
- Collision ends the game
- Multiple obstacle types (big/small cacti)

## Project Structure

```
Inc/
  ├── function.h          # Game logic and sprite definitions
  ├── lcd.h               # LCD driver interface
  └── main.h              # Main configuration
Src/
  ├── function.c          # Game implementation
  ├── lcd.c               # LCD driver
  └── main.c              # Main game loop
```

## Build & Flash

This project is designed for STM32 development environments (STM32CubeIDE, Keil, etc.). Configure your toolchain for STM32F1xx and flash to your board.

## Customization

- Modify `BUTTON_PIN` and `BUTTON_PORT` for your button configuration
- Adjust `MAX_OBSTACLES` for difficulty
- Change `GAME_SPEED_DELAY` for game speed

---

*Classic Chrome dino game reimagined for embedded systems!* 🎮
