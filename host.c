#include "quakedef.h"

extern const uint16_t RNDW, RNDH;
//extern bool bUsing8bpp; // read-only
// raw 32b pixel buffer from vid_main
extern uint32_t* pixbuf;
extern uint32_t palette[256];

// pics
typedef struct {
	uint32_t w, h;
	uint8_t* data;
} pic_t;

pic_t qpic;

// benchmarking stuff
float Sys_FloatTime( void );

// timestep info
float realTime = 0, frameTime = 0;

void Host_Init( void ) {
	COM_InitFiles();
	VID_Init();

	int32_t wadSize = 0;
	uint8_t* wadData = COM_FindFile( "gfx.wad", &wadSize );
	printf( "%d\n", wadSize );
	free( wadData ); wadData = NULL;

	// pic loading
	int32_t picFile = Sys_FileOpenRead( "gfx/sbar.lmp", NULL );
	if ( picFile >= 0 ) {
		Sys_FileRead( picFile, &qpic.w, 4 );
		Sys_FileRead( picFile, &qpic.h, 4 );
		qpic.data = (uint8_t*)malloc( qpic.w * qpic.h );
		Sys_FileRead( picFile, qpic.data, qpic.w * qpic.h );
		Sys_FileClose( picFile );
	}
}

// =======
// Drawing
// =======

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

	// ====
	// enough frame time has passed for another frame update

	// Poll events
	Sys_SendKeyEvents();

	/*static float ftt = 0;
	ftt += frameTime;
	if ( ftt > 0.5f ) {
		printf("%.7f\n", frameTime);
		ftt = 0;
	}*/

	DrawRect( 160, 100, 160, 100, 255, 127, 0, pixbuf );
	DrawPic32( 0, RNDH - qpic.h, qpic, pixbuf );

	// frame is ready to be finalized in VID
	VID_Update();

	return true;
}

void Host_Shutdown( void ) {
	// shutdown video
	VID_Shutdown();

	// shutdown filesystem
	COM_ShutdownFiles();

	// free pic data
	free( qpic.data ); qpic.data = NULL;
}
