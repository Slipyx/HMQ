#include "quakedef.h"

// benchmarking stuff
float Sys_FloatTime( void );

// timestep info
float realTime = 0, frameTime = 0;

void Host_Init( void ) {
	COM_InitFiles();
	VID_Init();

	// load wad for global persistance
	W_LoadWadFile( "gfx.wad" );
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
	// clear screen
	memset( vid.pixbuf, vid.palette[3], vid.RNDW * vid.RNDH * sizeof (vid.pixbuf[0]) );

	static uint8_t ln = 1;
	static float lnt = 0;
	lnt += frameTime;
	if ( lnt >= 0.125f ) {
		ln = rand() % 5 + 1;
		lnt = 0;
	}

	// sbar
	pic_t* sbar = (pic_t*)W_GetLumpName( "sBaR" );

	pic_t* qpic = (pic_t*)W_GetLumpName( va( "FACE%u", ln ) );
	//pic_t* qpic = (pic_t*)COM_FindFile( "gfx/sp_menu.lmp", NULL );

	static float xp = 80;
	DrawPic( sbar, (int)xp, vid.RNDH - sbar->h );
	DrawString( "Hello, World!", (int)xp+100, 80 );
	//DrawRect( xp+32, 112, 64, 32, 255, 127, 0 );
	DrawPic( qpic, (int)xp+112, 100 );
	xp -= 20 * frameTime;
	//if ( xp + sbar->w <= 0 ) xp = 160;
	//if ( xp >= vid.RNDH + 4 ) xp = 100;

	// frame is ready to be finalized in VID
	VID_Update();

	return true;
}

void Host_Shutdown( void ) {
	// shutdown video
	VID_Shutdown();

	// free global wad data
	W_CloseWadFile();

	// shutdown filesystem
	COM_ShutdownFiles();

	// free pic data
	//free( qpic.data ); qpic.data = NULL;
}
