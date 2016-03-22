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

// video modes
typedef enum { MS_WINDOWED, MS_FULLSCREEN } modestate_t;

typedef struct {
	modestate_t type;
	uint16_t width;
	uint16_t height;
	uint8_t hz;
} vmode_t;

static vmode_t modelist[40] = {0};
static uint8_t modecount = 0;
// initial modes are set in corresponding InitModes function
static uint8_t curWinMode = 0;
static uint8_t curFulMode = 0;
static uint8_t curRndScale = 1;

static void VID_InitWindowedModes( void ) {
	modelist[modecount].type = MS_WINDOWED;
	modelist[modecount].width = 320;
	modelist[modecount].height = 240;
	modelist[modecount].hz = 0;
	++modecount;
	modelist[modecount].type = MS_WINDOWED;
	modelist[modecount].width = 640;
	modelist[modecount].height = 480;
	modelist[modecount].hz = 0;
	++modecount;
	modelist[modecount].type = MS_WINDOWED;
	modelist[modecount].width = 800;
	modelist[modecount].height = 600;
	modelist[modecount].hz = 0;
	++modecount;
	modelist[modecount].type = MS_WINDOWED;
	modelist[modecount].width = 1024;
	modelist[modecount].height = 768;
	modelist[modecount].hz = 0;
	++modecount;
	curWinMode = 1;
}

static void VID_InitFullscreenModes( void ) {
	// fullscreen display modes
	curFulMode = modecount; // set first fullscreen mode as default
	const int ndm = SDL_GetNumDisplayModes( 0 );
	for ( int i = ndm - 1; i >= 0; --i ) {
		SDL_DisplayMode mode;
		SDL_GetDisplayMode( 0, i, &mode );

		// only count modes that are at least 24bpp and a factor of 60hz
		if ( SDL_BITSPERPIXEL( mode.format ) < 24 ) continue;
		if ( mode.refresh_rate % 60 != 0
			/*&& mode.refresh_rate % 72 != 0*/ ) continue;

		modelist[modecount].type = MS_FULLSCREEN;
		modelist[modecount].width = mode.w;
		modelist[modecount].height = mode.h;
		modelist[modecount].hz = mode.refresh_rate;
		++modecount;
	}
}

// (re-)sets renderer's logical size and size of texture and pixel buffer
static void VID_SetRndScale( uint8_t sFactor ) {
	curRndScale = sFactor;
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

// (re)creates a renderer and sets its logical size
// as well as the rendered texture's size and its pixel buffer
static void VID_InitRenderer( void ) {
	if ( renderer ) {
		SDL_DestroyRenderer( renderer ); renderer = NULL;
	}

	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC );
	//SDL_Renderer* ren = SDL_CreateSoftwareRenderer( winsurf );
	if ( renderer == NULL ) {
		SDL_DestroyWindow( window );
		fprintf( stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); exit( EXIT_FAILURE );
	}

	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );

	// set internal render size based on a scale of RNDW/H_BASE (320x200)
	VID_SetRndScale( curRndScale );

	// clear initial frame buffer
	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
	SDL_RenderClear( renderer );
}

static void VID_SetMode( uint8_t modenum ) {
	if ( modelist[modenum].type == MS_FULLSCREEN ) {
		bFullscreen = true;
		curFulMode = modenum;
	} else {
		bFullscreen = false;
		curWinMode = modenum;
	}

	printf( "Using mode %u (%s)\n",
		(bFullscreen ? curFulMode : curWinMode), (bFullscreen ? "FULLSCREEN" : "WINDOWED") );

	// set native fullscreen
	if ( bFullscreen ) {
		SDL_DisplayMode mode;
		mode.driverdata = 0;
		mode.format = SDL_PIXELFORMAT_UNKNOWN;
		mode.w = modelist[modenum].width;
		mode.h = modelist[modenum].height;
		mode.refresh_rate = modelist[modenum].hz;
		//SDL_SetWindowSize( window, mode.w, mode.h );
		SDL_SetWindowDisplayMode( window, &mode );
		SDL_SetWindowFullscreen( window, SDL_WINDOW_FULLSCREEN );
	} else {
		SDL_SetWindowFullscreen( window, 0 );
		//SDL_SetWindowDisplayMode( window, NULL );
		SDL_SetWindowSize( window, modelist[modenum].width, modelist[modenum].height );
		SDL_SetWindowPosition( window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
	}

	// HACK: (re)create the renderer, texture, and pixel buffer
	VID_InitRenderer();
}

int32_t MainWndProc( SDL_Window* wnd, SDL_Event* evt ) {
	switch ( evt->type ) {
	case SDL_QUIT:
		Sys_Shutdown();
		break;
	case SDL_KEYDOWN:
		switch ( evt->key.keysym.sym ) {
		// change window mode
		case SDLK_0:
			VID_SetMode( 0 );
			break;
		case SDLK_1:
			VID_SetMode( 1 );
			break;
		case SDLK_2:
			VID_SetMode( 2 );
			break;
		case SDLK_3:
			VID_SetMode( 3 );
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
		// toggle fullscreen mode
		if ( evt->key.keysym.sym == SDLK_F4 ) {
			if ( !bFullscreen ) VID_SetMode( curFulMode );
			else VID_SetMode( curWinMode );
		}
		break;
	default: break;
	}

	(void)wnd;
	return 0;
}

void VID_Init( void ) {
	// SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
		fprintf( stderr, "SDL_Init error: %s\n", SDL_GetError() );
		return EXIT_FAILURE;
	}

	VID_InitWindowedModes();
	VID_InitFullscreenModes();

	for ( vmode_t* vm = modelist; vm->width != 0; ++vm ) {
		static int vmc = 0;
		printf( "(%u) %ux%u (%uhz) %s\n", vmc, vm->width, vm->height,
			vm->hz, vm->type == MS_WINDOWED ? "Windowed" : "Fullscreen" );
		++vmc;
	}

	// create minimum window. size and position are set with modes
	window = SDL_CreateWindow( "HMQ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		RNDW_BASE, RNDH_BASE, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );

	if ( window == NULL ) {
		fprintf( stderr, "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); exit( EXIT_FAILURE );
	}

	VID_SetMode( curWinMode ); // set the window size and position
	// called at end of every SetMode
	//VID_InitRenderer(); 

	//winsurf = SDL_GetWindowSurface( window );

	// load palette from palette.lmp
	uint8_t* palData = COM_FindFile( "gfx/palette.lmp", NULL );

	uint8_t* palp = palData;
	for ( int i = 0; i < 256; ++i ) {
		palette[i] = (255 << 24) |
			(palp[0] << 16) | (palp[1] << 8) | (palp[2]);
		palp += 3;
	}
	free( palData ); palData = NULL;
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
