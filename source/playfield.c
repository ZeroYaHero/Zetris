#include <stdlib.h>

#include "playfield.h"

void SetActiveBlockComponent(Playfield* playfield, BlockComponent* inComponent)
{
    playfield->activeBlock.component = *inComponent;
    playfield->activeBlock.y = 0.0f;
    playfield->activeBlock.x = (playfield->columnCount - inComponent->rotSize) * 0.5f + HIDDEN_COLUMN_COUNT;
    playfield->activeBlock.rotState = 0;
}

bool AttemptWriteBlockRotation(Playfield* playfield, BlockEntity* inBlock, const bool clockwise)
{
    if(playfield->activeBlock.component.rotSize == NONE_2X2) return true;

    const uint8_t rotStateIndex = inBlock->rotState + ((clockwise) ? 1 : 0);
    const BlockCells rotatedCells = RotateBlockCells(inBlock->component.cells, inBlock->component.rotSize, clockwise);

    int8_t yWallKick = 0;
    int8_t xWallKick = 0;
    for(uint8_t testIndex = 0; testIndex < BLOCK_ROTATION_TESTS; testIndex++)
    {
        if(testIndex > 0)
        {
            UnpackedWallKickXY unpackedWallKickXY = UnpackWallKickXY((*inBlock->component.wallKicksData)[rotStateIndex][testIndex - 1]);
            yWallKick = unpackedWallKickXY.y;
            xWallKick = unpackedWallKickXY.x;
        }
        // if(!IsOutsideBounds(playfield, inBlock->x + xWallKick, inBlock->y - yWallKick)) {
        const bool areRotatedCellsColliding = AreCellsColliding(
            playfield,
            rotatedCells,
            inBlock->component.rotSize,
            inBlock->x + xWallKick,
            inBlock->y - yWallKick // Have to flip the sign here unfortunately because board is upside down.
        );
        if(!areRotatedCellsColliding)
        {
            inBlock->component.cells = rotatedCells;
            inBlock->x += xWallKick;
            inBlock->y -= yWallKick;
            inBlock->rotState = (inBlock->rotState + BLOCK_ROTATION_STATES + ((clockwise) ? 1 : -1)) & 3; // TODO: Make preprocessor maybe.
            return true;
        }
        // }
    }
    return false;
}

bool AttemptWriteBlockMoveX(Playfield* playfield, BlockEntity* inBlock, const bool positive)
{
    return AttemptWriteBlockMove(playfield, inBlock, (positive) ? 1 : -1, 0);
}

bool AttemptWriteBlockMoveY(Playfield* playfield, BlockEntity* inBlock, const bool positive)
{
    return AttemptWriteBlockMove(playfield, inBlock, 0, (positive) ? 1 : -1);
}

bool AttemptWriteBlockMove(Playfield* playfield, BlockEntity* inBlock, int8_t inX, int8_t inY)
{
    // We have to check if the corner (smallest index value) is not negative.
    // if(IsOutsideBounds(playfield, inBlock->x + inX, inBlock->y + inY)) return false;
    if(inBlock->x + inX < 0) {
        printf("break");
    }
    const bool cellCollisionAfterMove = AreCellsColliding(
        playfield,
        inBlock->component.cells,
        inBlock->component.rotSize,
        inBlock->x + inX,
        inBlock->y + inY
    );
    if(!cellCollisionAfterMove) {
        inBlock->x += inX;
        inBlock->y += inY;
        return true;
    }
    return false;
}

bool AttemptWriteBlockHardDrop(Playfield* playfield, BlockEntity* inBlock)
{
    uint8_t y = inBlock->y;
    for(;y < MAX_ROW_COLUMN_COUNT;y++) // Safeguard against for loop and segfault or UB
    {
        if(IsOnGround(playfield, inBlock->component.cells, inBlock->component.rotSize, inBlock->x, y))
        {
            inBlock->y = y;
            return true;
        }
    }
    return false;
}

bool IsOutsideBounds(Playfield* playfield, int inX, int inY)
{
    return inY < 0 || inY > (playfield->rowCount + HIDDEN_ROW_COUNT - 1) || inX < HIDDEN_COLUMN_COUNT || inX > (playfield->columnCount - 1);
}

bool IsActiveBlockOnGround(Playfield* playfield)
{
    return IsOnGround(
        playfield,
        playfield->activeBlock.component.cells,
        playfield->activeBlock.component.rotSize,
        playfield->activeBlock.x,
        playfield->activeBlock.y
    );
}

bool IsOnGround(Playfield* playfield, const BlockCells inCells, const uint8_t inSize, uint8_t inX, uint8_t inY)
{
    const bool areCellsColliding =        AreCellsColliding(playfield, inCells, inSize, inX, inY);
    const bool areLoweredCellsColliding = AreCellsColliding(playfield, inCells, inSize, inX, inY + 1);
    return !areCellsColliding && areLoweredCellsColliding;
}

bool AreCellsColliding(
    Playfield* playfield,
    BlockCells inCells,
    uint8_t inSize,
    uint8_t inX,
    uint8_t inY
)
{
    uint8_t blockCellCount = 0;
    for(uint8_t y = 0; y < inSize; y++)
    {
        for(uint8_t x = 0; x < inSize; x++)
        {
            // If there is a cell in this block position
            if(inCells & (1U << (BLOCK_SIZE * y + x)))
            {
                blockCellCount++;
                if(IsOutsideBounds(playfield, inX + x, inY + y))
                {
                    return true;
                }
                // If there is a cell in this playfield position
                if(playfield->cells[inY + y] & (1U << (inX + x)))
                {
                    return true;
                }
                if(blockCellCount == BLOCK_CELL_COUNT)
                {
                    return false;
                }
            }
        }
    }
    return false;
}

void ForceWriteBlockInPlayfield(Playfield* playfield, BlockEntity* inBlock)
{
    uint8_t blockCellCount = 0;
    for(uint8_t y = 0; y < inBlock->component.rotSize; y++)
    {
        for(uint8_t x = 0; x < inBlock->component.rotSize; x++)
        {
            if(inBlock->component.cells & (1U << (BLOCK_SIZE * y + x)))
            {
                blockCellCount++;
                if(IsOutsideBounds(playfield, inBlock->x + x, inBlock->y + y))
                {
                    continue;
                }
                playfield->cells[(uint8_t)(inBlock->y + y)] |= (1U << ((uint8_t)(inBlock->x + x)));
                if(blockCellCount == 4) return;
            }
        }
    }
}

uint8_t ClearRows(Playfield* playfield, uint8_t inYOffset)
{
    inYOffset = (inYOffset >= playfield->rowCount + HIDDEN_ROW_COUNT) ? playfield->rowCount + HIDDEN_ROW_COUNT - 1 : inYOffset;
    fprintf(stderr, "Checking for rows to clear starting from at %d", inYOffset);
    const uint32_t mask = (1U << playfield->columnCount) - 1;
    uint8_t rowsCleared = 0;
    while(true)
    {
        if(playfield->cells[inYOffset] == mask)
        {
            rowsCleared++;
        }
        else if(rowsCleared)
        {
            playfield->cells[inYOffset + rowsCleared] = playfield->cells[inYOffset];
            playfield->cells[inYOffset] = 0;
        }
        if(inYOffset == 0) break;
        inYOffset--;
    }
    return rowsCleared;
}

bool AreCellsAboveLine(Playfield* playfield)
{
    for(int8_t y = HIDDEN_ROW_COUNT - 1; y >= 0; y--)
    {
        if(playfield->cells[y]) return true;
    }
    return false;
}