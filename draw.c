#include "quakedef.h"

// =======
// Drawing
// =======

// conchars is a pic_t struct with just data and no w and h
// assumed 16 chars wide and high, each char 8x8
static uint8_t* conchars = NULL;

// offsets of source data for partialy visible pics
static uint32_t srcXofs = 0, srcYofs = 0;

// makes adjustments for clipping
// returns false if not be visible
static bool AdjustClipping( int32_t* xp, int32_t* yp, uint32_t* w, uint32_t* h ) {
	// return false if completely OOB
	if ( *xp >= vid.RNDW || *yp >= vid.RNDH )
		return false;
	if ( ((int32_t)(*xp + *w) <= 0) || ((int32_t)(*yp + *h) <= 0) ) {
		return false;
	}
	// partially OOB (positive)
	if ( (*xp + *w) > vid.RNDW )
		*w = vid.RNDW - *xp;
	if ( (*yp + *h) > vid.RNDH )
		*h = vid.RNDH - *yp;
	// partially OOB (negative)
	srcXofs = 0; srcYofs = 0;
	if ( *xp < 0 ) {
		srcXofs = -*xp;
		*xp = 0;
		*w -= srcXofs;
	}
	if ( *yp < 0 ) {
		srcYofs = -*yp;
		*yp = 0;
		*h -= srcYofs;
	}
	// should be properly clipped, so ok to draw now
	return true;
}

// draw single 8x8 char from conchars data
static void DrawChar( uint8_t idx, int32_t x, int32_t y ) {
	// size of conchars is assumed to be
	// 16 characters wide by 16 characters high
	static const uint8_t chw = 8; // size of each character

	if ( conchars == NULL ) conchars = W_GetLumpName( "CONCHARS" );

	uint32_t cw = chw, ch = chw;

	if ( AdjustClipping( &x, &y, &cw, &ch ) == false ) return;

	uint32_t* buffer = vid.pixbuf;
	buffer += (vid.RNDW * y + x);
	uint8_t row = (idx / 16);
	uint8_t* src = conchars + ((16 * chw * chw) * row + ((idx - (16 * row)) * chw));
	src += srcXofs;
	src += (chw * 16) * srcYofs;

	for ( uint32_t height = 0; height < ch; ++height ) {
		for ( uint32_t width = 0; width < cw; ++width ) {
			if ( *src != 0 ) // transparent
				*buffer = vid.palette[*src];
			++src; ++buffer;
		}
		buffer += vid.RNDW - cw;
		src += (chw * 16) - cw;
		//if ( cw < chw ) src += chw - cw;
	}
}

void DrawString( const char* str, int32_t x, int32_t y ) {
	const char* sc = str;
	int32_t cx = x;
	while ( *sc != '\0' ) {
		DrawChar( *sc, cx, y );
		cx += 8;
		++sc;
	}
}

void DrawPic( pic_t* pic, int32_t x, int32_t y ) {
	uint32_t pw = pic->w, ph = pic->h;

	if ( AdjustClipping( &x, &y, &pw, &ph ) == false ) return;

	uint32_t* buffer = vid.pixbuf;
	buffer += (vid.RNDW * y + x);
	uint8_t* src = pic->data;
	src += (pic->w * srcYofs + srcXofs);

	for ( uint32_t height = 0; height < ph; ++height ) {
		for ( uint32_t width = 0; width < pw; ++width ) {
			if ( *src != 0xff ) // transparent
				*buffer = vid.palette[*src];
			++src; ++buffer;
		}
		buffer += vid.RNDW - pw;
		if ( pw < pic->w ) src += pic->w - pw;
	}
}

void DrawRect( int32_t x, int32_t y, uint32_t w, uint32_t h,
	uint8_t rv, uint8_t gv, uint8_t bv ) {

	if ( AdjustClipping( &x, &y, &w, &h ) == false ) return;

	// build 32b color int from passed component values
	uint32_t col = ((255 << 24) | (rv << 16) | (gv << 8) | (bv));

	// move to first top left pixel
	uint32_t* buffer = vid.pixbuf;
	buffer += (vid.RNDW * y + x);

	// loop through width and height and fill pixels
	for ( uint32_t height = 0; height < h; ++height ) {
		for ( uint32_t width = 0; width < w; ++width ) {
			*buffer = col;
			++buffer;
		}
		buffer += vid.RNDW - w;
	}
}
