#include "quakedef.h"

#include <SDL2/SDL.h>

// globals
SDL_Window* window = NULL;
// the base 1x render scale size
static const Uint16 RNDW_BASE = 320;
static const Uint16 RNDH_BASE = 200;
// the actual render size used in a scale factor of base above
// updated in SetRndScale
Uint16 RNDW = 0;
Uint16 RNDH = 0;
uint32_t* pixbuf = NULL; // raw 32b pixel buffer
// the pixbuf8 pointer is only valid when using 8bpp (bUsing8bpp)
//uint8_t* pixbuf8 = NULL; // 8bpp index buffer
//SDL_Surface* winsurf = NULL;
uint32_t palette[256] = {0}; // 256 32bit colors array

static bool bFullscreen = false;

static SDL_Renderer* renderer = NULL;
static SDL_Texture* rndtex = NULL;

static void VID_SetMode( uint16_t width, uint16_t height ) {
	SDL_SetWindowSize( window, width, height );
	SDL_SetWindowPosition( window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
}

static void VID_SetRndScale( uint8_t sFactor ) {
	// set actual render size based on scale factor of base size
	RNDW = RNDW_BASE * sFactor;
	RNDH = RNDH_BASE * sFactor;
	// RNDW/H_BASE will be 16:10, RNDH * 1.2 to stretch to its corresponding 4:3 view
	SDL_RenderSetLogicalSize( renderer, RNDW, RNDH * 1.2f );

	if ( rndtex ) SDL_DestroyTexture( rndtex );
	rndtex = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, RNDW, RNDH );

	/*if ( bUsing8bpp ) {
		pixbuf8 = (uint8_t*)malloc( RNDW * RNDH * sizeof (pixbuf8[0]) );
	}*/
	// allocate memory for pixel buffer
	if ( pixbuf ) { free( pixbuf ); pixbuf = NULL; }
	pixbuf = (uint32_t*)malloc( RNDW * RNDH * sizeof (pixbuf[0]) );

	// clear initial pixel buffer state
	for ( int i = 0; i < RNDW * RNDH; ++i ) {
		/*if ( bUsing8bpp )
			pixbuf8[i] = 8;
		else*/
		pixbuf[i] = 0x202020;
	}
}

int32_t MainWndProc( SDL_Window* wnd, SDL_Event* evt ) {
	switch ( evt->type ) {
	case SDL_QUIT:
		Sys_Shutdown();
		break;
	case SDL_KEYDOWN:
		switch ( evt->key.keysym.sym ) {
		// change window size (1-4x)
		case SDLK_0:
			VID_SetMode( 320, 240 );
			break;
		case SDLK_1:
			VID_SetMode( 640, 480 );
			break;
		case SDLK_2:
			VID_SetMode( 960, 720 );
			break;
		case SDLK_3:
			VID_SetMode( 1280, 960 );
			break;
		// change render size scale (1-4x)
		case SDLK_KP_1:
			VID_SetRndScale( 1 );
			break;
		case SDLK_KP_2:
			VID_SetRndScale( 2 );
			break;
		case SDLK_KP_3:
			VID_SetRndScale( 3 );
			break;
		case SDLK_KP_4:
			VID_SetRndScale( 4 );
			break;
		}
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

void VID_Init( void ) {
	// SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
		fprintf( stderr, "SDL_Init error: %s\n", SDL_GetError() );
		return EXIT_FAILURE;
	}

	// create minimum window. size and position are set afterwards
	window = SDL_CreateWindow( "HMQ", 0, 0, 0, 0,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
		((bFullscreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) );

	if ( window == NULL ) {
		fprintf( stderr, "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	VID_SetMode( 640, 480 ); // set the window size and position

	//winsurf = SDL_GetWindowSurface( window );
	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC );
	//SDL_Renderer* ren = SDL_CreateSoftwareRenderer( winsurf );
	if ( renderer == NULL ) {
		SDL_DestroyWindow( window );
		fprintf( stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "nearest" );
	// set internal render size based on a scale of RNDW/H_BASE (320x200)
	VID_SetRndScale( 1 );

	// load palette from palette.lmp
	FILE* palFile = fopen( "gfx/palette.lmp", "rb" );
	uint8_t* palData = (uint8_t*)malloc( 256 * 3 );
	fread( palData, 3, 256, palFile );
	fclose( palFile );

	uint8_t* palp = palData;
	for ( int i = 0; i < 256; ++i ) {
		palette[i] = (255 << 24) |
			(palp[0] << 16) | (palp[1] << 8) | (palp[2]);
		palp += 3;
	}
	free( palData ); palData = NULL;

	// clear initial frame buffer
	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
	SDL_RenderClear( renderer );
}

// frame is ready to be finalized
void VID_Update( void ) {
	// if using 8bpp, do this to update the actual 32b pixel
	// buffer from the dummy indexed pixel buffer and palette structure
	/*if ( bUsing8bpp )
		for ( int i = 0; i < RNDW * RNDH; ++i )
			pixbuf[i] = palette[pixbuf8[i]];*/

	//SDL_UpdateWindowSurface( window );
	SDL_UpdateTexture( rndtex, NULL, pixbuf, RNDW * sizeof (pixbuf[0]) );
	SDL_RenderClear( renderer );
	SDL_RenderCopy( renderer, rndtex, NULL, NULL );
	SDL_RenderPresent( renderer );
}

void VID_Shutdown( void ) {
	/*if ( bUsing8bpp ) {
		free( pixbuf8 ); pixbuf8 = NULL;
	}*/
	free( pixbuf ); pixbuf = NULL;
	SDL_DestroyTexture( rndtex );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
}
