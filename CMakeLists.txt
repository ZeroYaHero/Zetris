cmake_minimum_required(VERSION 3.25)
project("zetris" LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(EXTERN_DIR "${CMAKE_SOURCE_DIR}/external")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/source")

add_executable(zetris
    "${SRC_DIR}/main.c"
    "${SRC_DIR}/game.c"
    "${SRC_DIR}/piece.c"
    "${SRC_DIR}/playfield.c"
)
target_include_directories(zetris PRIVATE "${INCLUDE_DIR}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(zetris PRIVATE DEBUG)
endif()

if(ENGINE_TYPE MATCHES Terminal)
    target_sources("zetris" PRIVATE "${SRC_DIR}/terminal.c")
    target_compile_definitions(zetris PRIVATE TERMINAL_ENGINE)
else()
    target_sources("zetris" PRIVATE "${SRC_DIR}/raylib.c")
    target_compile_definitions(zetris PRIVATE RAYLIB_ENGINE)
    # RayGUI
    set(RAYGUI_DIR "${EXTERN_DIR}/raygui")
    set(RAYGUI_SRC_DIR "${RAYGUI_DIR}/src")
    target_include_directories(zetris PRIVATE "${RAYGUI_SRC_DIR}")

    # Raylib
    set(RAYLIB_DIR "${EXTERN_DIR}/raylib")
    set(RAYLIB_INCLUDE_DIR "${RAYLIB_DIR}/src")
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

    add_subdirectory("${RAYLIB_DIR}")
    target_link_libraries(zetris PRIVATE raylib)
endif()
