#include <stdint.h>

#include "game.h"
#include "engine.h"
#include "raylib.h"

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	450
#define TARGET_FPS		60

const int		CELL_SIZE = 20;
const Vector2	CENTER_OF_SCREEN = { (float)SCREEN_WIDTH / 2, (float)SCREEN_HEIGHT / 2 };
Vector2			SCREEN_START;
Vector2			PLAYFIELD_SIZE; 
//uint8_t PIECE_BUFFER[VISIBLE_ROW_COUNT][VISIBLE_COLUMN_COUNT]; // TODO: colors

uint8_t GetActionBitFlags()
{
	uint8_t inputBitFlags = 0;
	if (IsKeyDown(KEY_SPACE))                        inputBitFlags |= ACTION_HARD_DROP;
	if (IsKeyDown(KEY_H) || IsKeyDown(KEY_R))        inputBitFlags |= ACTION_HOLD_PIECE;
	if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))     inputBitFlags |= ACTION_SOFT_DROP;
	if (IsKeyDown(KEY_E) || IsKeyDown(KEY_UP))       inputBitFlags |= ACTION_ROTATE_CLOCKWISE;
	if (IsKeyDown(KEY_Q))                            inputBitFlags |= ACTION_ROTATE_COUNTER;
	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))    inputBitFlags |= ACTION_MOVE_RIGHT;
	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))     inputBitFlags |= ACTION_MOVE_LEFT;
	return inputBitFlags;
}

void RenderFrame(const Game* game)
{
	BeginDrawing();
		ClearBackground(BLACK);
		DrawRectangle(SCREEN_START.x, SCREEN_START.y, PLAYFIELD_SIZE.x, PLAYFIELD_SIZE.y, DARKGRAY);
		const int controlledBlockXOffset = game->controlled_piece.pos_x;
		const int controlledBlockYOffset = game->controlled_piece.pos_y;
		const int ghostBlockYOffset = game->controlled_piece_ground_y;
		for (uint8_t y = game->playfield.ceiling; y < game->playfield.row_count; y++)
		{
			const int controlledYAdjusted = y - controlledBlockYOffset;
			const int ghostYAdjusted = y - ghostBlockYOffset;
			for (uint8_t x = COLUMN_OFFSET; x < game->playfield.column_count + COLUMN_OFFSET; x++)
			{
				const int controlledXAdjusted = x - controlledBlockXOffset;
				if (controlledXAdjusted >= 0 && controlledXAdjusted < game->controlled_piece.size &&
					controlledYAdjusted >= 0 && controlledYAdjusted < game->controlled_piece.size &&
					is_piece_cell(game->controlled_piece.cells, controlledXAdjusted, controlledYAdjusted))
				{
					DrawRectangle(SCREEN_START.x + (x - COLUMN_OFFSET) * CELL_SIZE, SCREEN_START.y + (y - game->playfield.ceiling) * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);
				}
				else if (controlledXAdjusted >= 0 && controlledXAdjusted < game->controlled_piece.size &&
					ghostYAdjusted >= 0 && ghostYAdjusted < game->controlled_piece.size &&
					is_piece_cell(game->controlled_piece.cells, controlledXAdjusted, ghostYAdjusted))
				{
					DrawRectangle(SCREEN_START.x + (x - COLUMN_OFFSET) * CELL_SIZE, SCREEN_START.y + (y - game->playfield.ceiling) * CELL_SIZE, CELL_SIZE, CELL_SIZE, ORANGE);
				}
				else if (is_playfield_cell(&game->playfield, x, y))
				{
					DrawRectangle(SCREEN_START.x + (x - COLUMN_OFFSET) * CELL_SIZE, SCREEN_START.y + (y - game->playfield.ceiling) * CELL_SIZE, CELL_SIZE, CELL_SIZE, YELLOW);
				}
			}
		}
	EndDrawing();
}

void game_loop()
{ 
	PLAYFIELD_SIZE = (Vector2){ (float)(CELL_SIZE * DEFAULT_COLUMN_COUNT), (float)(CELL_SIZE * (DEFAULT_ROW_COUNT - DEFAULT_CEILING) )};
	SCREEN_START = (Vector2){ CENTER_OF_SCREEN.x - PLAYFIELD_SIZE.x / 2.0, CENTER_OF_SCREEN.y - PLAYFIELD_SIZE.y / 2.0 };
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Zetris");
        SetTargetFPS(TARGET_FPS);
        Game game = get_default_initialized_game();
        while(!WindowShouldClose())
        {
            if(is_game_over(&game)) break;
            //on_tick(&game, GetFrameTime(), GetActionBitFlags());
            on_tick(&game, GetFrameTime(), GetActionBitFlags());
            RenderFrame(&game);
        }
    CloseWindow();
}