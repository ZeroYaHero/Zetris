#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "playfield.h"
#include "blocks.h"

#define ACTION_BIT_FLAGS			uint8_t
#define ACTION_SOFT_DROP			0b00000001
#define ACTION_HARD_DROP			0b00000010
#define ACTION_MOVE_RIGHT			0b00000100
#define ACTION_MOVE_LEFT			0b00001000
#define ACTION_ROTATE_CLOCKWISE		0b00010000
#define ACTION_ROTATE_COUNTER		0b00100000
#define ACTION_HOLD_PIECE			0b01000000
#define ACTION_PAUSE				0b10000000

#define SETTING_BIT_FLAGS           uint8_t
#define SETTING_CAN_HOLD            0b00000001
#define SETTING_INFINITE_LOCK_DELAY 0b00000010
#define SETTINGS_DEFAULT            SETTING_CAN_HOLD

#define LOCK_RESET_BIT_FLAGS        uint8_t
#define LOCK_RESET_ROTATE           0b00000001
#define LOCK_RESET_MOVE             0b00000010
#define LOCK_RESET_DROP             0b00000100

#define LOCK_DELAY                  0.5f
#define MAX_MOVES_BEFORE_LOCK       10

#define LEVEL_COUNT                 20

#define HORIZONTAL_VELOCITY         3.0f
#define VERTICAL_VELOCITY           3.0f

#define SIGN(val) \
    ( (val < 0) ? -1 : 1 )

typedef struct {
    float gravity;          // 4 bytes
    uint16_t lines_cleared; // I had this as a 32 bit integer, but apparently no one has even got near the maximum value of that (or of a 16 bit) https://www.guinnessworldrecords.com/world-records/98479-most-lines-cleared-on-tetris-nes-tengen-version   
} Level;

extern const Level ALL_LEVELS[LEVEL_COUNT];

// I could have done some tricky stuff where I have individual data members here, and then I get the address and cast to proper struct.
// It would be a way to keep the containers intact, but it seems really sketch and messy just readability wise. 
typedef struct {
    uint64_t score;
    PieceData* held_piece;
    PieceData* piece_queue[PIECE_COUNT];
    Piece controlled_piece;
    Playfield playfield;
	uint8_t controlled_piece_ground_y; // TODO: this could go in the controlled_piece...
    uint8_t level_index;
    uint8_t piece_queue_index;
    bool can_hold_piece;
    SETTING_BIT_FLAGS setting_bit_flags;
    ACTION_BIT_FLAGS previous_action_bit_flags;
} Game;

// Game loop functions
Game    get_default_initialized_game();                                             // Returns a game struct which uses defaults from define macros.
void    on_tick(Game* game, double delta_time, ACTION_BIT_FLAGS action_bit_flags);  // Call this every tick, with delta time since last tick, and the actions that were processed.
bool    is_game_over(Game* game);                                                   // Condition to check if game is over (cells above line)
void    reset_game(Game* game);                                                     // Reset the game data that is only tied to a round.

// Game logic functions
bool	    are_playfield_piece_cells_colliding(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y);
bool        are_piece_cells_on_playfield_ground(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y);
bool        attempt_rotate_piece(Playfield* playfield, Piece* piece, bool clockwise);
uint8_t     attempt_move_piece_until_collision(Playfield* playfield, Piece* piece, int8_t x_direction, int8_t y_direction, uint8_t distance); // Intended to be used for one axis at a time.
uint8_t     get_playfield_piece_cells_hard_drop_y(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y);
void        lock_piece_cells_in_playfield(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y);
void        reset_controlled_piece(Game* game, PieceData* optional_piece_data);
void        on_controlled_piece_place(Game* game);
PieceData*  pop_piece_queue(Game* game);
PieceData*  top_piece_queue(Game* game);

#endif // GAME_H