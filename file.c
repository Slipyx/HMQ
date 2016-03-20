#include "quakedef.h"

#define MAX_HANDLES 10

static FILE* fileHandles[MAX_HANDLES] = {NULL};

static int32_t FindHandle( void ) {
	for ( int i = 0; i < MAX_HANDLES; ++i )
		if ( fileHandles[i] == NULL )
			return i;

	return -1;
}

static int32_t FileLength( FILE* f ) {
	int cpos = ftell( f );
	fseek( f, 0, SEEK_END );
	int32_t endpos = ftell( f );
	fseek( f, cpos, SEEK_SET );
	return endpos;
}

int32_t Sys_FileOpenRead( const char* path, int32_t* fsize ) {
	int32_t hndix = FindHandle();
	if ( hndix < 0 ) {
		printf( "Couldn't find a free handle to open \"%s\"\n", path );
		return hndix;
	}

	fileHandles[hndix] = fopen( path, "rb" );

	if ( fileHandles[hndix] == NULL ) {
		//printf( "Couldn't open file for reading: \"%s\"\n", path );
		if ( fsize != NULL ) *fsize = -1;
		return -1;
	}

	if ( fsize != NULL )
		*fsize = FileLength( fileHandles[hndix] );

	return hndix;
}

int32_t Sys_FileOpenWrite( const char* path ) {
	int32_t hndix = FindHandle();
	if ( hndix < 0 ) {
		printf( "Couldn't find a free handle to open \"%s\"\n", path );
		return hndix;
	}

	fileHandles[hndix] = fopen( path, "wb" );

	if ( fileHandles[hndix] == NULL ) {
		//printf( "Couldn't open file for writing: \"%s\"\n", path );
		return -1;
	}

	return hndix;
}

void Sys_FileClose( int32_t hnd ) {
	if ( (fileHandles[hnd] == NULL)
		|| (hnd < 0 || hnd > MAX_HANDLES - 1) ) return;

	fclose( fileHandles[hnd] );
	fileHandles[hnd] = NULL;
}

void Sys_FileSeek( int32_t hnd, uint32_t pos ) {
	if ( (fileHandles[hnd] == NULL)
		|| (hnd < 0 || hnd > MAX_HANDLES - 1) ) return;

	fseek( fileHandles[hnd], pos, SEEK_SET );
}

uint32_t Sys_FileRead( int32_t hnd, void* dst, uint32_t cnt ) {
	if ( (fileHandles[hnd] == NULL)
		|| (hnd < 0 || hnd > MAX_HANDLES - 1)
		|| (dst == NULL) ) return 0;

	uint32_t rcnt = fread( dst, 1, cnt, fileHandles[hnd] );
	return rcnt;
}

uint32_t Sys_FileWrite( int32_t hnd, const void* src, uint32_t cnt ) {
	if ( (fileHandles[hnd] == NULL)
		|| (hnd < 0 || hnd > MAX_HANDLES - 1)
		|| (src == NULL) ) return 0;

	uint32_t wcnt = fwrite( src, 1, cnt, fileHandles[hnd] );
	return wcnt;
}
