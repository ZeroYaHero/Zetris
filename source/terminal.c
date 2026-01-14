#ifdef TERMINAL_ENGINE
#include <time.h>

#include "engine.h"
#include "util.h"

void GameLoop()
{
    Game game = InitGame();
    clock_t lastTime = clock();
    while(!IsGameOver(&game))
    {
        clock_t currentTime = clock();
        double deltaTime = (double)(currentTime - lastTime) / CLOCKS_PER_SEC;
        TickGame(&game, deltaTime, GetActionBitFlags());
        RenderFrame(&game);
        lastTime = currentTime;
        clock_t awaitStart = clock();
        while(1)
        {
            if(((double)(clock() - awaitStart) / CLOCKS_PER_SEC) > 0.5) break;
        }
    }
    printf("Game Over\n");
}

uint8_t GetActionBitFlags()
{
    return 0;
}

void RenderFrame(const Game* game)
{
    printf("\033[2J\033[H");
    fflush(stdout);
    printf("\033[?25l");
    const uint8_t activeBlockXOffset = (uint8_t)(game->playfield.activeBlock.x);
    const uint8_t activeBlockYOffset = (uint8_t)(game->playfield.activeBlock.y);
    const uint8_t ghostBlockYOffset = (uint8_t)(game->ghostBlockY);
    for(uint8_t y = HIDDEN_ROW_COUNT; y < game->playfield.rowCount + HIDDEN_ROW_COUNT; y++)
    {
        const int8_t activeYAdjusted = (y - activeBlockYOffset);
        const int8_t ghostYAdjusted = (y - ghostBlockYOffset);
        for(uint8_t x = 0; x < game->playfield.columnCount; x++)
        {
            const int8_t xAdjusted = (x - activeBlockXOffset);
            if(
                (game->playfield.cells[y] & (1U << x)) ||
                (xAdjusted >= 0 && xAdjusted < game->playfield.activeBlock.component.rotSize) &&
                (activeYAdjusted >= 0 && activeYAdjusted < game->playfield.activeBlock.component.rotSize) &&
                (game->playfield.activeBlock.component.cells & (1U << (BLOCK_SIZE * activeYAdjusted + xAdjusted)))
            ){
                printf("# ");
            }
            else if(
                (xAdjusted >= 0 && xAdjusted < game->playfield.activeBlock.component.rotSize) &&
                (ghostYAdjusted >= 0 && ghostYAdjusted < game->playfield.activeBlock.component.rotSize) &&
                (game->playfield.activeBlock.component.cells & (1U << (BLOCK_SIZE * ghostYAdjusted + xAdjusted)))
            ) {
                printf("+ ");
            }
            else {
                printf(". ");
            }
        }
        printf("\n");
        // output[(y - 4)];
    }
    printf("\033[?25h");
    printf("------------------\n");
    // printf(output);
    // free(output);
}
#endif // TERMINAL_ENGINE