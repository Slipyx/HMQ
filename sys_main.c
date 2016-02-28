#include "quakedef.h"

#include <SDL2/SDL.h>

// globals
extern SDL_Window* window;

// treat screen buffer as indexed 8bpp?
// enabled by default, command line -32bpp disables
// should be set before Host_Init and remain read only from then
//bool bUsing8bpp = true;

static bool bRunning = true;

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

// MainWndProc from vid_main
void MainWndProc( SDL_Window* wnd, SDL_Event* evt );

// poll events
void Sys_SendKeyEvents( void ) {
	SDL_Event evt;
	while ( SDL_PollEvent( &evt ) ) {
		MainWndProc( window, &evt );
	}
}

// Shutdown and stop main loop
void Sys_Shutdown( void ) {
	bRunning = false;
}

#define AR_LEN( ar ) (sizeof(ar) / sizeof(ar[0]))

int main( int argc, const char** argv ) {
	// do you even parse bro
	com_argv[0] = ""; // largc - 1 will be how many actual args
	for ( uint8_t i = 1; i < argc && i <= MAX_NUM_ARGVS; ++i ) {
		com_argv[i] = argv[i];
		++com_argc;
	}

	/*uint8_t pf = COM_CheckParm( "-32bpp" );
	if ( pf != 0 ) {
		printf( "8bpp disabled. Using 32bpp.\n" );
		//bUsing8bpp = false;
	} else
		printf( "Using 8bpp.\n" );*/

	srand( time( NULL ) );

	Host_Init();
	float startTime = Sys_InitFloatTime();

	// main loop
	while ( bRunning ) {

		// calc frame time and step through a frame
		// timestep filtering is done in host
		float currentTime = Sys_FloatTime();

		// check if enough frame time has passed and process another frame
		Host_Frame( currentTime - startTime );

		startTime = currentTime;
	}

	// Host shutdown
	Host_Shutdown();

	// SDL cleanup
	SDL_Quit();

	return EXIT_SUCCESS;
}
