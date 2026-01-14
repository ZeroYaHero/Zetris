#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

#include "game.h"
#include "action.h"

void GameLoop();

uint8_t GetActionBitFlags();

void RenderFrame(const Game* game);

#endif //ENGINE_H
