#include "blocks.h"
#include "util.h"

BlockCells RotateBlockCells(const BlockCells inCells, const RotationSize rotSize, const bool clockwise)
{    
    BlockCells rotCells = 0;
    for(uint8_t y = 0; y < rotSize; y++)
    {
        const uint8_t rotX = clockwise ? y : (rotSize - 1 - y);
        for(uint8_t x = 0; x < rotSize; x++)
        {
            const uint8_t rotY = clockwise ? (rotSize - 1 - x): x;
            const uint8_t rotShift = BLOCK_SIZE * rotY + rotX;
            const uint8_t destShift = BLOCK_SIZE * y + x;
            rotCells |= (inCells >> rotShift & 1) << destShift;
        }
    }
    return rotCells;
}

// Found this doing some searching to extend the sign: https://stackoverflow.com/questions/5814072/sign-extend-a-nine-bit-number-in-c
// This lead me to this: https://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
// Super neat. Essentially what it does is get the MSB (sign), XOR with the value to remove the sign, and then subtract it to get it to become negative.
// This is known as Sign Extension (or sext, LOL).
// Sign extension is normally done automatically for 8 bit to 16 bit, 16 to 32, etc.
// But because there is no such thing as a 4 bit (nibble, non addressable in byte addressable systems) have to do it manually!
UnpackedWallKickXY UnpackWallKickXY(const uint8_t packedWallKickXY)
{
    int8_t x = (((packedWallKickXY >> 4) & 0x0F) ^ 0b1000) - 0b1000;
    int8_t y = ((packedWallKickXY & 0x0F) ^ 0b1000) - 0b1000;
    return (UnpackedWallKickXY){x, y};
}

const BlockComponent I_COMPONENT = {
    0b0000000011110000,
    /**
     * v MSB
     * _ _ _ _
     * _ _ _ _ 
     * # # # #
     * _ _ _ _
     *       ^ LSB
     */
    I_TYPE,
    ROT_4X4,
    &WALL_KICKS_I
};

const BlockComponent O_COMPONENT = {
    0b0000000000110011,
    /**
     * _ _ _ _
     * _ _ _ _
     * _ _ # #
     * _ _ # #
     */
    O_TYPE,
    NONE_2X2,
    0
};

const BlockComponent T_COMPONENT = {
    0b0000000001110010,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # #
     * _ _ # _
     */
    T_TYPE,
    ROT_3X3,
    &WALL_KICKS_TSZJL
};

const BlockComponent S_COMPONENT = {
    0b0000000000110110,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ _ # #
     * _ # # _
     */
    S_TYPE,
    ROT_3X3,
    &WALL_KICKS_TSZJL
};

const BlockComponent Z_COMPONENT = {
    0b0000000001100011,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # _
     * _ _ # #
     */
    Z_TYPE,
    ROT_3X3,
    &WALL_KICKS_TSZJL
};

const BlockComponent J_COMPONENT = {
    0b0000000001110001,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # #
     * _ _ _ #
     */
    J_TYPE,
    ROT_3X3,
    &WALL_KICKS_TSZJL
};

const BlockComponent L_COMPONENT = {
    0b0000000001110100,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # #
     * _ # _ _
     */
    L_TYPE,
    ROT_3X3,
    &WALL_KICKS_TSZJL
};

const BlockComponent* ALL_BLOCK_COMPONENTS[BLOCK_COUNT] = {
    &I_COMPONENT,
    &O_COMPONENT,
    &T_COMPONENT,
    &S_COMPONENT,
    &Z_COMPONENT,
    &J_COMPONENT,
    &L_COMPONENT
};

#define PACK_XY(left, right) \
    ( (uint8_t) ( ( ( (left) & 0x0F ) << 4 ) | ( (right) & 0x0F ) ) )

const PackedWallKicksData WALL_KICKS_TSZJL = {
    // State: 0
    {   // Left rotation: 0->L
        PACK_XY(1, 0),
        PACK_XY(1, 1),
        PACK_XY(0, -2),
        PACK_XY(1, -2)
    }, 
    {   // Right rotation: 0->R
        PACK_XY(-1, 0),
        PACK_XY(-1, 1),
        PACK_XY(0, -2),
        PACK_XY(-1, -2)
    },

    // State: R
    {   // Left rotation: R->0
        PACK_XY(1, 0),
        PACK_XY(1, -1),
        PACK_XY(0, 2),
        PACK_XY(1, 2)
    },
    {   // Right rotation: R->2
        PACK_XY(1, 0),
        PACK_XY(1, -1),
        PACK_XY(0, 2),
        PACK_XY(1, 2)
    },

    // State: 2
    {   // Left rotation: 2->R
        PACK_XY(-1, 0),
        PACK_XY(-1, 1),
        PACK_XY(0, -2),
        PACK_XY(-1, -2)
    },
    {   // Right rotation: 2->L
        PACK_XY(1, 0),
        PACK_XY(1, 1),
        PACK_XY(0, -2),
        PACK_XY(1, -2)
    },

    // State: L
    {   // Left rotation: L->2
        PACK_XY(-1, 0),
        PACK_XY(-1, -1),
        PACK_XY(0, 2),
        PACK_XY(-1, 2)
    },
    {   // Right rotation: L->0
        PACK_XY(-1, 0),
        PACK_XY(-1, -1),
        PACK_XY(0, 2),
        PACK_XY(-1, 2)
    },
};

const PackedWallKicksData WALL_KICKS_I = {
    // State: 0
    {   // Left rotation: 0->L
        PACK_XY(-1, 0),
        PACK_XY(2, 0),
        PACK_XY(-1, 2),
        PACK_XY(2, -1)
    }, 
    {   // Right rotation: 0->R
        PACK_XY(-2, 0),
        PACK_XY(1, 0),
        PACK_XY(-2, -1),
        PACK_XY(1, 2)
    },

    // State: R
    {   // Left rotation: R->0
        PACK_XY(2, 0),
        PACK_XY(-1, 0),
        PACK_XY(2, 1),
        PACK_XY(-1, -2)
    },
    {   // Right rotation: R->2
        PACK_XY(-1, 0),
        PACK_XY(2, 0),
        PACK_XY(-1, 2),
        PACK_XY(2, -1)
    },

    // State: 2
    {   // Left rotation: 2->R
        PACK_XY(1, 0),
        PACK_XY(-2, 0),
        PACK_XY(1, -2),
        PACK_XY(-2, 1)
    },
    {   // Right rotation: 2->L
        PACK_XY(2, 0),
        PACK_XY(-1, 0),
        PACK_XY(2, 1),
        PACK_XY(-1, -2)
    },

    // State: L
    {   // Left rotation: L->2
        PACK_XY(-2, 0),
        PACK_XY(1, 0),
        PACK_XY(-2, -1),
        PACK_XY(1, 2)
    },
    {   // Right rotation: L->0
        PACK_XY(1, 0),
        PACK_XY(-2, 0),
        PACK_XY(1, -2),
        PACK_XY(-2, 1)
    },
};

#ifdef DEBUG
const char* STATES = "0R2L";
// ~~Use puts because the log level (I forget what it's called) is occupied by Raylib.~~
// ^ No.
void PrintCells(const BlockCells cells)
{
    for(uint8_t y = 0; y < BLOCK_SIZE; y++)
    {
        for(uint8_t x = 0; x < BLOCK_SIZE; x++)
        {
            uint8_t bit = (cells >> (BLOCK_SIZE * y + x) & 1);
            printf("%c ", (bit) ? '#' : '.');
        }
        printf("\n");
    }
    printf("\n");
}

void PrintAllBlockRotations()
{
    for(uint8_t i = 0; i < BLOCK_COUNT; i++)
    {
        const BlockComponent block = *ALL_BLOCK_COMPONENTS[i];
        BlockCells cells = block.cells;
        printf("0\n");
        PrintCells(cells);
        if(block.rotSize > 2)
        {
            printf("0->R\n");
            cells = RotateBlockCells(cells, block.rotSize, true);
            PrintCells(cells);

            printf("R->2\n");
            cells = RotateBlockCells(cells, block.rotSize, true);
            PrintCells(cells);

            printf("2->L\n");
            cells = RotateBlockCells(cells, block.rotSize, true);
            PrintCells(cells);

            printf("0->L\n");
            cells = RotateBlockCells(block.cells, block.rotSize, false);
            PrintCells(cells);
        }
    }
}

void PrintLRWallKicks(const PackedWallKicksData* wallKickData, const uint8_t state, const bool right)
{
    printf("\t%s Rotation Wall Kicks\n", (right) ? "Right" : "Left");
    for(int test = 0; test < BLOCK_ROTATION_TESTS - 1; test++)
    {
        UnpackedWallKickXY currentWallKickXY = UnpackWallKickXY((*wallKickData)[state * 2 + ((right) ? 1 : 0)][test]);
        printf("\t\tX: %d, Y: %d\n", currentWallKickXY.x, currentWallKickXY.y);
    }
}

void PrintBlockWallKicks(const PackedWallKicksData* wallKickData)
{
    for(int state = 0; state < BLOCK_ROTATION_STATES; state++)
    {
        printf("Current State %c\n", STATES[state]);
        PrintLRWallKicks(wallKickData, state, false);
        PrintLRWallKicks(wallKickData, state, true);
    }
}

void PrintAllWallKicks()
{
    printf("T, S, Z, J, L Wall Kicks:\n");
    PrintBlockWallKicks(&WALL_KICKS_TSZJL);
    printf("I Wall Kicks:\n");
    PrintBlockWallKicks(&WALL_KICKS_I);
}
#endif // DEBUG