#include "quakedef.h"

// =======
// Drawing
// =======

void DrawPic( uint32_t x, uint32_t y, pic_t pic ) {
	uint32_t pw = pic.w, ph = pic.h;

	// fix any render view overflow
	if ( x >= vid.RNDW )
		x %= vid.RNDW;
	if ( y >= vid.RNDH )
		y %= vid.RNDH;
	if ( (x + pw) > vid.RNDW )
		pw = vid.RNDW - x;
	if ( (y + ph) > vid.RNDH )
		ph = vid.RNDH - y;

	uint32_t* buffer = vid.pixbuf;
	buffer += (vid.RNDW * y + x);
	uint8_t* src = pic.data;

	for ( uint16_t height = 0; height < ph; ++height ) {
		for ( uint16_t width = 0; width < pw; ++width ) {
			if ( *src != 0xff ) // transparent
				*buffer = vid.palette[*src];
			++src; ++buffer;
		}
		buffer += vid.RNDW - pw;
		if ( pw < pic.w ) src += pic.w - pw;
	}
}

void DrawRect( uint32_t x, uint32_t y, uint16_t w, uint16_t h,
	uint8_t rv, uint8_t gv, uint8_t bv ) {

	// fix any render view overflow
	if ( x >= vid.RNDW )
		x %= vid.RNDW;
	if ( y >= vid.RNDH )
		y %= vid.RNDH;
	if ( (x + w) > vid.RNDW )
		w = vid.RNDW - x;
	if ( (y + h) > vid.RNDH )
		h = vid.RNDH - y;

	// build 32b color int from passed component values
	uint32_t col = ((255 << 24) | (rv << 16) | (gv << 8) | (bv));

	// move to first top left pixel
	uint32_t* buffer = vid.pixbuf;
	buffer += (vid.RNDW * y + x);

	// loop through width and height and fill pixels
	for ( uint16_t height = 0; height < h; ++height ) {
		for ( uint16_t width = 0; width < w; ++width ) {
			*buffer = col;
			++buffer;
		}
		buffer += vid.RNDW - w;
	}
}
