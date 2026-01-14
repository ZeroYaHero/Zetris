<div align="center">

<img src="ZETRIS.png" width="384">

</div>

## Rules
Zetris is a game where you place a set of blocks into an enclosed grid one at a time.
When a row is filled its blocks vanish, and the subsequent rows above fall simulating gravity. As time goes on,
the blocks start to fall faster. You don't want the blocks to reach above the visible screen, or else its game over.
You can move horizontally, drop, or rotate each block as its falling. You can even hold a block for later if you can't fit it quite yet.
## How It Works
- Blocks are set up with a 16 bit integer. Rotated with a function to swap x & y indices.
- Playfield is (at max) a 32 element array of 32 bit integers
- Bitwise math is done to check for collisions
- In order to run the same independent of framerate, deltaTime is used with x and y accumulators to allow for "half" cell velocities.
- There is a terminal & raylib version. Only the raylib version "works." But it is rough!


## Clone, Build & Run

### Clone
```
git clone
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

## Lessons Learned
Wow. This was my first really "pure" C project without any guides and just really googling.
It was a lot of fun, but it was also a PITA. I refactored a ton of it before even just getting it to run, because
I couldn't tell if I was encapsulating too much (making functions have large parameter lists to gather proper context), 
or if I was relying on constants too much. A similar story could be said with the dividing up functions. 
*"Should I divide this function into A, B, and C instead of just ABC?"* This was kind of the best part,
there was no technical difficulties with libraries or reading documentation (minus the T****s wiki). There was just me, the logic, and the memory.

Here and there I "cheated" by looking at other peoples T****s clones, but it was more because I did not trust my solutions even if they worked.
I was like *"No way this works?"* Annoyingly, Google has become pretty unusable making it really hard for
me to happily take an answer to my question without getting a valid resource.


### Inconsistencies I am not happy with
- The existence of IsOnGround, but also using ghostBlockY
- Having information about the playfield also in the game object
- Using uint8_t for "low memory use"

### Things I am happy with
- It works
- Comments that share where I got my inspiration from, or if I feel emotionally okay with the code. Lol.
- Sometimes repeated code is okay if it *makes sense*