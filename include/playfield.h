#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#define HIDDEN_ROW_COUNT            4
#define VISIBLE_ROW_COUNT           16
#define HIDDEN_COLUMN_COUNT         1
#define VISIBLE_COLUMN_COUNT        10
#define MAX_ROW_COLUMN_COUNT        32

#include <stdint.h>
#include <stdbool.h>

#include "blocks.h"

typedef uint32_t PlayfieldCells[MAX_ROW_COLUMN_COUNT];

typedef struct {
    PlayfieldCells cells;               // 32 * 32 = 1024 bits (4 * 32 = 128 bytes)
    BlockEntity activeBlock;           // 104 bit (13 bytes)
    uint8_t rowCount;                   // 8 bit (1 byte)
    uint8_t columnCount;                // 8 bit (1 byte)
} Playfield;                            // Total: 1152 bits (144 bytes)

void SetActiveBlockComponent(
    Playfield* playfield,
    BlockComponent* inComponent
);

bool AttemptWriteBlockRotation(
    Playfield* playfield,
    BlockEntity* inBlock,
    bool clockwise
);

bool AttemptWriteBlockMoveX(
    Playfield* playfield,
    BlockEntity* inBlock,
    bool positive
);

bool AttemptWriteBlockMoveY(
    Playfield* playfield,
    BlockEntity* inBlock,
    bool positive
);

// This really should not be used. It is more just a way to do X and Y without rewriting the same code.
bool AttemptWriteBlockMove(
    Playfield* playfield,
    BlockEntity* inBlock,
    int8_t inX,
    int8_t inY
);

bool AttemptWriteBlockHardDrop(
    Playfield* playfield,
    BlockEntity* inBlock
);

bool IsOutsideBounds(
    Playfield* playfield,
    int inX,
    int inY
);

bool IsOnGround(
    Playfield* playfield,
    BlockCells inCells,
    uint8_t inSize,
    uint8_t inX,
    uint8_t inY
);

bool AreCellsColliding(
    Playfield* playfield,
    BlockCells inCells,
    uint8_t inSize,
    uint8_t inX,
    uint8_t inY
);

// Will write if cells collide or are outside playfield bounds (hence the "force),
// but you don't have to worry about integer underflow, overflow, or segfault.
void ForceWriteBlockInPlayfield(
    Playfield* playfield,
    BlockEntity* inBlock
);

uint8_t ClearRows(
    Playfield* playfield,
    uint8_t inYOffset
);

bool AreCellsAboveLine(
    Playfield* playfield
);

#endif // PLAYFIELD_H