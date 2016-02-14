#include "quakedef.h"

extern const Uint16 SCRW, SCRH;
//extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* wintex;
//extern SDL_Surface* winsurf;

// raw 32bpp pixel buffer
static Uint32* pixbuf = NULL;

// timestep info
float realTime = 0, frameTime = 0;

void Host_Init( void ) {
	//pixbuf = (Uint32*)(winsurf->pixels);
	pixbuf = (Uint32*)malloc( SCRW * SCRH * 4 );

	for ( int i = 0; i < SCRW * SCRH; ++i )
		*(pixbuf + i) = 0x202020;
}

// takes in a raw delta time and proceeds if enough
// has accumulated for another fixed timestep
void Host_Frame( float _t ) {
	static float ort = 0;
	realTime += _t;

	if ( realTime - ort < 1.0f / 72 )
		return;

	frameTime = realTime - ort;
	ort = realTime;
	// cap frameTime so we dont get big time steps
	if ( frameTime > 0.1f ) frameTime = 0.1f;
	// enough frame time has passed for another frame update

	// do it
	printf("%.7f\n", frameTime);

	// clear buffer
	/*for ( int i = 0; i < SCRW * SCRH; ++i )
		*(pixbuf + i) = 0x202020;*/

	static Uint8 rv = 255;
	static Uint8 gv = 127;
	static float xp = 0;
	xp += ((cos( realTime*8 )+1)*0.5f) * 200 * frameTime;
	static float yp = 0;
	yp += ((sin( realTime*16 )+1)*0.5f) * 200 * frameTime;

	*(pixbuf + (SCRW * ((int)yp%SCRH) + ((int)xp%SCRW))) = (rv<<16)|(gv<<8);

	//SDL_UpdateWindowSurface( window );
	SDL_UpdateTexture( wintex, NULL, pixbuf, SCRW * 4 );

	SDL_RenderClear( renderer );
	SDL_RenderCopy( renderer, wintex, NULL, NULL );
	SDL_RenderPresent( renderer );
}

void Host_Shutdown( void ) {
	free( pixbuf ); pixbuf = NULL;
}
