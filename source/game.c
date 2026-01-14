#include <math.h>

#include "game.h"
#include "action.h"
#include "util.h"

void ResetLockDelay(Game* game)
{
    game->hitGround = false;
    game->lockTimer = 0.0f;
    game->lockDelayMoves = 0;
}

BlockComponent* GetNextInBlockBag(Game* game)
{
    if(game->blockBagIndex >= BLOCK_COUNT)
    {
        game->blockBagIndex = 0;
        shuffle(game->blockBag, BLOCK_COUNT, sizeof(BlockComponent*));
    }
    return game->blockBag[game->blockBagIndex++];
}

void PlaceBlock(Game* game, BlockEntity* inBlock)
{
    ForceWriteBlockInPlayfield(&game->playfield, inBlock);
    ClearRows(&game->playfield, inBlock->y + inBlock->component.rotSize);
    SetActiveBlockComponent(&game->playfield, GetNextInBlockBag(game));
    ResetLockDelay(game);
    game->canHoldNext = true;
    game->xAccumulation = 0.0f;
    game->yAccumulation = 0.0f;
}

Game InitGame()
{
    Game game = {
        .playfield = {
            .cells = {0},
            .activeBlock = {0},
            .rowCount = VISIBLE_ROW_COUNT,
            .columnCount = VISIBLE_COLUMN_COUNT
        },
        .score = 0,
        .linesCleared = 0,
        .gravity = 1.0f,
        .yAccumulation = 0.0f,
        .xAccumulation = 0.0f,
        .heldBlock = 0,
        .ghostBlockY = 0,
        .blockBagIndex = 0,
        .canHoldNext = true,
        ._settingBitFlags = SETTING_CAN_HOLD,
    };
    memcpy(game.blockBag, ALL_BLOCK_COMPONENTS, BLOCK_COUNT * sizeof(BlockComponent*));
    shuffle(game.blockBag, BLOCK_COUNT, sizeof(BlockComponent*));
    SetActiveBlockComponent(&game.playfield, GetNextInBlockBag(&game));
    return game;

}

void TickGame(Game* game, const double deltaTime, const uint8_t actionBitFlags)
{
    // If this tick the player attempted to HOLD_BLOCK
    if(actionBitFlags & ACTION_HOLD_BLOCK && game->canHoldNext && (game->_settingBitFlags & SETTING_CAN_HOLD))
    {
        BlockComponent* newHeldBlock = &game->playfield.activeBlock.component;
        SetActiveBlockComponent(&game->playfield, (game->heldBlock) ? game->heldBlock : GetNextInBlockBag(game));
        game->heldBlock = newHeldBlock;
        game->canHoldNext = false;

        ResetLockDelay(game);
        game->xAccumulation = 0.0f;
        game->yAccumulation = 0.0f;
    }
    // If this tick the player attempted to HARD_DROP
    else if(actionBitFlags & ACTION_HARD_DROP)
    {
        AttemptWriteBlockHardDrop(&game->playfield, &game->playfield.activeBlock); // Can't use ghostBlockY because what if first simulation?
        PlaceBlock(game, &game->playfield.activeBlock);
    }
    // If this tick the player MOVE_x, SOFT_DROP, or ROTATE_x
    else {
        bool xChanged = false;
        bool yChanged = false;
        bool rotChanged = false;

        // Rotation
        if((actionBitFlags & ACTION_ROTATE_CLOCKWISE) || (actionBitFlags & ACTION_ROTATE_COUNTER))
        {
            rotChanged = AttemptWriteBlockRotation(&game->playfield, &game->playfield.activeBlock, actionBitFlags & ACTION_ROTATE_CLOCKWISE);
        }

        // Horizontal movement
        game->xAccumulation += (((actionBitFlags & ACTION_MOVE_RIGHT) ? HORIZONTAL_VELOCITY : 0.0f) + ((actionBitFlags & ACTION_MOVE_LEFT) ? -HORIZONTAL_VELOCITY : 0.0f)) * deltaTime;
        while(fabsf(game->xAccumulation) >= 1)
        {
            if(AttemptWriteBlockMoveX(&game->playfield, &game->playfield.activeBlock, sgn(game->xAccumulation) > 0))
            {
                game->xAccumulation -= sgn(game->xAccumulation);
                xChanged = true;
            }
            else
            {
                game->xAccumulation = 0.0f;
                break;
            }
        }
        // Vertical movement
        game->yAccumulation += (game->gravity + ((actionBitFlags & ACTION_SOFT_DROP) ? VERTICAL_VELOCITY : 0.0f)) * deltaTime;
        while(fabsf(game->yAccumulation) >= 1)
        {
            if(AttemptWriteBlockMoveY(&game->playfield, &game->playfield.activeBlock, sgn(game->yAccumulation) > 0))
            {
                game->yAccumulation -= sgn(game->yAccumulation);
                yChanged = true;
            }
            else
            {
                game->yAccumulation = 0.0f;
                break;
            }
        }
        // Ghost block
        BlockEntity ghostBlock = game->playfield.activeBlock; // Shallow copy is okay considering no pointer data members...still icky.
        AttemptWriteBlockHardDrop(&game->playfield, &ghostBlock);
        game->ghostBlockY = ghostBlock.y;
        // Lock delay
        if(yChanged)
        {
            ResetLockDelay(game);
        }
        else if(game->hitGround)
        {
            if(xChanged || rotChanged)
            {
                game->lockTimer = 0.0f;
                if(!(game->_settingBitFlags & SETTING_INFINITE_DELAY))
                {
                    game->lockDelayMoves++;
                    if(game->lockDelayMoves >= MAX_MOVES_BEFORE_LOCK)
                    {
                        PlaceBlock(game, &ghostBlock);
                    }
                } else {
                    ResetLockDelay(game);
                }
            }
            else
            {
                game->lockTimer += deltaTime;
                if(game->lockTimer >= LOCK_DELAY)
                {
                    PlaceBlock(game, &ghostBlock);
                }
            }
        }
        else if(game->playfield.activeBlock.y == game->ghostBlockY)
        {
            game->hitGround = true;
        }

    }

}

bool IsGameOver(Game* game)
{
    return AreCellsAboveLine(&game->playfield);
}