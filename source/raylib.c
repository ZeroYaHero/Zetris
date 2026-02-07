#include <stdint.h>

#include "game.h"
#include "engine.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	450
#define TARGET_FPS		60

// Source: https://youtu.be/w0FSHNzSr_M?si=FdDbWrEIgIq2gdsi
#if defined(_WIN32) && !defined(_DEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

const int		CELL_SIZE = 25;
const Vector2	CENTER_OF_SCREEN = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f};
const Vector2	HELD_PIECE_SIZE = { 200.0f, 200.0f };
const Vector2	PIECE_QUEUE_SIZE = { 200.0f, 200.0f };
const Vector2	PAUSE_CONTINUE_BUTTON_SIZE = { 215.0f, 75.0f };
const Vector2	RESTART_BUTTON_SIZE = { 215.0f, 75.0f };
Vector2			PLAYFIELD_START;
Vector2			PLAYFIELD_SIZE; 
Vector2			HELD_PIECE_START;
Vector2			PIECE_QUEUE_START;
bool			isPaused = false;
bool			pressedEscapeLastTick = false;
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

void DrawPlayfieldAndPiece(const Game* game)
{
	DrawRectangle(PLAYFIELD_START.x, PLAYFIELD_START.y, PLAYFIELD_SIZE.x, PLAYFIELD_SIZE.y, DARKGRAY);
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
				DrawRectangle(PLAYFIELD_START.x + (x - COLUMN_OFFSET) * CELL_SIZE, PLAYFIELD_START.y + (y - game->playfield.ceiling) * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);
			}
			else if (controlledXAdjusted >= 0 && controlledXAdjusted < game->controlled_piece.size &&
				ghostYAdjusted >= 0 && ghostYAdjusted < game->controlled_piece.size &&
				is_piece_cell(game->controlled_piece.cells, controlledXAdjusted, ghostYAdjusted))
			{
				DrawRectangle(PLAYFIELD_START.x + (x - COLUMN_OFFSET) * CELL_SIZE, PLAYFIELD_START.y + (y - game->playfield.ceiling) * CELL_SIZE, CELL_SIZE, CELL_SIZE, ORANGE);
			}
			else if (is_playfield_cell(&game->playfield, x, y))
			{
				DrawRectangle(PLAYFIELD_START.x + (x - COLUMN_OFFSET) * CELL_SIZE, PLAYFIELD_START.y + (y - game->playfield.ceiling) * CELL_SIZE, CELL_SIZE, CELL_SIZE, YELLOW);
			}
		}
	}
}

void DrawPieceQueue(const Game* game)
{
	DrawRectangle(PIECE_QUEUE_START.x, PIECE_QUEUE_START.y, PIECE_QUEUE_SIZE.x, PIECE_QUEUE_SIZE.y, GRAY);
	PieceData* next_piece = top_piece_queue(game);
	if (next_piece)
	{
		for (uint8_t y = 0; y < next_piece->size; y++)
		{
			for (uint8_t x = 0; x < next_piece->size; x++)
			{
				if (is_piece_cell(next_piece->cells, x, y))
				{
					DrawRectangle(PIECE_QUEUE_START.x + (CELL_SIZE * x), PIECE_QUEUE_START.y + (CELL_SIZE * y), CELL_SIZE, CELL_SIZE, GREEN);
				}
			}
		}
	}
}

void DrawHeldPiece(const Game* game)
{
	DrawRectangle(HELD_PIECE_START.x, HELD_PIECE_START.y, HELD_PIECE_SIZE.x, HELD_PIECE_SIZE.y, GRAY);
	if (game->held_piece)
	{
		for (uint8_t y = 0; y < game->held_piece->size; y++)
		{
			for (uint8_t x = 0; x < game->held_piece->size; x++)
			{
				if (is_piece_cell(game->held_piece->cells, x, y))
				{
					DrawRectangle(HELD_PIECE_START.x + (CELL_SIZE * x), HELD_PIECE_START.y + (CELL_SIZE * y), CELL_SIZE, CELL_SIZE, GREEN);
				}
			}
		}
	}
}

void RenderFrame(const Game* game)
{
	DrawFPS(0, 0);
	ClearBackground(BLACK);
	DrawRectangle(PLAYFIELD_START.x, PLAYFIELD_START.y, PLAYFIELD_SIZE.x, PLAYFIELD_SIZE.y, DARKGRAY);
	DrawPlayfieldAndPiece(game);
	DrawHeldPiece(game);
	DrawPieceQueue(game);
	// Level
	DrawText(
		TextFormat("Level: %d", game->level_index + 1),
		PLAYFIELD_START.x + PLAYFIELD_SIZE.x,
		PLAYFIELD_START.y + PIECE_QUEUE_SIZE.y,
		20,
		WHITE
	);
	// Score
	DrawText(
		TextFormat("Score: %d", game->score),
		PLAYFIELD_START.x + PLAYFIELD_SIZE.x,
		PLAYFIELD_START.y + PIECE_QUEUE_SIZE.y + 20,
		20,
		WHITE
	);
	// Lines Cleared
	DrawText(
		TextFormat("Lines Cleared: %d", game->playfield.lines_cleared),
		PLAYFIELD_START.x + PLAYFIELD_SIZE.x,
		PLAYFIELD_START.y + PIECE_QUEUE_SIZE.y + 40,
		20,
		WHITE
	);
}

void OnPlay(Game* game)
{
	tick(game, GetFrameTime(), GetActionBitFlags());

	BeginDrawing();
	RenderFrame(game);
	EndDrawing();
}

bool HandleAndCheckPause()
{
	if (IsKeyDown(KEY_ESCAPE))
	{
		if (!pressedEscapeLastTick)
		{
			isPaused = !isPaused;
			pressedEscapeLastTick = true;
		}
	}
	else
	{
		pressedEscapeLastTick = false;
	}
	return isPaused;
}

void OnPause(Game* game)
{
	BeginDrawing();
	RenderFrame(game);
	int pressedContinue = GuiButton(
		(Rectangle) {
		CENTER_OF_SCREEN.x - PAUSE_CONTINUE_BUTTON_SIZE.x * 0.5f,
		CENTER_OF_SCREEN.y - PAUSE_CONTINUE_BUTTON_SIZE.y - 10.0f,
		PAUSE_CONTINUE_BUTTON_SIZE.x,
		PAUSE_CONTINUE_BUTTON_SIZE.y},
		"Continue"
	);
	int pressedRestart = GuiButton(
		(Rectangle) {
		CENTER_OF_SCREEN.x - RESTART_BUTTON_SIZE.x * 0.5f,
		CENTER_OF_SCREEN.y + 10.0f,
		RESTART_BUTTON_SIZE.x,
		RESTART_BUTTON_SIZE.y},
		"Restart"
	);
	EndDrawing();

	if (pressedContinue) isPaused = false;
	if (pressedRestart)
	{
		isPaused = false;
		*game = get_default_initialized_game(); // Temporary
	}
}

void OnGameOver(Game* game)
{
	BeginDrawing();
	RenderFrame(game);
	const char* gameOverText = "GAME OVER!";
	int gameOverTextWidth = MeasureText(gameOverText, 50);
	DrawText(
		gameOverText,
		CENTER_OF_SCREEN.x - gameOverTextWidth * 0.5f,
		CENTER_OF_SCREEN.y - 150,
		50,
		WHITE
	);

	const char* scoreText = TextFormat("SCORE: %d", game->score);
	int scoreTextWidth = MeasureText(scoreText, 20);
	DrawText(
		scoreText,
		CENTER_OF_SCREEN.x - scoreTextWidth * 0.5f,
		CENTER_OF_SCREEN.y - 100,
		20,
		WHITE
	);

	int pressedRestart = GuiButton(
		(Rectangle) {
		CENTER_OF_SCREEN.x - RESTART_BUTTON_SIZE.x * 0.5f,
		CENTER_OF_SCREEN.y - RESTART_BUTTON_SIZE.y * 0.5f,
		RESTART_BUTTON_SIZE.x,
		RESTART_BUTTON_SIZE.y},
		"Restart"
	);
	EndDrawing();

	if (pressedRestart)
	{
		*game = get_default_initialized_game();
	}
}

//void OnTitleScreen()
//{
//
//}

void game_loop()
{ 
	PLAYFIELD_SIZE = (Vector2){ (float)(CELL_SIZE * DEFAULT_COLUMN_COUNT), (float)(CELL_SIZE * (DEFAULT_ROW_COUNT - DEFAULT_CEILING) )};
	PLAYFIELD_START = (Vector2){ CENTER_OF_SCREEN.x - PLAYFIELD_SIZE.x / 2.0, CENTER_OF_SCREEN.y - PLAYFIELD_SIZE.y / 2.0 };
	HELD_PIECE_START = (Vector2){PLAYFIELD_START.x - HELD_PIECE_SIZE.x, PLAYFIELD_START.y};
	PIECE_QUEUE_START = (Vector2){ PLAYFIELD_START.x + PLAYFIELD_SIZE.x, PLAYFIELD_START.y };
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Zetris");
	//#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
	//if (FileExists("ZETRIS_LOGO.png"))
	//{
	//	Image zetrisLogo = LoadImage("ZETRIS_LOGO.png");
	//	ExportImageAsCode(zetrisLogo, "raylibzetrislogo.h");
	//}
	//#endif // DEBUG
	SetExitKey(KEY_NULL);
	SetTargetFPS(TARGET_FPS);
	Game game = get_default_initialized_game();
	while (!WindowShouldClose())
	{
		if (HandleAndCheckPause())
		{
			OnPause(&game);
		}
		else if (is_game_over(&game))
		{
			OnGameOver(&game);
		}
		else
		{
			OnPlay(&game);
		}
	}
    CloseWindow();
}