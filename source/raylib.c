#ifdef RAYLIB_ENGINE
#include <stdint.h>

#include "engine.h"
#include "raylib.h"
#include "util.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define TARGET_FPS 10

Vector2 CENTER_OF_SCREEN = { (float)SCREEN_WIDTH/2, (float)SCREEN_HEIGHT/2 };
uint8_t PIECE_BUFFER[VISIBLE_ROW_COUNT][VISIBLE_COLUMN_COUNT] = {};

void GameLoop()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Zetris");
        SetTargetFPS(TARGET_FPS);
        Game game = InitGame();
        while(!WindowShouldClose())
        {
            if(IsGameOver(&game)) break;
            TickGame(&game, GetFrameTime(), GetActionBitFlags());
            RenderFrame(&game);
        }
    CloseWindow();
}

uint8_t GetActionBitFlags()
{
    uint8_t inputBitFlags = 0;
    if(IsKeyDown(KEY_SPACE))                        inputBitFlags |= ACTION_HARD_DROP;
    if(IsKeyDown(KEY_H) || IsKeyDown(KEY_R))        inputBitFlags |= ACTION_HOLD_BLOCK;
    if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))     inputBitFlags |= ACTION_SOFT_DROP;
    if(IsKeyDown(KEY_E) || IsKeyDown(KEY_UP))       inputBitFlags |= ACTION_ROTATE_CLOCKWISE;
    if(IsKeyDown(KEY_Q))                            inputBitFlags |= ACTION_ROTATE_COUNTER;
    if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))    inputBitFlags |= ACTION_MOVE_RIGHT;
    if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))     inputBitFlags |= ACTION_MOVE_LEFT;
    return inputBitFlags;
}

void RenderFrame(const Game* game)
{
    const int cellSize = 15.0f;
    const int screenXStart = 200;
    const int screenYStart = 100;
    BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangle(screenXStart, screenYStart, cellSize * game->playfield.columnCount, cellSize * game->playfield.rowCount, RED);
        const uint8_t activeBlockXOffset = (uint8_t)(game->playfield.activeBlock.x);
        const uint8_t activeBlockYOffset = (uint8_t)(game->playfield.activeBlock.y);
        const uint8_t ghostBlockYOffset = (uint8_t)(game->ghostBlockY);
        for(uint8_t y = HIDDEN_ROW_COUNT; y < game->playfield.rowCount + HIDDEN_ROW_COUNT; y++)
        {
            const int8_t activeYAdjusted = (y - activeBlockYOffset);
            const int8_t ghostYAdjusted = (y - ghostBlockYOffset);
            for(uint8_t x = HIDDEN_COLUMN_COUNT; x < game->playfield.columnCount + HIDDEN_COLUMN_COUNT; x++)
            {
                const int8_t xAdjusted = (x - activeBlockXOffset);
                if(
                    (game->playfield.cells[y] & (1U << x)) ||
                    (xAdjusted >= 0 && xAdjusted < game->playfield.activeBlock.component.rotSize) &&
                    (activeYAdjusted >= 0 && activeYAdjusted < game->playfield.activeBlock.component.rotSize) &&
                    (game->playfield.activeBlock.component.cells & (1U << (BLOCK_SIZE * activeYAdjusted + xAdjusted)))
                ){
                    DrawRectangle(screenXStart + x * cellSize, screenYStart + (y - HIDDEN_ROW_COUNT) * cellSize, cellSize, cellSize, WHITE);
                }
                else if(
                    (xAdjusted >= 0 && xAdjusted < game->playfield.activeBlock.component.rotSize) &&
                    (ghostYAdjusted >= 0 && ghostYAdjusted < game->playfield.activeBlock.component.rotSize) &&
                    (game->playfield.activeBlock.component.cells & (1U << (BLOCK_SIZE * ghostYAdjusted + xAdjusted)))
                ) {
                    // printf("+ ");
                    DrawRectangle(screenXStart + x * cellSize, screenYStart + (y - HIDDEN_ROW_COUNT) * cellSize, cellSize, cellSize, GRAY);
                }
                // else {
                //     printf(". ");
                // }
            }
            printf("\n");
            // output[(y - 4)];
        }
    EndDrawing();
}
#endif // RAYLIB_ENGINE