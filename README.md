# GameBreak Emulator

## Description

An work-in-progress Nintendo GameBoy & GameBoy Color Emulator in C language

## Feature

Support:
- both GameBoy & GameBoy Color
- external controller (WIP)
- Game Cartridge using: no mapper, MBC1, MBC2, MBC3 or MBC5
- Cross-platform: support both Windows & Linux

## Usage

For now, the path of the game ROM must be hardcoded manually in the source code, file `main.c`

## Build

Just enter the following commands in an environment having cmake, gcc and SDL2

```bash
cmake .
cmake --build .
```

## Known issue

- Zelda OS & OA crash before menu
- Pokemon GS have black screen & color palettes issues
- Pokemon Cristal: same but color appears on text popup

## Thanks

Thanks to the [GameBoy Pandocs](https://gbdev.io/pandocs), which provides incredibly useful information about the GameBoy hardware and more
