#ifndef PIECE_H
#define PIECE_H

#include <stdint.h>
#include <stdbool.h>

#define PIECE_COUNT                 7
#define PIECE_ROTATION_DIRECTIONS   2
#define PIECE_ROTATION_STATES       4
#define PIECE_ROTATION_TESTS        5
#define PIECE_MAX_SIZE              4
#define PIECE_CELL_COUNT            4

// Pack a Vector2 (of minimum -8 and maximum +7) into a byte. Useful for wall-kicks as they never surpass -2 or +2.
#define PACK_XY(x, y) \
    ( (uint8_t) ( ( ( (x) & 0x0F ) << 4 ) | ( (y) & 0x0F ) ) )

// Unpack the X component from a byte packed wall-kick.
#define UNPACK_X(xy) \
    ( ( ( ( wall_kick >> 4 ) & 0x0F ) ^ 0b1000 ) - 0b1000 )

// Unpack the Y component from a byte packed wall-kick.
#define UNPACK_Y(xy) \
    ( ( ( wall_kick & 0x0F ) ^ 0b1000 ) - 0b1000 );

/**
    ^
    Found this doing some searching to extend the sign : https://stackoverflow.com/questions/5814072/sign-extend-a-nine-bit-number-in-c
    This lead me to this: https://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
    Super neat. Essentially what it does is get the MSB (sign), XOR with the value to remove the sign, and then subtract it to get it to become negative.
    This is known as Sign Extension (or SEXT...)
    Sign extension is normally done automatically for 8 bit to 16 bit, 16 to 32, etc.
    Because there is no such thing as a 4 bit (nibble, non addressable), we have to do it manually!
*/

// Intended to be used for coloring (or debugging).
typedef enum {
    I_TYPE = 1,
    O_TYPE,
    T_TYPE,
    S_TYPE,
    Z_TYPE,
    J_TYPE,
    L_TYPE
} PieceType; // Probably 8 bits.

// Used for collision checks and rotating. 
typedef enum {
    NONE_2X2 = 2,
    ROT_3X3,
    ROT_4X4
} PieceSize; // Probably 8 bits.

typedef uint16_t        PieceCells; // Type-alias for piece cells into a 4x4 bit matrix (16 bit unsigned int). Read from LSB to MSB.
typedef uint8_t         WallKick;   // Type-alias for wall-kick packed into 8 bit unsigned int.     
typedef WallKick        PieceRotationWallKicks[PIECE_ROTATION_STATES * PIECE_ROTATION_DIRECTIONS][PIECE_ROTATION_TESTS - 1]; 
// I like the strategy of type-aliasing for 2d array pointers: https://stackoverflow.com/questions/14808908/pointer-to-2d-arrays-in-c

PieceCells  get_rotated_piece_cells(PieceCells in_cells, PieceSize size, bool clockwise); // Rotate given cells of a block clockwise or counter clockwise.
bool        is_piece_cell(PieceCells cells, uint8_t pos_x, uint8_t pos_y);

extern const PieceRotationWallKicks WALL_KICKS_TSZJL;
extern const PieceRotationWallKicks WALL_KICKS_I;

typedef struct {                                        // This is where the cells, type, size, as well as pointer to the proper rotation wall-kicks are stored (since all piece but 'I' share the same). 
    const PieceRotationWallKicks* rotation_wall_kicks;  // 4-8 bytes
    PieceCells cells;                                   // 2 bytes
    PieceType type;                                     // 1 byte
    PieceSize size;                                     // 1 byte
} PieceData; 

extern const PieceData I_DATA;  
extern const PieceData O_DATA;
extern const PieceData T_DATA;
extern const PieceData S_DATA;
extern const PieceData Z_DATA;
extern const PieceData J_DATA;
extern const PieceData L_DATA;
extern const PieceData* ALL_PIECE_DATA[PIECE_COUNT];

const PieceData* get_piece_data(PieceType piece_type);

typedef struct {                                        // Piece entity (container for data, transform, and lock components)
    const PieceRotationWallKicks* rotation_wall_kicks;  // 4-8 bytes
	float velo_x;                                       // 4 bytes
	float velo_y;                                       // 4 bytes
    float timer;                                        // 4 bytes
	PieceCells cells;                                   // 2 bytes
	PieceType type;                                     // 1 byte
	PieceSize size;                                     // 1 byte
	uint8_t rotation;                                   // 1 byte
	uint8_t pos_x;                                      // 1 byte
	uint8_t pos_y;                                      // 1 byte
	uint8_t moves;                                      // 1 byte
	bool on_ground;                                     // 1 byte
} Piece;

// PieceData related functions.
void copy_data_into_piece(Piece* piece, PieceData* piece_data);
// PieceTransform related functions.
void set_piece_position(Piece* piece, uint8_t x, uint8_t y);
void reset_piece_velocity(Piece* piece);
void reset_piece_transform(Piece* piece);
// PieceLock related functions.
void reset_piece_lock(Piece* piece);

/**  
    How Wall Kicks Work:
    - A piece has 4 states of rotation. They are identified by the following: '0', 'R', '2', and 'L'. 
        - A piece can get to 'L' by one *left* rotation, OR by three *right* rotations. Same can be said for '2' with two *left* or two *right*.
    - A piece keeps track of a rotation index which is a number 1-4, but the wall-kick tests are within indices of 0-7.
        - The reason for the larger range for wall-kicks is because they are determined by a directed graph.
        - In example, we are in state '0', and intend to make a *left* rotation. That is the edge '0->L'.
        - To get the proper wall-kick for that edge, we multiply our rotation index by two, and add nothing if we are turning *left*, or add one if we are turning *right*.
        - This is the reason PieceRotationWallKicks are a 2D array of row size 8.
    - Once a piece is rotated, cell collision should be checked. (Cell collision can also be outside of bounds).
        - If cell collision, go through 4 wall-kick tests given the above adjusted rotation index until no cell collision.
    - Wall-kicks are packed into an 8 bit unsigned int, so they are properly unpacked first.
    - Wall-kicks describe position adjustments to the piece.
    - If there is no rotation that can be made, no rotation is made (simple as that!)
    - If there is a rotation that can be made, we write to our piece's rotation index: (index + 4 +- direction) & 3
*/

#ifdef DEBUG
#include <stdio.h>
extern const char* STATES;
void        print_cells(PieceCells cells);    // Print a block using puts(). Used for debugging purposes.
void        print_all_piece_rotations();    // Print all blocks and their rotations.
void        print_all_wall_kicks();
#endif // DEBUG

#endif // PIECE_H