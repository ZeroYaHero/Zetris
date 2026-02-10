#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Defaults
#define DEFAULT_ROW_COUNT           20
#define DEFAULT_COLUMN_COUNT        10
#define DEFAULT_CEILING             4
// Limits
#define COLUMN_OFFSET               2
#define MAX_ROW_COUNT               32
#define MAX_COLUMN_COUNT            32

typedef uint32_t PlayfieldCells[MAX_ROW_COUNT];

// Contains the locked/static cells in the playfield, as well as some "boundaries."
typedef struct {
    PlayfieldCells cells;       // 4 * MAX_ROW_COLUMN_COUNT bytes
    uint32_t lines_cleared;     // 4 bytes
    uint8_t row_count;          // 1 byte
    uint8_t column_count;       // 1 byte
    uint8_t ceiling;            // 1 byte
} Playfield;

bool    is_outside_bounds(const Playfield* playfield, const uint8_t, const uint8_t pos_y);                    // If position is outside bounds.
bool    is_playfield_cell(const Playfield* playfield, const uint8_t pos_x, const uint8_t pos_y);              // Checks if cell or empty.
bool    are_cells_above_ceiling(const Playfield* playfield);                                      // Determines if cells in the playfield are above the ceiling.
bool    attempt_add_playfield_cell_at(Playfield* playfield, uint8_t pos_x, uint8_t pos_y);  // Write bit (cell) in playfield
uint8_t clear_filled_lines(Playfield* playfield, uint8_t bottom_offset);                    // Starts from bottom and moves up to clear rows. Returns the number of rows it cleared for given playfield.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PLAYFIELD_H