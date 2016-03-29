#ifndef _DRAW_H
#define _DRAW_H

// pics
typedef struct {
	uint32_t w, h;
	// first four bytes of data
	// actual size is w * h bytes
	uint8_t data[4];
} pic_t;

void DrawPic( pic_t* pic, int32_t x, int32_t y );
void DrawRect( int32_t x, int32_t y, uint32_t w, uint32_t h,
	uint8_t rv, uint8_t gv, uint8_t bv );
void DrawString( const char* str, int32_t x, int32_t y );

#endif // _DRAW_H
