#include "quakedef.h"

uint8_t com_argc = 1; // com_argv0 will always be empty
const char* com_argv[MAX_NUM_ARGVS + 1] = {NULL};

uint8_t COM_CheckParm( const char* parm ) {
	for ( uint8_t i = 1; i < com_argc; ++i ) {
		if ( Q_strcmp( com_argv[i], parm ) == 0 ) {
			return i;
		}
	}

	return 0;
}

const char* va( const char* format, ... ) {
	va_list vl;
	va_start( vl, format );

	static char buf[2][10240];
	static uint8_t bidx = 1;
	bidx = (bidx + 1) % 2;

	vsnprintf( buf[bidx], sizeof (buf[bidx]), format, vl );

	va_end( vl );

	return buf[bidx];
}

#define BASEGAME "id1"
#define MAX_OS_PATH 128

// PAK files
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
	char name[MAX_OS_PATH];
	int32_t handle;
	uint32_t numFiles;
	packfile_t* pakFiles;
} pack_t;
#pragma pack( pop )

typedef struct searchpath_s {
	pack_t* pack;
	struct searchpath_s* next;
} searchpath_t;

static searchpath_t* com_searchpaths = NULL;

// pak
static pack_t* COM_LoadPackFile( const char* path ) {
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
		Q_strncpy( pak->name, path, MAX_OS_PATH );
		pak->handle = pakHnd;
		pak->numFiles = numPakFiles;
		pak->pakFiles = pakFiles;

		return pak;

		//free( pakFiles ); pakFiles = NULL;
		//Sys_FileClose( pakHnd );
	}

	return NULL;
}

static void COM_AddGameDirectory( const char* dir ) {
	char buf[MAX_OS_PATH];
	pack_t* pack;

	for ( uint8_t i = 0; ; ++i ) {
		snprintf( buf, MAX_OS_PATH, "%s/PAK%u.PAK", dir, i );
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

	// search bare path first before PAK files
	int32_t sz;
	char rawpath[MAX_OS_PATH];
	snprintf( rawpath, MAX_OS_PATH, "%s/%s", BASEGAME, name );
	int32_t fhnd = Sys_FileOpenRead( rawpath, &sz );
	if ( fhnd >= 0 ) {
		if ( size ) *size = sz;
		uint8_t* rdat = (uint8_t*)malloc( sz );
		Sys_FileRead( fhnd, rdat, sz );
		Sys_FileClose( fhnd );
		return rdat;
	} else {
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
	}

	printf( "File \"%s\" not found in search paths.\n", name );
	return NULL;
}

void COM_InitFiles( void ) {
	COM_AddGameDirectory( BASEGAME );
}

void COM_ShutdownFiles( void ) {
	// free ALL the things
	while ( com_searchpaths ) {
		searchpath_t* sp = com_searchpaths;
		com_searchpaths = sp->next;
		free( sp->pack->pakFiles ); sp->pack->pakFiles = NULL;
		Sys_FileClose( sp->pack->handle );
		free( sp->pack ); sp->pack = NULL;
		free( sp ); sp = NULL;
	}
}
