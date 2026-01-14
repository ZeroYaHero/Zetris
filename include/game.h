#ifndef GAME_H
#define GAME_H

#define SETTING_CAN_HOLD         0b00000001
#define SETTING_INFINITE_DELAY   0b00000010

#define LOCK_DELAY               0.5f
#define MAX_MOVES_BEFORE_LOCK    10

#define HORIZONTAL_VELOCITY      3.0f
#define VERTICAL_VELOCITY        3.0f

#include <stdbool.h>
#include <stdint.h>

#include "playfield.h"

typedef struct {
    uint32_t linesCleared;
    float gravity;
} Level;

typedef struct {
    Playfield playfield;
    uint64_t score;
    double lockTimer;
    BlockComponent* blockBag[BLOCK_COUNT];
    uint32_t linesCleared;
    float gravity;
    float xAccumulation;
    float yAccumulation;
    BlockComponent* heldBlock;
    uint8_t _settingBitFlags;
    uint8_t ghostBlockY;
    uint8_t lockDelayMoves;
    uint8_t blockBagIndex;
    bool hitGround;
    bool canHoldNext;
} Game;

Game                InitGame();
void                TickGame(Game* game, double deltaTime, uint8_t actionBitFlags);
bool                IsGameOver(Game* game);

#endif // GAME_H