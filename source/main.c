#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "engine.h"
#include "game.h"

int main(int argc, char* argv[])
{
    #if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
        print_all_piece_rotations();
        print_all_wall_kicks();
    #endif // DEBUG

    srand(time(NULL)); // TODO: Seed with argument if given
    game_loop();
}