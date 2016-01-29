#include "quakedef.h"

#include <SDL2/SDL.h>

static bool bRunning = true;

// Internal timer data
static double secsPerCount = 0;
static Uint64 stime = 0, ctime = 0;

// Public timer functions
float Sys_InitFloatTime( void ) {
	// initialize timer info
	secsPerCount = 1.0 / SDL_GetPerformanceFrequency();
	stime = SDL_GetPerformanceCounter();

	return stime * secsPerCount;
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

	SDL_DisplayMode dtm;
	SDL_GetDesktopDisplayMode( 0, &dtm );
	SDL_Window* win = SDL_CreateWindow( "HMQ", (dtm.w-640)/2, (dtm.h-480)/2, 640, 480, SDL_WINDOW_SHOWN );
	if ( win == NULL ) {
		fprintf( stderr, "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	SDL_Renderer* ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED /*|
		SDL_RENDERER_PRESENTVSYNC*/ );
	if ( ren == NULL ) {
		SDL_DestroyWindow( win );
		fprintf( stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}
	SDL_RenderClear( ren );

	float startTime = Sys_InitFloatTime();

	// main loop
	SDL_Event evt;
	while ( bRunning ) {
		// calc for frame time
		float currentTime = Sys_FloatTime();

		while ( SDL_PollEvent( &evt ) ) {
			MainWndProc( win, &evt );
		}

		//printf( "%.14f\n", currentTime );

		SDL_SetRenderDrawColor( ren, 32, 32, 32, 255 );
		SDL_RenderClear( ren );

		// do da drawin
		//SDL_SetRenderDrawColor( ren, 0, 255, 0, 255 );
		//SDL_RenderDrawRect( ren, &rect );

		SDL_RenderPresent( ren );
	}

	// SDL cleanup
	SDL_DestroyRenderer( ren );
	SDL_DestroyWindow( win );
	SDL_Quit();

	return EXIT_SUCCESS;
}
