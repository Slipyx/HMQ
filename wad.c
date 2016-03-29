#include "quakedef.h"

uint8_t* wadData = NULL;

// load a global wad file in memory for searching with GetLump
void W_LoadFile( const char* wadName ) {
	// check for a previously loaded wad
	if ( wadData ) {
		free( wadData ); wadData = NULL;
	}

	int32_t wadSize = 0;
	wadData = COM_FindFile( wadName, &wadSize );
	if ( wadData[0] != 'W' || wadData[1] != 'A'
		|| wadData[2] != 'D' || wadData[3] != '2' ) {
		printf( "W_LoadFile Error: %s is not a valid WAD file.\n", wadName );
		free( wadData ); wadData = NULL;
	}
	if ( wadData )
		printf( "Loaded WAD file \"%s\" with size %d and %u entries.\n",
			wadName, wadSize, *(uint32_t*)&wadData[4] );
}

// search currently loaded global wad for specified pic lump name
// and return a pointer to a pic struct
pic_t* W_GetPicLump( const char* lumpName ) {
	if ( wadData ) {
		uint32_t nwf = *(uint32_t*)&wadData[4];
		uint32_t wdiroffs = *(uint32_t*)&wadData[8];
		for ( uint32_t i = 0; i < nwf; ++i ) {
			uint8_t* cwd = &wadData[wdiroffs+(i*32)];
			char fname[16];
			Q_strncpy( fname, (const char*)&cwd[16], 16 );
			//printf( "%s\n", fname );
			if ( Q_strcmp( fname, lumpName ) == 0 ) {
				// found lump in wad file, wadData[wfoffs] points to lump data
				uint32_t wfoffs = *(uint32_t*)&cwd[0];
				//uint32_t wfsz = *(uint32_t*)&cwd[4];
				return (pic_t*)&wadData[wfoffs];
			}
		}
		printf( "WAD pic lump \"%s\" not found.\n", lumpName );
		return NULL;
	}
	printf( "W_GetLump Error: No WAD file loaded.\n" );
	return NULL;
}

// free global wad data
void W_CloseFile( void ) {
	free( wadData ); wadData = NULL;
}
