#include <stdlib.h>

#include "playfield.h"

bool is_outside_bounds(const Playfield* playfield, const uint8_t pos_x, const uint8_t pos_y)
{
    return pos_x >= (playfield->column_count + COLUMN_OFFSET) || pos_y >= playfield->row_count || pos_x < COLUMN_OFFSET;
}

bool is_playfield_cell(const Playfield* playfield, const uint8_t pos_x, const uint8_t pos_y)
{
    return is_outside_bounds(playfield, pos_x, pos_y) || (playfield->cells[pos_y] & (1U << (pos_x - COLUMN_OFFSET)));
}

bool are_cells_above_ceiling(const Playfield* const playfield)
{
	for (uint8_t y = playfield->ceiling; y > 0; y--)
	{
		if (playfield->cells[y - 1]) return true;
	}
	return false;
}

bool attempt_add_playfield_cell_at(Playfield* playfield, const uint8_t pos_x, const uint8_t pos_y)
{
	if (!is_outside_bounds(playfield, pos_x, pos_y))
	{
		playfield->cells[pos_y] |= (1U << (pos_x - COLUMN_OFFSET));
        return true;
	}
    return false;
}

// TODO: Change this to filled lines (or others to filled rows).
uint8_t clear_filled_lines(Playfield* playfield, const uint8_t pos_y)
{
    uint8_t y = (pos_y > playfield->row_count) ? playfield->row_count : pos_y;

    const uint32_t mask = (1U << playfield->column_count) - 1;
    uint8_t rows_cleared = 0;
    for (; y > 0; y--)
    {
        if (playfield->cells[y - 1] == mask)
        {
            rows_cleared++;
        }
        else if (rows_cleared)
        {
            playfield->cells[y - 1 + rows_cleared] = playfield->cells[y - 1];
            playfield->cells[y - 1] = 0;
        }
    }
    playfield->lines_cleared += rows_cleared;
    return rows_cleared;
}