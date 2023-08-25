#include <stdio.h>
#include "src/gfx/sdl.h"
//#include "src/core.h"

#define FPS 120
#define tr "../tloz.gb"
//#define tr "../testrom/blargg/cpu_instrs/individual/03-op sp,hl.gb"
//#define tr "../testrom/blargg/instr_timing/instr_timing.gb"

/*
static_assert(sizeof(Controller)    == 11,"invalid size");
static_assert(sizeof(Color)         == 1, "invalid size");
static_assert(sizeof(Coord)         == 2, "invalid size");
static_assert(sizeof(CoordRoom)     == 1, "invalid size");
static_assert(sizeof(CoordWorld)    == 1, "invalid size");
static_assert(sizeof(NESPixel)      == 1, "invalid size");
static_assert(sizeof(RGBPixel)      == 3, "invalid size");
static_assert(sizeof(GfxFlag)       == 1, "invalid size");
static_assert(sizeof(OverworldTile) == 1, "invalid size");
static_assert(sizeof(Palette)       == 4, "invalid size");
static_assert(sizeof(Region)        == 2, "invalid size");*/

//int SDL_main(int argc, char * argv[]);

int main00() {
	INFO("GobouEmulator starting", "cartridge file = %s\n", tr);
	
	open_cartridge(tr);
	
	Reset_io_ports(1);
	Reset_cpu(1);
	Reset_ppu(1);
	Reset_dma(1);
	
	Reset_cartridge(1);
	Reset_mmu(1);
	Reset_mapper(1);
	
	/*
	if (open_cartridge(tr)
	&& load_header()
	&& init_memory()) {
		INFO("Memory loaded", "@ %p\n", Memory);
		init_mapper();
		
		INFO("Load cartridge...", "\n");
		load_cartridge();
	}*/
	
	/* approx mapper1 test
	u8 t[ROM1];
	u8 r=1;
	for (u16 i = 0; i <= ROM1; i++) {
		t[i]=read(i+ROM1);
	}
	write(0x2000, 255);
	for (u16 i = 0; i <= ROM1; i++) {
		r&=t[i]==read(i+ROM1);
	}
	
	DEBUG(">> r", " = %d\n", r);
	
	for (u16 i = XRAM; i < XRAM+0x2000; i++) {
		if (0xff!=read(i)) {
			ERROR("Ram fault", "%04X %02X\n", i, read(i));
			break;
		}
	}
	
	write(0x0000, 0xAA);
	
	for (u16 i = XRAM; i <= XRAM+0x2000; i++) {
		write(i, 42);
		if (42!=read(i)) ERROR("Ram fault", "%04X %hhu\n", i, memory_read(i));
	}*/
	
	INFO("rom:", " %p\nenter:", memoryMap.rom0+ROM0);
	getchar();
	
	INFO("Gobou Start !", "\n");
	//cpu_init();
	//for (u32 i = 0; i < (4194304<<8); i++) {
	u32 pass = 0;
	for (u32 i = 0; i < (4000000 * 100); i++) {
		//if (i > 47000 && ioLY > 0x88 && ioLY < 0x92)
		//	DEBUG("\t", "%d \t A %02X (z %hhu) | C %02X | E %02X | HL %04X ( LY %hhu 0x%02X)\n", i, A, z, C, E, HL,ioLY, ioLY);
		if (!pass) {
			DEBUG("\t", "%u \t A %02X | B %02X | C %02X | D %02X | E %02X | HL %04X  [%c %c %c %c]  ( LY %hhu 0x%02X)\n", i, A, B, C, D, E, HL,
				  (z) ? 'Z': 'z', (n) ? 'N': 'n', (h) ? 'H': 'h', (c) ? 'C': 'c',
			      ioLY, ioLY);
			if (!(i & 0xf)) {
				int cu = getchar();
				if (cu != '\n') {
					printf("pass:");
					fflush(stdout);
					scanf("%u%*c", &pass);
				}
			}
		} else pass--;
		cpu_run();
		if (PC > 0x100) break;
	}
	
	char xx=0;
	for (u32 i = 0; i < 20000000; i++) {
		//if (!(i & 0xffff))
		if (i > 100) xx=1;
		//if (xx)DEBUG("\t", "%d \t A %02X (z %hhu) | C %02X | E %02X | HL %04X ( LY %hhu 0x%02X)\n", i, A, z, C, E, HL,ioLY, ioLY);
		cpu_run();
	}
	INFO("Gobou Exit !", "\n");
	
	free(Memory);
	return 0;
}









static inline u32 min(u32 a, u32 b){
	return a > b ? b: a;
}


int main(int argc, char * argv[]) {
	
	if (GfxSetup()) {
		
		emulator_start(tr);
		
		void * px;
		int pitch;
		u8 run = 1;
		u64 frametime = 0, framelimit = 1;
        u32 fps = 0;
		u16 slowdown = 0;
        u64 frame_count_time = 0;
		SDL_Event event;
		SDL_GameController * ctrl = NULL;
		SDL_JoystickID ctrlID = -2;
		SDL_KeyCode debug_key = 0;
		
		while (run) {
			
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						run = 0;
                        break;
						
		            case SDL_KEYDOWN:{
						switch (event.key.keysym.sym) {
							case SDLK_z:     { ControllerPressB(); break; }
							case SDLK_a:     { ControllerPressA(); break; }
							case SDLK_s:     { ControllerPressSelect(); break; }
							case SDLK_q:     { ControllerPressStart(); break; }
							case SDLK_LEFT:  { ControllerPressLeft(); break; }
							case SDLK_RIGHT: { ControllerPressRight(); break; }
							case SDLK_UP:    { ControllerPressUp(); break; }
							case SDLK_DOWN:  { ControllerPressDown(); break; }
							default:
								debug_key = event.key.keysym.sym;
								printf("DEBUG: Set key as %d (%c)\n", debug_key, debug_key);
						}break;
					}
					
		            case SDL_KEYUP: {
						switch (event.key.keysym.sym) {
							case SDLK_z:     { ControllerReleaseB(); break; }
							case SDLK_a:     { ControllerReleaseA(); break; }
							case SDLK_s:     { ControllerReleaseSelect(); break; }
							case SDLK_q:     { ControllerReleaseStart(); break; }
							case SDLK_LEFT:  { ControllerReleaseLeft(); break; }
							case SDLK_RIGHT: { ControllerReleaseRight(); break; }
							case SDLK_UP:    { ControllerReleaseUp(); break; }
							case SDLK_DOWN:  { ControllerReleaseDown(); break; }
						}break;
					}
					
					case SDL_CONTROLLERDEVICEADDED:
						if (SDL_IsGameController(event.cdevice.which) && !ctrl) {
							ctrl = SDL_GameControllerOpen(event.cdevice.which);
			                ctrlID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl));
							printf("new ctrl: %s\n", SDL_GameControllerNameForIndex(ctrlID));
						}break;
						
					case SDL_CONTROLLERDEVICEREMOVED:
						if (event.cdevice.which == ctrlID) {
							printf("rm ctrl %d\n", event.cdevice.which);
							SDL_GameControllerClose(ctrl);
							ctrl = NULL;
							ctrlID = -2;
						}break;
						
					case SDL_CONTROLLERBUTTONDOWN:
						printf("kd %d\n", event.cbutton.button);
						switch (event.cbutton.button) {
							case SDL_CONTROLLER_BUTTON_A: { ControllerPressA(); break; }
								case SDL_CONTROLLER_BUTTON_B: { ControllerPressB(); break; }
								case SDL_CONTROLLER_BUTTON_START: { ControllerPressStart(); break; }
								case SDL_CONTROLLER_BUTTON_BACK: { ControllerPressSelect(); break; }
								case SDL_CONTROLLER_BUTTON_DPAD_LEFT: { ControllerPressLeft(); break; }
								case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: { ControllerPressRight(); break; }
								case SDL_CONTROLLER_BUTTON_DPAD_UP: { ControllerPressUp(); break; }
								case SDL_CONTROLLER_BUTTON_DPAD_DOWN: { ControllerPressDown(); break; }
						}break;
					case SDL_CONTROLLERBUTTONUP:
						printf("ku %d\n", event.cbutton.button);
						switch (event.cbutton.button) {
							case SDL_CONTROLLER_BUTTON_A: { ControllerReleaseA(); break; }
							case SDL_CONTROLLER_BUTTON_B: { ControllerReleaseB(); break; }
							case SDL_CONTROLLER_BUTTON_START: { ControllerReleaseStart(); break; }
							case SDL_CONTROLLER_BUTTON_BACK: { ControllerReleaseSelect(); break; }
							case SDL_CONTROLLER_BUTTON_DPAD_LEFT: { ControllerReleaseLeft(); break; }
							case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: { ControllerReleaseRight(); break; }
							case SDL_CONTROLLER_BUTTON_DPAD_UP: { ControllerReleaseUp(); break; }
							case SDL_CONTROLLER_BUTTON_DPAD_DOWN: { ControllerReleaseDown(); break; }
						}break;
				}
			}
			frametime = SDL_GetTicks64();
			/*
			if (engine->ctrl.LEFT == CTRLKEY_HOLD && engine->ctrl.RIGHT == CTRLKEY_HOLD) { // DEBUG COMMAND
				switch (debug_key) {
					case SDLK_w: {
						if (slowdown) slowdown -= 20;
						else framelimit = 0;
						printf("DEBUG W >> Fastup : by %hu (unleashed : %d)\n", slowdown, !framelimit);
						break;
					}
					case SDLK_x: {
						if (!framelimit) framelimit = 1;
						else slowdown += 20;
						printf("DEBUG X >> Slowdown : by %hu\n", slowdown);
						break;
					}
					case SDLK_i: {
						engine->render->region.layers ^= LAYER_BACKGROUND_S;
						printf("DEBUG I >> Switch  by %d %d %d\n", engine->render->region.layers & 4, engine->render->region.layers & 2, engine->render->region.layers & 1);
						break;
					}
					case SDLK_o: {
						engine->render->region.layers ^= LAYER_BACKGROUND;
						printf("DEBUG I >> Switch  by %d %d %d\n", engine->render->region.layers & 4, engine->render->region.layers & 2, engine->render->region.layers & 1);
						break;
					}
					case SDLK_p: {
						engine->render->region.layers ^= LAYER_FOREGROUND_S;
						printf("DEBUG I >> Switch  by %d %d %d\n", engine->render->region.layers & 4, engine->render->region.layers & 2, engine->render->region.layers & 1);
						break;
					}
					default:
				}
				debug_key = 0;
			}*/
			
			
			SDL_LockTexture(LCD, NULL, &px, &pitch);
			
			ppu_set_screen(px);
			emulator_loop(NULL);
			//renderRGBA(px, engine->render);	//memcpy(px, s.pixels, sizeof(Screen));
			
			SDL_UnlockTexture(LCD);
			SDL_Rect rc = {0, 0, 160, 144};
			SDL_RenderCopy(renderer, LCD, NULL, &rc);
			
			GfxRender_Memory();
			GfxRender_VRAM();
			
            fps++;
            if (frametime - frame_count_time > 1000){
                printf("%.1f fps\n", fps * 1000. / (double)(frametime - frame_count_time));
                frame_count_time = frametime;
                fps = 0;
            }
			frametime = min(1000 / FPS, 1000 / FPS - SDL_GetTicks64() + frametime);
			if (framelimit) SDL_Delay(frametime);
			if (slowdown) SDL_Delay(slowdown);
			SDL_RenderPresent(renderer);
			SDL_SetRenderDrawColor(renderer, 31, 31, 31, 0);
			SDL_RenderClear(renderer);
		}
	
		GfxQuit();
	}
}
