#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    #ifdef DEBUG
        PrintAllBlockRotations();
        PrintAllWallKicks();
    #endif // DEBUG

    srand(time(NULL)); // Seed with argument if given
    game_loop();
}