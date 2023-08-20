cmake_minimum_required(VERSION 3.26)
project(GobouEmu C)

set(CMAKE_C_STANDARD 23)
add_compile_options("-O2")
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

add_executable(GobouEmu main.c src/units/mmu.c src/utils.c src/cartridge.c src/mapper.c src/mapper.h src/units/cpu.c src/units/cpu.h src/units/ppu.c src/units/ppu.h src/io_ports.c src/io_ports.h src/clock.c src/clock.h src/units/dma.c src/units/dma.h)
