#include "quakedef.h"

extern const uint16_t RNDW, RNDH;
//extern bool bUsing8bpp; // read-only
// raw 32b pixel buffer from vid_main
extern uint32_t* pixbuf;
extern uint32_t palette[256];

#define MAX_PACKED_FILES 2048
#pragma pack( push, 1 )
typedef struct {
	char magic[4];
	uint32_t diroffs;
	uint32_t dirsize;
} dpackheader_t;

typedef struct {
	char name[56];
	uint32_t offset;
	uint32_t size;
} dpackfile_t;

typedef struct {
	char name[56];
	uint32_t offset;
	uint32_t size;
} packfile_t;

typedef struct {
	char name[128];
	int32_t handle;
	uint32_t numFiles;
	packfile_t* pakFiles;
} pack_t;

typedef struct searchpath_s {
	pack_t* pack;
	struct searchpath_s* next;
} searchpath_t;
#pragma pack( pop )

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

// pak
pack_t* COM_LoadPackFile( const char* path ) {
	int32_t pakSz;
	int32_t pakHnd = Sys_FileOpenRead( path, &pakSz );
	//printf( "%d - %d\n", pakHnd, pakSz );

	if ( pakHnd >= 0 ) {
		dpackheader_t pakHead;
		Sys_FileRead( pakHnd, &pakHead, sizeof (dpackheader_t) );

		if ( pakHead.magic[0] != 'P' || pakHead.magic[1] != 'A'
			|| pakHead.magic[2] != 'C' || pakHead.magic[3] != 'K' ) {
				printf( "Not a valid pak file.\n" );
				Sys_FileClose( pakHnd );
				return NULL;
		}

		uint32_t numPakFiles = pakHead.dirsize / sizeof (dpackfile_t);

		//printf( "%u, %u, %u\n", numPakFiles, pakHed.diroffs, pakHed.dirsize );

		Sys_FileSeek( pakHnd, pakHead.diroffs );
		dpackfile_t tmpPakFiles[MAX_PACKED_FILES];
		Sys_FileRead( pakHnd, tmpPakFiles, pakHead.dirsize );

		packfile_t* pakFiles = (packfile_t*)malloc( numPakFiles * sizeof (packfile_t) );

		for ( uint32_t i = 0; i < numPakFiles; ++i ) {
			Q_strncpy( pakFiles[i].name, tmpPakFiles[i].name, sizeof (tmpPakFiles[0].name) );
			pakFiles[i].offset = tmpPakFiles[i].offset;
			pakFiles[i].size = tmpPakFiles[i].size;
		}

		pack_t* pak = (pack_t*)malloc( sizeof (pack_t) );
		Q_strncpy( pak->name, path, 64 );
		pak->handle = pakHnd;
		pak->numFiles = numPakFiles;
		pak->pakFiles = pakFiles;

		return pak;

		//free( pakFiles ); pakFiles = NULL;
		//Sys_FileClose( pakHnd );
	}

	return NULL;
}

static searchpath_t* com_searchpaths = NULL;

void COM_AddGameDirectory( const char* dir ) {
	char buf[128];
	pack_t* pack;

	for ( uint8_t i = 0; ; ++i ) {
		snprintf( buf, 128, "%s/PAK%u.PAK", dir, i );
		pack = COM_LoadPackFile( buf );
		if ( pack == NULL ) break;
		searchpath_t* newpath = (searchpath_t*)malloc( sizeof (searchpath_t) );
		newpath->pack = pack;
		newpath->next = com_searchpaths;
		com_searchpaths = newpath;
		printf( "Added pack file \"%s\" to search path with %u entries.\n",
			pack->name, pack->numFiles );
	}
}

uint8_t* COM_FindFile( const char* name, int32_t* size ) {
	if ( !name ) return NULL;

	searchpath_t* searchp;
	for ( searchp = com_searchpaths; searchp != NULL; searchp = searchp->next ) {
		pack_t* cpack = searchp->pack;
		for ( uint16_t i = 0; i < cpack->numFiles; ++i ) {
			packfile_t cpfile = cpack->pakFiles[i];
			if ( Q_strcmp( name, cpfile.name ) == 0 ) {
				if ( size ) *size = cpfile.size;
				Sys_FileSeek( cpack->handle, cpfile.offset );
				uint8_t* rdat = (uint8_t*)malloc( cpfile.size );
				Sys_FileRead( cpack->handle, rdat, cpfile.size );
				return rdat;
			}
		}
	}

	printf( "File \"%s\" not found in search paths.\n", name );
	return NULL;
}

void Host_Init( void ) {
	VID_Init();

	COM_AddGameDirectory( "id1" );

	int32_t wadSize = 0;
	uint8_t* wadData = COM_FindFile( "gfx.wad", &wadSize );
	printf( "%d\n", wadSize );
	free( wadData ); wadData = NULL;

	// free ALL the things
	searchpath_t* sp;
	for ( sp = com_searchpaths; sp != NULL; ) {
		searchpath_t* next = sp->next;
		free( sp->pack->pakFiles ); sp->pack->pakFiles = NULL;
		Sys_FileClose( sp->pack->handle );
		free( sp->pack ); sp->pack = NULL;
		free( sp ); sp = NULL;
		sp = next;
	}
	free( com_searchpaths ); com_searchpaths = NULL;

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
/*void DrawPic8( uint32_t x, uint32_t y, pic_t pic, uint8_t* buffer ) {
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
}*/

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

/*void DrawRect8( uint32_t x, uint32_t y, uint16_t w, uint16_t h,
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
}*/

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

	// free pic data
	free( qpic.data ); qpic.data = NULL;
}
