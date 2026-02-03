#include "piece.h"

PieceCells get_rotated_piece_cells(const PieceCells in_cells, const PieceSize size, const bool clockwise)
{
    PieceCells out_cells = 0;
	for (uint8_t y = 0; y < size; y++)
	{
		const uint8_t rotated_x = clockwise ? y : (size - y - 1);
		for (uint8_t x = 0; x < size; x++)
		{
			const uint8_t rotated_y = clockwise ? (size - 1 - x) : x;
			const uint8_t rotated_shift = PIECE_MAX_SIZE * rotated_y + rotated_x;
			const uint8_t destination_shift = PIECE_MAX_SIZE * y + x;
			out_cells |= (in_cells >> rotated_shift & 1) << destination_shift;
		}
	}
    return out_cells;
}

bool is_piece_cell(const PieceCells cells, const uint8_t pos_x, const uint8_t pos_y)
{
    return cells & (1U << PIECE_MAX_SIZE * pos_y + pos_x);
}

const PieceData* get_piece_data(const PieceType piece_type)
{
    switch (piece_type)
    {
    case I_TYPE:
        return &I_DATA;
    case O_TYPE:
        return &O_DATA;
    case T_TYPE:
        return &T_DATA;
    case S_TYPE:
        return &S_DATA;
    case Z_TYPE:
        return &Z_DATA;
    case J_TYPE:
        return &J_DATA;
    case L_TYPE:
        return &L_DATA;
    }
}

// PieceData related functions.
void copy_data_into_piece(Piece* piece, PieceData* const piece_data)
{
    piece->rotation_wall_kicks = piece_data->rotation_wall_kicks;
    piece->cells = piece_data->cells;
    piece->type = piece_data->type;
    piece->size = piece_data->size;
}

// PieceTransform related functions.
void set_piece_position(Piece* piece, const uint8_t x, const uint8_t y)
{
    piece->pos_x = x;
    piece->pos_y = y;
}

void reset_piece_velocity(Piece* piece)
{
    piece->velo_x = 0;
    piece->velo_y = 0;
}

void reset_piece_transform(Piece* piece)
{
    reset_piece_velocity(piece);
    set_piece_position(piece, 0, 0);
    piece->rotation = 0;
}

// PieceLock related functions.
void reset_piece_lock(Piece* piece)
{
    piece->on_ground = false;
    piece->timer = 0.0f;
    piece->moves = 0;
}

const PieceData I_DATA = {
    &WALL_KICKS_I,
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
    ROT_4X4
};

const PieceData O_DATA = {
    0,
    0b0000000000110011,
    /**
     * _ _ _ _
     * _ _ _ _
     * _ _ # #
     * _ _ # #
     */
    O_TYPE,
    NONE_2X2
};

const PieceData T_DATA = {
    &WALL_KICKS_TSZJL,
    0b0000000001110010,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # #
     * _ _ # _
     */
    T_TYPE,
    ROT_3X3,
};

const PieceData S_DATA = {
    &WALL_KICKS_TSZJL,
    0b0000000000110110,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ _ # #
     * _ # # _
     */
    S_TYPE,
    ROT_3X3
};

const PieceData Z_DATA = {
    &WALL_KICKS_TSZJL,
    0b0000000001100011,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # _
     * _ _ # #
     */
    Z_TYPE,
    ROT_3X3
};

const PieceData J_DATA = {
    &WALL_KICKS_TSZJL,
    0b0000000001110001,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # #
     * _ _ _ #
     */
    J_TYPE,
    ROT_3X3
};

const PieceData L_DATA = {
    &WALL_KICKS_TSZJL,
    0b0000000001110100,
    /**
     * _ _ _ _
     * _ _ _ _ 
     * _ # # #
     * _ # _ _
     */
    L_TYPE,
    ROT_3X3
};

const PieceData* ALL_PIECE_DATA[PIECE_COUNT] = {
    &I_DATA,
    &O_DATA,
    &T_DATA,
    &S_DATA,
    &Z_DATA,
    &J_DATA,
    &L_DATA
};

const PieceRotationWallKicks WALL_KICKS_TSZJL = {
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

const PieceRotationWallKicks WALL_KICKS_I = {
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

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
const char* STATES = "0R2L";

void print_cells(const PieceCells in_cells)
{
    for(uint8_t y = 0; y < PIECE_MAX_SIZE; y++)
    {
        for(uint8_t x = 0; x < PIECE_MAX_SIZE; x++)
        {
            printf("%c ", is_piece_cell(in_cells, x, y) ? '#' : '.');
        }
        printf("\n");
    }
    printf("\n");
}

void print_all_piece_rotations()
{
    for(uint8_t i = 0; i < PIECE_COUNT; i++)
    {
        const PieceData piece_data = *ALL_PIECE_DATA[i];
        PieceCells cells = piece_data.cells;
        printf("0\n");
        print_cells(cells);
        if(piece_data.size > 2)
        {
            //TODO: need to fix lol.
            printf("0->R\n");
            cells = get_rotated_piece_cells(cells, piece_data.size, true);
            print_cells(cells);

            printf("R->2\n");
            cells = get_rotated_piece_cells(cells, piece_data.size, true);
            print_cells(cells);

            printf("2->L\n");
            cells = get_rotated_piece_cells(cells, piece_data.size, true);
            print_cells(cells);

            printf("0->L\n");
            cells = get_rotated_piece_cells(cells, piece_data.size, true);
            print_cells(cells);
        }
    }
}

void print_LR_wall_kicks(const PieceRotationWallKicks* rotation_wall_kicks, const uint8_t state, const bool right)
{
    printf("\t%s Rotation Wall Kicks\n", (right) ? "Right" : "Left");
    for(int test = 0; test < PIECE_ROTATION_TESTS - 1; test++)
    {
        WallKick wall_kick = (*rotation_wall_kicks)[state * 2 + ((right) ? 1 : 0)][test];
        printf("\t\tX: %d, Y: %d\n", UNPACK_X(wall_kick), UNPACK_Y(wall_kick));
    }
}

void print_piece_wall_kicks(const PieceRotationWallKicks* rotation_wall_kicks)
{
    for(int state = 0; state < PIECE_ROTATION_STATES; state++)
    {
        printf("Current State %c\n", STATES[state]);
        print_LR_wall_kicks(rotation_wall_kicks, state, false);
        print_LR_wall_kicks(rotation_wall_kicks, state, true);
    }
}

void print_all_wall_kicks()
{
    printf("T, S, Z, J, L Wall Kicks:\n");
    print_piece_wall_kicks(&WALL_KICKS_TSZJL);
    printf("I Wall Kicks:\n");
    print_piece_wall_kicks(&WALL_KICKS_I);
}
#endif // DEBUG