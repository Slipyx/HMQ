#ifndef _DRAW_H
#define _DRAW_H

// pics
typedef struct {
	uint32_t w, h;
	uint8_t* data;
} pic_t;

void DrawPic( uint32_t x, uint32_t y, pic_t pic );
void DrawRect( uint32_t x, uint32_t y, uint16_t w, uint16_t h,
	uint8_t rv, uint8_t gv, uint8_t bv );

#endif // _DRAW_H
