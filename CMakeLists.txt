cmake_minimum_required(VERSION 3.16)
project(rompecabezas_sdl C)
set(CMAKE_C_STANDARD 11)

# Requiere vcpkg para SDL2 y SDL2_image:
#   vcpkg install sdl2 sdl2-image
#   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
#   cmake --build build --config Release

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_executable(rompecabezas_sdl
    sdl_sliding_puzzle.c
)

target_link_libraries(rompecabezas_sdl PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
)
