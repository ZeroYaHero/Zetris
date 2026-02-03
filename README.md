<div align="center">

<img src="ZETRIS_LOGO.png" width="384">

<img src="ZETRIS_GAME.png" width="512">

</div>

## Rules
Zetris is a game where you place a set of pieces into an enclosed grid (called the Playfield) one at a time.
When a row (or line) is filled the cells vanish, and the subsequent rows (or lines) above fall simulating gravity. 
As time goes on, the pieces start to fall faster. You don't want the pieces to reach above the visible screen, or else its game over.
You can move horizontally, drop, or rotate each piece as its falling. You can even hold a piece for later, but you cannot hold pieces back to back. Good luck!

## Clone, Build & Run

### Clone
```
git clone https://github.com/ZeroYaHero/Zetris.git
```
Navigate to repository:
```
cd zetris
```
### Build
Configure and generate build files under `build` directory using `CMakeLists.txt` in project root directory:
```
cmake -S . -B build
```
You can also specify CMake build type: Debug adds some additional prints for sanity tests:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=[<build-type>]
```
Build in build directory
```
cmake --build build
```

### Run
```
zetris.exe
```

## Project Structure
Generally, the project is structured so `piece.h` and `playfield.h` are independent of the others implementation. They do not include eachother, and instead contain only relevant utility. They are connected in `game.h` which assumes the presence of both.

The `game.h` is where the logic is... in a way it could be considered the engine... but truly Zetris is a game, and it does not try to be more than that. It aims to be an interface (or rather to be plugged-in-and-played) with an `engine.h` implementation. Engine may not be the best word for what this structure resembles, but for rendering and input it does not seem too far off? Admittedly, it does seem backwards that the game does not rely on the engine, but instead the engine relies on the game. Although it may seem contradictory at first, it actually makes the game incredibly portable. I think there is potential in being able to write a games logic completely independent of renderer or input capturing devices.

Instead of global constants, logic is contained within structs to encapsulate context (`Piece` and `Playfield` inside `Game`). The idea is in the future it wouldn't be too hard to extend the project to allow two players on the same machine.

## `piece.h` 
 `PieceData` struct declaration and definition. This contains the cell layout, type, wall-kick data pointer, and size.
 
 `Piece` struct declaration and definition. The `Piece` contains three different "components"
- Data: *Same data members as mentioned before for `PieceData`*
- Transform: position, velocity, and rotation.
- Lock: data relating to locking a piece in the playfield.

The `Piece` struct does not contain the components as literal nested structs. It was considered it at first, but it just ends up complicating accessing data members. It also makes it harder to align members with minimal padding. 

There is also all the "prefabs" for the unique `PieceData`'s I, O, T, S, Z, J, and L.

Header also contains piece limits defined with preprocessor symbols, and preprocessor functions for packing and unpacking wall-kick data.
## `playfield.h`
Contains declaration and definition of the `Playfield` struct. As well as declarations for utility functions that help query or modify it. Lots of limits defined with preprocessor symbols.

## `game.h`
All the game logic functions are declared here. Data for each game is accessed via a declared and defined `Game` struct, which contains...
- Controlling piece
- Playfield struct (static cells)
- Held `PieceData` pointer
- Piece queue (array of `PieceData` and respective index)
- Score
- Level index

`Level` struct declaration and definition. A level contains gravity speed and the number of lines that need to be cleared before moving to the next level.

"Actions" which is represented in a 8 bit integer and uses bit flags. This is how the game processes input every tick/frame. Supplying the game the bit flags is implementation based.

## `engine.h`
`game_loop` function... Thats it!

## Attempted Low Memory Footprint
In Zetris, collision detection and piece placement is done with bitwise operators. A zero represents the absence of a cell while a one represents the presence of a cell. This is true for both Pieces and the Playfield.

Pieces are contained inside a 4 by 4 boolean matrix (though Pieces can technically be smaller). This is achieved by a 16 bit unsigned integer, and aligning the elements with the least significant bit.

The Playfield, at max, is a 32 by 32 "bitboard." This is achieved by a 32 element array of 32 bit unsigned integers. This technically overshoots the necessary amount of cells for the default game (20 by 10), but it is intended to be flexible so the Playfield could be extended if desired. 

Due to the maximum indices of the Playfield being 31 and 31, and indices cannot be negative, Piece positions are encoded in 8 bit unsigned integers. Though, there is a little bit of jank associated with this decision, as the positions have to be shifted by 2 "columns." I wish I could do a good job explaining this here, but I just recommend reading the code.

Structs are aligned for minimal padding, with the data members which require the largest address divisibility at the top. When possible, structs are nested directly without pointers to prevent the CPU from needing to do a second read (though, this is an area I want to learn more about). 

Functions use pointers when the pointer would be smaller than passing the struct or data by value. Otherwise, pass by value is used.

There is only one `malloc` call and its in a `shuffle` function (not mine). Technically there is no need for the `malloc` due to the fact that the size is known before compiling, but it's just a symptom of me being lazy to change it when it simply works. Everything else is within the stack. Though, I don't think this is necessary a "flex." Knowing when and how to manage dynmically allocated memory on the heap I think is a valuable skill. However, at the games current state I do not see a reason to use much heap allocation.