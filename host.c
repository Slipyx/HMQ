#include "quakedef.h"

// vid_main
//extern const uint16_t RNDW, RNDH;

// benchmarking stuff
float Sys_FloatTime( void );

// timestep info
float realTime = 0, frameTime = 0;

pic_t qpic;

void Host_Init( void ) {
	COM_InitFiles();
	VID_Init();

	int32_t wadSize = 0;
	uint8_t* wadData = COM_FindFile( "gfx.wad", &wadSize );
	printf( "%d\n", wadSize );
	free( wadData ); wadData = NULL;

	// pic loading
	uint8_t* picFileDat = COM_FindFile( "gfx/menuplyr.lmp", NULL );
	if ( picFileDat ) {
		qpic.w = *(uint32_t*)&picFileDat[0];
		qpic.h = *(uint32_t*)&picFileDat[4];
		qpic.data = (uint8_t*)malloc( qpic.w * qpic.h );
		memcpy( qpic.data, &picFileDat[8], qpic.w * qpic.h );
		free( picFileDat ); picFileDat = NULL;
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
	//memset( pixbuf, 0x40, RNDW * RNDH * sizeof (pixbuf[0]) );

	DrawRect( 160, 100, 160, 100, 255, 127, 0 );
	static float xp = 0;
	xp += 10 * frameTime;
	DrawPic( (uint32_t)xp, vid.RNDH - qpic.h, qpic );

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
