#include "quakedef.h"

extern const uint16_t RNDW, RNDH;
//extern SDL_Renderer* renderer;
//extern SDL_Texture* wintex;
//extern SDL_Window* window;
//extern SDL_Surface* winsurf;
extern bool bUsing8bpp; // read-only

// raw 32b pixel buffer
uint32_t* pixbuf = NULL;
// the pixbuf8 pointer is only valid when using 8bpp (bUsing8bpp)
uint8_t* pixbuf8 = NULL; // 8bpp index buffer

uint32_t palette[256] = {0}; // 256 32bit colors array

// pics
typedef struct {
	uint32_t w, h;
	uint8_t* data;
} pic_t;

pic_t qpic;

float Sys_FloatTime( void );

// timestep info
float realTime = 0, frameTime = 0;

void Host_Init( void ) {
	//pixbuf = (Uint32*)(winsurf->pixels);
	pixbuf = (uint32_t*)malloc( RNDW * RNDH * sizeof (pixbuf[0]) );

	if ( bUsing8bpp ) {
		pixbuf8 = (uint8_t*)malloc( RNDW * RNDH * sizeof (pixbuf8[0]) );
	}

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

	for ( int i = 0; i < RNDW * RNDH; ++i ) {
		if ( bUsing8bpp )
			pixbuf8[i] = 8;
		else
			pixbuf[i] = 0x202020;
	}

	// pic loading
	FILE* picFile = fopen( "gfx/sbar.lmp", "rb" );
	fread( &qpic.w, 4, 1, picFile );
	fread( &qpic.h, 4, 1, picFile );
	qpic.data = (uint8_t*)malloc( qpic.w * qpic.h );
	fread( qpic.data, 1, qpic.w * qpic.h, picFile );
	fclose( picFile );
}

// =======
// Drawing
// =======
void DrawPic8( uint32_t x, uint32_t y, pic_t pic, uint8_t* buffer ) {
	uint32_t pw = pic.w, ph = pic.h;

	// fix any render view overflow
	if ( x >= RNDW )
		x %= RNDW;
	if ( y >= RNDH )
		y %= RNDH;
	if ( (x + pw) > RNDW )
		pw = RNDW - x;
	if ( (y + ph) > RNDH )
		ph = RNDH - y;

	buffer += (RNDW * y + x);
	uint8_t* src = pic.data;

	for ( uint16_t height = 0; height < ph; ++height ) {
		for ( uint16_t width = 0; width < pw; ++width ) {
			*buffer = *src;
			++src; ++buffer;
		}
		buffer += RNDW - pw;
		if ( pw < pic.w ) src += pic.w - pw;
	}
}

void DrawPic32( uint32_t x, uint32_t y, pic_t pic, uint32_t* buffer ) {
	uint32_t pw = pic.w, ph = pic.h;

	// fix any render view overflow
	if ( x >= RNDW )
		x %= RNDW;
	if ( y >= RNDH )
		y %= RNDH;
	if ( (x + pw) > RNDW )
		pw = RNDW - x;
	if ( (y + ph) > RNDH )
		ph = RNDH - y;

	buffer += (RNDW * y + x);
	uint8_t* src = pic.data;

	for ( uint16_t height = 0; height < ph; ++height ) {
		for ( uint16_t width = 0; width < pw; ++width ) {
			*buffer = palette[*src];
			++src; ++buffer;
		}
		buffer += RNDW - pw;
		if ( pw < pic.w ) src += pic.w - pw;
	}
}

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

	if ( bUsing8bpp ) {
		DrawRect8( RNDW/2-16, RNDH/2-16, 32, 32, 127, pixbuf8 );
		DrawPic8( (RNDW - qpic.w) / 2, RNDH - qpic.h, qpic, pixbuf8 );
	} else {
		DrawRect( 160, 200, 100, 200, 255, 127, 0, pixbuf );
		DrawPic32( 10, 400, qpic, pixbuf );
	}

	// if using 8bpp, do this to update the actual 32b pixel
	// buffer from the dummy indexed pixel buffer and palette structure
	if ( bUsing8bpp )
		for ( int i = 0; i < RNDW * RNDH; ++i )
			pixbuf[i] = palette[pixbuf8[i]];

	return true;
}

void Host_Shutdown( void ) {
	// free pic data
	free( qpic.data ); qpic.data = NULL;

	if ( bUsing8bpp ) {
		free( pixbuf8 ); pixbuf8 = NULL;
	}
	free( pixbuf ); pixbuf = NULL;
}
