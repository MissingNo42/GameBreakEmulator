cmake_minimum_required(VERSION 3.26)
project(GobouEmu C)

set(CMAKE_C_STANDARD 23)
add_compile_options("-Ofast")
add_executable(GobouEmu main.c src/units/mmu.c src/utils.c src/cartridge.c src/mapper.c src/mapper.h src/units/cpu.c src/units/cpu.h src/units/ppu.c src/units/ppu.h src/io_ram.c src/io_ram.h src/clock.c src/clock.h src/units/dma.c src/units/dma.h)
