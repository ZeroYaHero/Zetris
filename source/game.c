#include <math.h>

#include "game.h"
#include "util.h"

Game get_default_initialized_game()
{
    Game game = {
        .score = 0,
        .held_piece = 0,
        .piece_queue = 0,
        .controlled_piece = {0},
        .playfield = {
            .cells = {0},
            .lines_cleared = 0,
            .row_count = DEFAULT_ROW_COUNT,
            .column_count = DEFAULT_COLUMN_COUNT,
            .ceiling = DEFAULT_CEILING
        },
        .level_index = 0,
        .piece_queue_index = 0,
        .cleared_lines_last_piece = 0,
        .combo_count = 0,
        .can_hold_piece = (SETTINGS_DEFAULT & SETTING_CAN_HOLD),
        .setting_bit_flags = SETTINGS_DEFAULT,
        .previous_action_bit_flags = 0
	};
    memcpy(game.piece_queue, ALL_PIECE_DATA, PIECE_COUNT * sizeof(PieceData*));
    shuffle(game.piece_queue, PIECE_COUNT, sizeof(PieceData*));
    reset_controlled_piece(&game, pop_piece_queue(&game));
	return game;
}

void tick(Game* game, double delta_time, ACTION_BIT_FLAGS action_bit_flags)
{
    // Action bit flags that are not intended to be long pressed. 
    ACTION_BIT_FLAGS unique_action_bit_flags = (action_bit_flags ^ game->previous_action_bit_flags) & action_bit_flags;
    // Lock reset event
    LOCK_RESET_BIT_FLAGS lock_reset_bit_flags = 0;
    // Hold
    if (unique_action_bit_flags & ACTION_HOLD_PIECE && game->can_hold_piece && game->setting_bit_flags & SETTING_CAN_HOLD)
    {
        PieceData* to_be_held = get_piece_data(game->controlled_piece.type);
        reset_controlled_piece(game, (game->held_piece) ? game->held_piece : pop_piece_queue(game));
        game->held_piece = to_be_held;
        game->can_hold_piece = false;
    }
    // Hard drop
    else if (unique_action_bit_flags & ACTION_HARD_DROP)
    {
        uint8_t hard_drop_y = get_playfield_piece_cells_hard_drop_y(
            &game->playfield, 
            game->controlled_piece.cells, 
            game->controlled_piece.size,
            game->controlled_piece.pos_x,
            game->controlled_piece.pos_y
        );
        set_piece_position(&game->controlled_piece, game->controlled_piece.pos_x, hard_drop_y);
        on_controlled_piece_place(game);
    }
    // Rotation, Movement, and Gravity
    else
    {
        if (unique_action_bit_flags & ACTION_ROTATE_CLOCKWISE || unique_action_bit_flags & ACTION_ROTATE_COUNTER)
        {
            if (attempt_rotate_piece(&game->playfield, &game->controlled_piece, unique_action_bit_flags & ACTION_ROTATE_CLOCKWISE))
            {
                lock_reset_bit_flags |= LOCK_RESET_ROTATE;
            }
        }

        // Essentially, this does: hey, do we have stored velocity for the opposite direction of this frames action?
        // If so, it simply will reset the velocity to 0 so we don't have to "gain back" what was lost.
        uint8_t x_distance_traveled = 0;
        if (unique_action_bit_flags & ACTION_MOVE_RIGHT) 
        {
            game->controlled_piece.velo_x = 0.0f;
            x_distance_traveled = attempt_move_piece_until_collision(&game->playfield, &game->controlled_piece, 1, 0, 1);
        }
        else if (unique_action_bit_flags & ACTION_MOVE_LEFT)
        {
            game->controlled_piece.velo_x = 0.0f;
            x_distance_traveled = attempt_move_piece_until_collision(&game->playfield, &game->controlled_piece, -1, 0, 1);
        }
        else 
        {
			game->controlled_piece.velo_x += ((action_bit_flags & ACTION_MOVE_RIGHT ? HORIZONTAL_VELOCITY : 0.0f) + (action_bit_flags & ACTION_MOVE_LEFT ? -HORIZONTAL_VELOCITY : 0.0f)) * delta_time;
			uint8_t x_distance = (uint8_t)(fabsf(game->controlled_piece.velo_x)); // This could lead to weird shit, probably would be better to clamp first.
			if (x_distance) // If there is greater than 0 cell distance
			{
				int8_t x_direction = SIGN(game->controlled_piece.velo_x);
                x_distance_traveled = attempt_move_piece_until_collision(&game->playfield, &game->controlled_piece, x_direction, 0, x_distance);
                if (!x_distance_traveled) // If the piece couldn't move a single cell
                {
                    game->controlled_piece.velo_x = 0.0f;                           // Reset velocity. This is okay since we move anyways on first input.
                    action_bit_flags &= ~(ACTION_MOVE_RIGHT & ACTION_MOVE_LEFT);    // Remove the intent to move right or left so we can reconsider next frame. Just don't want velocity to get too high.
                }
                else
                {
                    game->controlled_piece.velo_x += (-x_direction * x_distance_traveled);
                }
			}
        }
		if (x_distance_traveled)
		{
			lock_reset_bit_flags |= LOCK_RESET_MOVE;
		}
        
        // Gravity & soft drop
        game->controlled_piece.velo_y += ((action_bit_flags & ACTION_SOFT_DROP ? VERTICAL_VELOCITY : 0.0f) + ALL_LEVELS[game->level_index].gravity) * delta_time;
		uint8_t y_distance = (uint8_t)(fabsf(game->controlled_piece.velo_y)); // This could lead to weird shit, probably would be better to clamp first.
		if (y_distance)
		{
			int8_t y_direction = SIGN(game->controlled_piece.velo_y);
			uint8_t y_velocity_consumed = attempt_move_piece_until_collision(&game->playfield, &game->controlled_piece, 0, y_direction, y_distance);
			if (y_velocity_consumed)
			{
                game->controlled_piece.velo_y += (-y_direction * y_velocity_consumed);
                lock_reset_bit_flags |= LOCK_RESET_DROP;
			}
		}
    }

    // Ghost
    game->controlled_piece_ground_y = get_playfield_piece_cells_hard_drop_y(
        &game->playfield, 
        game->controlled_piece.cells, 
        game->controlled_piece.size, 
        game->controlled_piece.pos_x,
        game->controlled_piece.pos_y
    );
    
    // Ground check using Ghost
    if (game->controlled_piece_ground_y == game->controlled_piece.pos_y)
    {
        game->controlled_piece.on_ground = true;
        game->controlled_piece.velo_y = 0.0f;
    }

	// Lock
	if (game->controlled_piece.on_ground) // on_ground is read from the previous frame AFTER current movement.
	{
		if (lock_reset_bit_flags & LOCK_RESET_DROP)
		{
			reset_piece_lock(&game->controlled_piece);
		}
		else if (lock_reset_bit_flags & (LOCK_RESET_ROTATE | LOCK_RESET_MOVE))
		{
			game->controlled_piece.timer = 0.0f;
			if (!(game->setting_bit_flags & SETTING_INFINITE_LOCK_DELAY))
			{
				game->controlled_piece.moves++;
				if (game->controlled_piece.moves >= MAX_MOVES_BEFORE_LOCK)
				{
					on_controlled_piece_place(game);
				}
			}
		}
		else
		{
			game->controlled_piece.timer += delta_time;
			if (game->controlled_piece.timer >= LOCK_DELAY)
			{
				on_controlled_piece_place(game);
			}
		}
	}

    if (game->level_index < LEVEL_COUNT - 1 &&
        game->playfield.lines_cleared >= ALL_LEVELS[game->level_index].lines_cleared)
    {
        game->level_index++;
    }

    game->previous_action_bit_flags = action_bit_flags;
}

bool is_game_over(Game* game)
{
    return are_cells_above_ceiling(&game->playfield);
}

void reset_game(Game* game)
{
    exit(1);
}

bool are_playfield_piece_cells_colliding(Playfield* const playfield, const PieceCells piece_cells, const PieceSize piece_size, const uint8_t pos_x, const uint8_t pos_y)
{
    uint8_t visited_piece_cells = 0;
    for (uint8_t y = 0; y < piece_size; y++)
    {
    	for (uint8_t x = 0; x < piece_size; x++)
    	{
    		// If there is a cell in this block position
    		if (is_piece_cell(piece_cells, x, y))
    		{
    			visited_piece_cells++;
    			// If there is a cell in this playfield position
    			if (is_playfield_cell(playfield, pos_x + x, pos_y + y))
    			{
    				return true;
    			}
    			if (visited_piece_cells == PIECE_CELL_COUNT)
    			{
    				return false;
    			}
    		}
    	}
    }
    return false;
    //for (uint8_t y = 0; y < piece_size; y++)
    //{
    //    uint8_t piece_row_cells = (uint8_t)((piece_cells >> (y * PIECE_MAX_SIZE)) & 0x0F);
    //    // If not out of bounds
    //    if ((pos_y + y) < playfield->row_count)
    //    {
    //        // If concerned with left wall
    //        if (pos_x < COLUMN_OFFSET && piece_row_cells & ~(UINT32_MAX << (COLUMN_OFFSET - pos_x)))
    //         {
    //            return true;
    //        }
    //        // If concerned with right wall
    //        if (piece_row_cells & (UINT32_MAX << (playfield->column_count - (pos_x - COLUMN_OFFSET))))
    //        {
    //            return true;
    //        }
    //        // If no wall collision, check for playfield collision.
    //        if ((playfield->cells[pos_y + y] >> (pos_x - COLUMN_OFFSET)) & piece_row_cells)
    //        {
    //            return true;
    //        }
    //    }
    //    // If out of bounds and there are piece row cells
    //    else if (piece_row_cells)
    //    {
    //        return true;
    //    }
    //}
    //return false;
}

bool are_piece_cells_on_playfield_ground(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y)
{
	const bool do_cells_collide = are_playfield_piece_cells_colliding(playfield, piece_cells, piece_size, pos_x, pos_y);
	const bool do_lowered_cells_collide = are_playfield_piece_cells_colliding(playfield, piece_cells, piece_size, pos_x, pos_y + 1);
	return !do_cells_collide && do_lowered_cells_collide;
}

bool attempt_rotate_piece(Playfield* playfield, Piece* piece, bool clockwise)
{
    if (piece->size == NONE_2X2) return true;

    const uint8_t rotation_tests_index = piece->rotation + (clockwise ? 1 : 0);
    const PieceCells rotated_cells = get_rotated_piece_cells(piece->cells, piece->size, clockwise);

    for (uint8_t test_index = 0; test_index < PIECE_ROTATION_TESTS; test_index++)
    {
		int8_t x_wall_kick = 0;
		int8_t y_wall_kick = 0;
        if (test_index > 0)
        {
            const WallKick wall_kick = piece->rotation_wall_kicks[rotation_tests_index][test_index - 1];
            x_wall_kick = UNPACK_X(wall_kick);
            y_wall_kick = -UNPACK_Y(wall_kick); // Flip sign since board is "upside down".
        }

        if (((piece->pos_x + x_wall_kick) >= 0) &&
            ((piece->pos_y + y_wall_kick) >= 0) && 
            !are_playfield_piece_cells_colliding(playfield, rotated_cells, piece->size, piece->pos_x + x_wall_kick, piece->pos_y + y_wall_kick))
        {
            piece->cells = rotated_cells;
			piece->rotation = (piece->rotation + PIECE_ROTATION_STATES + ((clockwise) ? 1 : -1)) & (PIECE_ROTATION_STATES - 1);
            set_piece_position(piece, (uint8_t)(piece->pos_x + x_wall_kick), (uint8_t)(piece->pos_y + y_wall_kick));
            return true;
        }
    }
    return false;
}

uint8_t attempt_move_piece_until_collision(Playfield* playfield, Piece* piece, int8_t x_direction, int8_t y_direction, uint8_t distance)
{
    uint8_t traveled = 0;
    for (; traveled < distance; traveled++)
    {
		if (((piece->pos_x + x_direction) < 0) ||
			((piece->pos_y + y_direction) < 0) ||
			are_playfield_piece_cells_colliding(playfield, piece->cells, piece->size, piece->pos_x + x_direction, piece->pos_y + y_direction))
		{
            break;
        }
        set_piece_position(piece, (uint8_t)(piece->pos_x + x_direction), (uint8_t)(piece->pos_y + y_direction));
    }
    return traveled;
}

uint8_t get_playfield_piece_cells_hard_drop_y(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y)
{
    for (; pos_y < playfield->row_count; pos_y++)
    {
        if (are_piece_cells_on_playfield_ground(playfield, piece_cells, piece_size, pos_x, pos_y))
        {
            break;
        }
    }
    return pos_y;
}

void lock_piece_cells_in_playfield(Playfield* playfield, PieceCells piece_cells, PieceSize piece_size, uint8_t pos_x, uint8_t pos_y)
{
	uint8_t visited_piece_cells = 0;
	for (uint8_t y = 0; y < piece_size; y++)
	{
		for (uint8_t x = 0; x < piece_size; x++)
		{
			if (is_piece_cell(piece_cells, x, y))
			{
				visited_piece_cells++;
				attempt_add_playfield_cell_at(playfield, pos_x + x, pos_y + y);
				if (visited_piece_cells == 4) return;
			}
		}
	}

	//for (uint8_t y = 0; y < piece_size; y++)
    //{
    //    uint8_t piece_row_cells = (uint8_t)((piece_cells >> (PIECE_MAX_SIZE * y)) & 0x0F);
    //    if (pos_x < COLUMN_OFFSET)
    //    {
    //        playfield->cells[pos_y + y] |= ((uint32_t)(piece_row_cells) >> (COLUMN_OFFSET - pos_x));
    //    }
    //    else
    //    {
    //        playfield->cells[pos_y + y] |= ((uint32_t)(piece_row_cells) << (pos_x - COLUMN_OFFSET));
    //    }
    //}
}

void reset_controlled_piece(Game* game, PieceData* optional_piece_data)
{
    reset_piece_lock(&game->controlled_piece);
    reset_piece_velocity(&game->controlled_piece);
    if (optional_piece_data)
    {
        copy_data_into_piece(&game->controlled_piece, optional_piece_data);
        game->controlled_piece.rotation = 0;
    }
    set_piece_position(&game->controlled_piece, (game->playfield.column_count / 2) - (game->controlled_piece.size / 2) + COLUMN_OFFSET, 0);
}

void on_controlled_piece_place(Game* game)
{
    lock_piece_cells_in_playfield(
        &game->playfield,
        game->controlled_piece.cells,
        game->controlled_piece.size,
        game->controlled_piece.pos_x,
        game->controlled_piece.pos_y
    );
    uint8_t cleared_lines = clear_filled_lines(&game->playfield, game->controlled_piece.pos_y + game->controlled_piece.size);
    if (cleared_lines)
    {
        switch (cleared_lines)
        {
        case 1:
            game->score += (game->level_index + 1) * 100;
            break;
        case 2:
            game->score += (game->level_index + 1) * 300;
            break;
        case 3:
            game->score += (game->level_index + 1) * 500;
            break;
        case 4:
            game->score += (game->level_index + 1) * 800;
            break;
        }
        game->score += 50 * game->combo_count * (game->level_index + 1);
        game->combo_count++;
        game->cleared_lines_last_piece = cleared_lines;
    }
    else
    {
        game->combo_count = 0;
    }
    reset_controlled_piece(game, pop_piece_queue(game));
    game->can_hold_piece = true;
}

PieceData* pop_piece_queue(Game* game)
{
	PieceData* retval = game->piece_queue[game->piece_queue_index];
	game->piece_queue_index++;
	if (game->piece_queue_index >= PIECE_COUNT)
	{
		game->piece_queue_index = 0;
		shuffle(game->piece_queue, PIECE_COUNT, sizeof(PieceData*));
	}
	return retval;
}

PieceData* top_piece_queue(Game* game)
{
	return game->piece_queue[game->piece_queue_index];
}

const Level ALL_LEVELS[LEVEL_COUNT] = {
	{
		.gravity = 1.75f,
		10
	},
	{
		.gravity = 3.0f,
		25
	},
	{
		.gravity = 5.0f,
		50
	},
	{
		.gravity = 8.0f,
		75
	},
	{
		.gravity = 10.0f,
		100
	},
	{
		.gravity = 15.0f,
		125
	},
	{
		.gravity = 18.0f,
		175
	},
	{
		.gravity = 20.0f,
		0
	}
};