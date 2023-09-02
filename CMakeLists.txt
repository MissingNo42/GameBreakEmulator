cmake_minimum_required(VERSION 3.26)
project(GameBreakEmulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_VERBOSE_MAKEFILE TRUE)

add_compile_options("-O1")
#add_compile_options("-Os") #size
#add_compile_options("-Ofast")
#add_compile_options("-flto") # link opti
#add_compile_options("-ffast-math") # break ISO/IEEE

add_compile_options("-g3") # debug

add_compile_options("-Wno-unknown-pragmas") # for pragma region
add_compile_options("-Wno-unused-parameter")

add_compile_options("-Wall")
add_compile_options("-Wextra")
add_compile_options("-Wshadow")
add_compile_options("-pedantic")
add_compile_options("-Wformat=2")
add_compile_options("-Wformat-overflow")
add_compile_options("-Wformat-truncation")
add_compile_options("-Wundef")
add_compile_options("-fno-common")

add_compile_options("-fstack-usage")
add_compile_options("-Wstack-usage=4194304") # debug
#add_compile_options("-Wconversion")
#add_compile_options("-Wpadded")

#add_compile_options("-ffunction-sections")
#add_compile_options("-fdata-sections")
#add_compile_options("--gc-sections")
#add_compile_options("-print-gc-sections")

add_executable(GameBreakEmulator main.c src/units/mmu.c src/utils.c src/cartridge.c src/mapper.c src/units/cpu.c src/units/ppu.c src/io_ports.c src/timer.c src/units/dma.c src/gfx/sdl.c
        src/core.c
        src/units/ctrl.c
        src/units/apu.c
        src/units/bios.c)


INCLUDE(FindPkgConfig)

pkg_check_modules(SDL2 REQUIRED sdl2)

include_directories(${SDL2_INCLUDE_DIRS})

link_directories (${SDL2_LIBRARY_DIRS})

target_link_libraries (GameBreakEmulator ${SDL2_LIBRARIES})
