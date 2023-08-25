//
// Created by Romain on 21/08/2023.
//

#ifndef GOBOUEMU_SDL_H
#define GOBOUEMU_SDL_H

#include <SDL2/SDL.h>
#include "../core.h"

#define S_WIDTH 1024
#define S_HEIGHT 512

extern SDL_Window * window;
extern SDL_Renderer * renderer;
extern SDL_Texture * LCD, * DebugMemory, * DebugTileMaps, * DebugTileData;

s32 GfxSetup();
void GfxQuit();
void GfxRender_Memory();
void GfxRender_VRAM();

#endif //GOBOUEMU_SDL_H
