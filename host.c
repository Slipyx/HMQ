#include "quakedef.h"

extern const uint16_t RNDW, RNDH;
//extern SDL_Renderer* renderer;
//extern SDL_Texture* wintex;
//extern SDL_Window* window;
//extern SDL_Surface* winsurf;
extern bool bUsing8bpp; // read-only

// raw 32b pixel buffer
uint32_t* pixbuf = NULL;

// the two pointers here are only valid when using 8bpp (bUsing8bpp)
uint8_t* pixbuf8 = NULL; // 8bpp index buffer
uint32_t* palette = NULL; // 256 32bit colors array

float Sys_FloatTime( void );

// timestep info
float realTime = 0, frameTime = 0;

void Host_Init( void ) {
	//pixbuf = (Uint32*)(winsurf->pixels);
	pixbuf = (uint32_t*)malloc( RNDW * RNDH * sizeof (pixbuf[0]) );

	if ( bUsing8bpp ) {
		pixbuf8 = (uint8_t*)malloc( RNDW * RNDH * sizeof (pixbuf8[0]) );

		// generate random palette
		palette = (uint32_t*)malloc( 256 * sizeof (palette[0]) );
		palette[0] = 0;
		for ( int i = 1; i < 256; ++i ) {
			palette[i] = (255 << 24) |
				((rand() % 256) << 16) | ((rand() % 256) << 8) | (rand() % 256);
		}
	}

	uint8_t bgpalidx = rand() % 256; // for 8bpp

	for ( int i = 0; i < RNDW * RNDH; ++i ) {
		if ( bUsing8bpp )
			pixbuf8[i] = bgpalidx;
		else
			pixbuf[i] = 0x202020;
	}
}

// =======
// Drawing
// =======
void DrawRect( uint32_t x, uint32_t y, uint16_t w, uint16_t h,
	uint8_t rv, uint8_t gv, uint8_t bv, uint32_t* buffer ) {

	// fix any render view overflow
	if ( x >= RNDW )
		x %= RNDW;
	if ( y >= RNDH )
		y %= RNDH;
	if ( (x + w) > RNDW )
		w = RNDW - x;
	if ( (y + h) > RNDH )
		h = RNDH - y;

	// build 32b color int from passed component values
	uint32_t col = ((255 << 24) | (rv << 16) | (gv << 8) | (bv));

	// move to first top left pixel
	buffer += (RNDW * y + x);

	// loop through width and height and fill pixels
	for ( uint16_t height = 0; height < h; ++height ) {
		for ( uint16_t width = 0; width < w; ++width ) {
			*buffer = col;
			++buffer;
		}
		buffer += RNDW - w;
	}
}

void DrawRect8( uint32_t x, uint32_t y, uint16_t w, uint16_t h,
	uint8_t col, uint8_t* buffer ) {

	// fix any render view overflow
	if ( x >= RNDW )
		x %= RNDW;
	if ( y >= RNDH )
		y %= RNDH;
	if ( (x + w) > RNDW )
		w = RNDW - x;
	if ( (y + h) > RNDH )
		h = RNDH - y;

	// move to first top left pixel
	buffer += (RNDW * y + x);

	// loop through width and height and fill pixels
	for ( uint16_t height = 0; height < h; ++height ) {
		for ( uint16_t width = 0; width < w; ++width ) {
			*buffer = col;
			++buffer;
		}
		buffer += RNDW - w;
	}
}

// takes in a raw delta time and proceeds if enough
// has accumulated for another fixed timestep
// returns true if a frame has processed and needs finalized
bool Host_Frame( float _t ) {
	static float ort = 0;
	realTime += _t;

	/*if ( realTime - ort < 1.0f / 72 )
		return false;*/

	frameTime = realTime - ort;
	ort = realTime;
	// cap frameTime so we dont get big time steps
	if ( frameTime > 0.1f ) frameTime = 0.1f;
	// enough frame time has passed for another frame update

	// do it
	static float ftt = 0;
	ftt += frameTime;
	if ( ftt > 0.5f ) {
		printf("%.7f\n", frameTime);
		ftt = 0;
	}

	// clear buffer
	/*for ( int i = 0; i < RNDW * RNDH; ++i )
		*(pixbuf + i) = 0x202020;*/

	// when using 8bpp only modify 8b idx buffer
	// if not, can modify the 32b pixel buffer directly
	/*if ( bUsing8bpp )
		pixbuf8[RNDW * 100 + 200] = 0x7f;
	else
		pixbuf[RNDW * 100 + 200] = 0xFF00FF00;*/

	if ( bUsing8bpp )
		DrawRect8( 160-16, 120-16, 32, 32, 1, pixbuf8 );
	else
		DrawRect( 10, 10, 100, 200, 255, 127, 0, pixbuf );

	// if using 8bpp, do this to update the actual 32b pixel
	// buffer from the dummy indexed pixel buffer and palette structure
	if ( bUsing8bpp )
		for ( int i = 0; i < RNDW * RNDH; ++i )
			pixbuf[i] = palette[pixbuf8[i]];

	return true;
}

void Host_Shutdown( void ) {
	if ( bUsing8bpp ) {
		free( palette ); palette = NULL;
		free( pixbuf8 ); pixbuf8 = NULL;
	}
	free( pixbuf ); pixbuf = NULL;
}
