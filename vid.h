#ifndef _VID_H
#define _VID_H

typedef struct {
	// the actual render size used based on a scale factor of base RNDW/H
	// updated in SetRndScale
	uint16_t RNDW;
	uint16_t RNDH;

	// raw 32b pixel buffer
	uint32_t* pixbuf;

	// 256 32bit colors array
	uint32_t palette[256];
} viddef_t;

extern viddef_t vid;

void VID_Init( void );
void VID_Update( void );
void VID_Shutdown( void );

#endif // _VID_H
