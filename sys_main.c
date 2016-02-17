#include "quakedef.h"

#include <SDL2/SDL.h>

// globals
const Uint16 RNDW = 320, RNDH = 240;
//SDL_Surface* winsurf = NULL; // access to window width and height
extern uint32_t* pixbuf; // raw 32b pixel buffer made in Host

// treat screen buffer as indexed 8bpp?
// enabled by default, command line -32bpp disables
// should be set before Host_Init and remain read only from then
bool bUsing8bpp = true;

static bool bRunning = true;
static bool bFullscreen = false;

// Internal timer data
static double secsPerCount = 0;
static Uint64 startime = 0, curtime = 0;

// Public timer functions
float Sys_InitFloatTime( void ) {
	// initialize timer info
	secsPerCount = 1.0 / SDL_GetPerformanceFrequency();
	startime = SDL_GetPerformanceCounter();

	return 0;
}

float Sys_FloatTime( void ) {
	curtime = SDL_GetPerformanceCounter();

	return (curtime - startime) * secsPerCount;
}

// Shutdown and stop main loop
void Sys_Shutdown( void ) {
	bRunning = false;
}

int32_t MainWndProc( SDL_Window* wnd, SDL_Event* evt ) {
	switch ( evt->type ) {
	case SDL_QUIT:
		Sys_Shutdown();
		break;
	case SDL_KEYUP:
		// toggle fullscreen
		if ( evt->key.keysym.sym == SDLK_F4 ) {
			bFullscreen = !bFullscreen;
			SDL_SetWindowFullscreen( wnd, ((bFullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) );
		}

		break;
	default: break;
	}

	return 0;
}

#define AR_LEN( ar ) (sizeof(ar) / sizeof(ar[0]))

int main( int argc, const char** argv ) {
	// do you even parse bro
	com_argv[0] = ""; // largc - 1 will be how many actual args
	for ( uint8_t i = 1; i < argc && i <= MAX_NUM_ARGVS; ++i ) {
		com_argv[i] = argv[i];
		++com_argc;
	}

	uint8_t pf = COM_CheckParm( "-32bpp" );
	if ( pf != 0 ) {
		printf( "8bpp disabled. Using 32bpp.\n" );
		bUsing8bpp = false;
	} else
		printf( "Using 8bpp.\n" );

	srand( time( NULL ) );

	// SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
		fprintf( stderr, "SDL_Init error: %s\n", SDL_GetError() );
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow( "HMQ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		960, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
		((bFullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) );

	if ( window == NULL ) {
		fprintf( stderr, "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	//winsurf = SDL_GetWindowSurface( window );
	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC );
	//SDL_Renderer* ren = SDL_CreateSoftwareRenderer( winsurf );
	if ( renderer == NULL ) {
		SDL_DestroyWindow( window );
		fprintf( stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );
	SDL_RenderSetLogicalSize( renderer, RNDW, RNDH );

	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
	SDL_RenderClear( renderer );

	SDL_Texture* wintex = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, RNDW, RNDH );

	Host_Init();
	float startTime = Sys_InitFloatTime();

	// main loop
	SDL_Event evt;
	while ( bRunning ) {
		while ( SDL_PollEvent( &evt ) ) {
			MainWndProc( window, &evt );
		}

		// calc frame time and step through a frame
		// timestep filtering is done in host
		float currentTime = Sys_FloatTime();

		// Host_Frame returns true if a frame was processed and needs to be finalized
		if ( Host_Frame( currentTime - startTime ) ) {
			//SDL_UpdateWindowSurface( window );
			SDL_UpdateTexture( wintex, NULL, pixbuf, RNDW * sizeof (pixbuf[0]) );
			SDL_RenderClear( renderer );
			SDL_RenderCopy( renderer, wintex, NULL, NULL );
			SDL_RenderPresent( renderer );
		}

		startTime = currentTime;
	}

	// Host shutdown
	Host_Shutdown();

	// SDL cleanup
	SDL_DestroyTexture( wintex );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit();

	return EXIT_SUCCESS;
}
