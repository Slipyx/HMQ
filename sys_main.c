#include "quakedef.h"

//#include <SDL2/SDL.h>

// globals
const Uint16 SCRW = 320, SCRH = 240;
//SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* wintex = NULL;
//SDL_Surface* winsurf = NULL; // access to window width and height

static bool bRunning = true;

// Internal timer data
static double secsPerCount = 0;
static Uint64 stime = 0, ctime = 0;

// Public timer functions
float Sys_InitFloatTime( void ) {
	// initialize timer info
	secsPerCount = 1.0 / SDL_GetPerformanceFrequency();
	stime = SDL_GetPerformanceCounter();

	return 0;
}

float Sys_FloatTime( void ) {
	ctime = SDL_GetPerformanceCounter();

	return (ctime - stime) * secsPerCount;
}

// Shutdown and stop main loop
void Sys_Shutdown( void ) {
	bRunning = false;
}

int32_t MainWndProc( SDL_Window* wnd, SDL_Event* evt ) {
	(void)wnd;

	switch ( evt->type ) {
	case SDL_QUIT:
		Sys_Shutdown();
		break;
	case SDL_KEYUP:
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

	uint8_t pf = COM_CheckParm( "-fps" );
	if ( pf != 0 ) {
		if ( com_argv[pf + 1] )
			printf( "fps set at %d\n", Q_atoi( com_argv[pf + 1] ) );
		else
			printf( "no fps value given\n" );
	}

	// SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
		fprintf( stderr, "SDL_Init error: %s\n", SDL_GetError() );
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow( "HMQ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
	if ( window == NULL ) {
		fprintf( stderr, "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	//winsurf = SDL_GetWindowSurface( window );
	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED
		/*| SDL_RENDERER_PRESENTVSYNC*/ );
	//SDL_Renderer* ren = SDL_CreateSoftwareRenderer( winsurf );
	if ( renderer == NULL ) {
		SDL_DestroyWindow( window );
		fprintf( stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );
	SDL_RenderSetLogicalSize( renderer, SCRW, SCRH );

	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
	SDL_RenderClear( renderer );

	wintex = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		SCRW, SCRH );

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
		Host_Frame( currentTime - startTime );
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
