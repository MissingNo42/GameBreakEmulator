//
// Created by Romain on 21/08/2023.
//

#ifndef GBEMU_SDL_H
#define GBEMU_SDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include "../core.h"

#define S_WIDTH 2048
#define S_HEIGHT 1024

extern SDL_Window * window;
extern SDL_Renderer * renderer;
extern SDL_Texture * LCD, * DebugMemory, * DebugTileMaps, * DebugAttrMaps, *DebugColor, * DebugTileData;

s32 GfxSetup();
void GfxQuit();
void GfxRender_Memory();
void GfxRender_VRAM();

#endif //GBEMU_SDL_H
