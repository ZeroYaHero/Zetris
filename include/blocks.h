#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCK_COUNT             7
#define BLOCK_ROTATION_STATES   4
#define BLOCK_ROTATION_TESTS    5
#define BLOCK_SIZE              4
#define BLOCK_CELL_COUNT        4

#include <stdint.h>
#include <stdbool.h>

typedef enum : uint8_t {
    I_TYPE = 1,
    O_TYPE,
    T_TYPE,
    S_TYPE,
    Z_TYPE,
    J_TYPE,
    L_TYPE
} BlockType;

typedef enum : uint8_t {
    NONE_2X2 = 2,
    ROT_3X3,
    ROT_4X4
} RotationSize;

// https://stackoverflow.com/questions/14808908/pointer-to-2d-arrays-in-c
// Following this post, I like the strategy of using a typedef for easier pointers to 2D arrays. 
typedef uint8_t     PackedWallKicksData[8][4];
typedef uint16_t    BlockCells;

typedef struct {                          // Block struct for pieces. Cells are LSB -> MSB.
    BlockCells cells;                     // 16 bit (2 byte)
    BlockType blockType;                  // 8 bit
    RotationSize rotSize;                 // 8 bit  (1 byte)
    const PackedWallKicksData* wallKicksData;   // 8 bit  (1 byte)
} BlockComponent;                         // Total: 40 bits (1 byte)

typedef struct {                    // Block that exists only within playfield.
    BlockComponent component;       // 48 bit
    uint8_t x;                      // 8 bit
    uint8_t y;                      // 8 bit
    uint8_t rotState;               // 8 bit
} BlockEntity;                      // Total: 104 bits (13 bytes)

typedef struct {
    int8_t x;           // 8 bit
    int8_t y;           // 8 bit
} UnpackedWallKickXY;     // Total: 16 bit (1 byte)

extern const BlockComponent I_COMPONENT;
extern const BlockComponent O_COMPONENT;
extern const BlockComponent T_COMPONENT;
extern const BlockComponent S_COMPONENT;
extern const BlockComponent Z_COMPONENT;
extern const BlockComponent J_COMPONENT;
extern const BlockComponent L_COMPONENT;

extern const BlockComponent* ALL_BLOCK_COMPONENTS[BLOCK_COUNT];

/**  How Wall Kicks Work:
    1. The first rotation is a 0,0 offset, so each block tests 4 wall kicks after basic rotation failure.
    2. A rotation to the left is a decrease in current rotation index.
    3. A rotation to the right is a increase in the current rotation index.
    4. Rotation index is calculated using bitwise and to get remainder of index and the direction: (index + 4 +- direction) & 3
    5. If the direction is negative (counter clockwise, left), we use the index * 2.
    6. If it is positive (clockwise, right), we use index * 2 + 1.
    7. A wall kick has and X and Y offsets. This is encoded into a single byte,
    with the X component MSB aligned with the bytes MSB, and the Y component LSB aligned with the bytes LSB.
    0bXXXXYYYY
    8. Wall kicks at least are -2 and at most are +2. Negatives are accounted for using twos complement.
    9. Tests are traversed via 2nd array index.
*/

extern const PackedWallKicksData WALL_KICKS_TSZJL;
extern const PackedWallKicksData WALL_KICKS_I;

BlockCells          RotateBlockCells(BlockCells inCells, RotationSize rotSize, bool clockwise);   // Rotate given cells of a block clockwise or counter clockwise.
UnpackedWallKickXY  UnpackWallKickXY(uint8_t packedWallKickXY);

#ifdef DEBUG
#include <stdio.h>
extern const char* STATES;
void        PrintCells(uint16_t cells);                                                 // Print a block using puts(). Used for debugging purposes.
void        PrintAllBlockRotations();                                                   // Print all blocks and their rotations.
void        PrintAllWallKicks();
#endif // DEBUG

#endif // BLOCKS_H