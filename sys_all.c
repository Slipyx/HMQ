#include "quakedef.h"

//#include <SDL2/SDL.h>

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
	/*if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
		fprintf( stderr, "SDL_Init error: %s\n", SDL_GetError() );
		return EXIT_FAILURE;
	}

	SDL_Window* win = SDL_CreateWindow( "HMQ", 100, 100, 640, 480, SDL_WINDOW_SHOWN );
	if ( win == NULL ) {
		fprintf( stderr, "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	SDL_Renderer* ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC );
	if ( ren == NULL ) {
		SDL_DestroyWindow( win );
		fprintf( stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	// main loop
	SDL_Event evt;
	uint8_t bQuit = 0;
	while ( bQuit == 0 ) {
		while ( SDL_PollEvent( &evt ) ) {
			if ( evt.type == SDL_QUIT ) bQuit = 1;
		}

		SDL_RenderClear( ren );
		SDL_RenderPresent( ren );
	}

	// SDL cleanup
	SDL_DestroyRenderer( ren );
	SDL_DestroyWindow( win );
	SDL_Quit();*/

	return EXIT_SUCCESS;
}
