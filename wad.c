#include "quakedef.h"

// data structures
typedef struct {
	uint32_t pos;
	uint32_t dsize;
	uint32_t size;
	uint8_t type;
	uint8_t compression;
	uint16_t padding;
	char name[16]; // 0 padded
} lumpinfo_t;

typedef struct {
	char magic[4];
	uint32_t numLumps;
	uint32_t dirOffs;
} wadinfo_t;

// globally loaded wad data
static uint8_t* wadData = NULL;
static uint32_t numLumps = 0;
static lumpinfo_t* lumpinfos = NULL;

// ensure lowercase
static void W_CleanupName( const char* in, char* out ) {
	uint8_t i = 0;
	for ( i = 0; i < 16; ++i ) {
		char c = in[i];
		if ( c == 0 ) break;
		if ( c >= 'A' && c <= 'Z' )
			c |= (1 << 5);
		out[i] = c;
	}
	// pad end with zeros
	for ( ; i < 16; ++i )
		out[i] = 0;
}

// load a global wad file in memory for searching with GetLump
void W_LoadWadFile( const char* fileName ) {
	// check for a previously loaded wad
	if ( wadData ) {
		free( wadData ); wadData = NULL;
	}

	int32_t wadSize = 0;
	wadData = COM_FindFile( fileName, &wadSize );
	if ( wadData ) {
		wadinfo_t* wadHeader = (wadinfo_t*)wadData;

		if ( wadHeader->magic[0] != 'W' || wadHeader->magic[1] != 'A'
			|| wadHeader->magic[2] != 'D' || wadHeader->magic[3] != '2' ) {
			printf( "W_LoadWadFile Error: %s is not a valid WAD file.\n", fileName );
			free( wadData ); wadData = NULL;
			return;
		}

		numLumps = wadHeader->numLumps;
		lumpinfos = (lumpinfo_t*)(wadData + wadHeader->dirOffs);

		lumpinfo_t* cLump = lumpinfos;
		for ( uint32_t i = 0; i < numLumps; ++i ) {
			W_CleanupName( cLump->name, cLump->name );
			++cLump;
		}

		printf( "Loaded WAD file \"%s\" with size %d and %u entries.\n",
			fileName, wadSize, wadHeader->numLumps );
	}
}

// search wad for lumpinfo for name specified
static lumpinfo_t* W_GetLumpInfo( const char* name ) {
	char cName[16];
	W_CleanupName( name, cName );

	lumpinfo_t* cLump = lumpinfos;
	for ( uint32_t i = 0; i < numLumps; ++i ) {
		if ( Q_strcmp( cLump->name, cName ) == 0 ) {
			// found lump in wad file
			return cLump;
		}
		++cLump;
	}
	return NULL;
}

// search currently loaded global wad for specified lump name
// and return a pointer to the lump data
uint8_t* W_GetLumpName( const char* name ) {
	if ( wadData ) {
		lumpinfo_t* lump = W_GetLumpInfo( name );
		if ( lump ) return (wadData + lump->pos);
		else {
			printf( "WAD lump \"%s\" not found.\n", name );
			return NULL;
		}
	}
	printf( "W_GetLumpName Error: No WAD file loaded (%s).\n", name );
	return NULL;
}

// free global wad data
void W_CloseWadFile( void ) {
	free( wadData ); wadData = NULL;
}
