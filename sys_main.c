#include "quakedef.h"

#include <SDL2/SDL.h>

// globals
// vid_main
extern SDL_Window* window;

// treat screen buffer as indexed 8bpp?
// enabled by default, command line -32bpp disables
// should be set before Host_Init and remain read only from then
//bool bUsing8bpp = true;

static bool bRunning = true;

// Internal timer data
static double secsPerCount = 0;
static Uint64 startime = 0, curtime = 0;

// Public timer functions
float Sys_InitFloatTime( void ) {
	// initialize timer info
	secsPerCount = 1.0 / SDL_GetPerformanceFrequency();
	startime = SDL_GetPerformanceCounter();

	return 0;
}

float Sys_FloatTime( void ) {
	curtime = SDL_GetPerformanceCounter();

	return (curtime - startime) * secsPerCount;
}

// file api
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

// MainWndProc from vid_main
void MainWndProc( SDL_Window* wnd, SDL_Event* evt );

// poll events
void Sys_SendKeyEvents( void ) {
	SDL_Event evt;
	while ( SDL_PollEvent( &evt ) ) {
		MainWndProc( window, &evt );
	}
}

// Shutdown and stop main loop
void Sys_Shutdown( void ) {
	bRunning = false;
}

#define AR_LEN( ar ) (sizeof(ar) / sizeof(ar[0]))

int main( int argc, const char** argv ) {
	// do you even parse bro
	com_argv[0] = ""; // largc - 1 will be how many actual args
	for ( uint8_t i = 1; i < argc && i <= MAX_NUM_ARGVS; ++i ) {
		com_argv[i] = argv[i];
		++com_argc;
	}

	/*uint8_t pf = COM_CheckParm( "-32bpp" );
	if ( pf != 0 ) {
		printf( "8bpp disabled. Using 32bpp.\n" );
		//bUsing8bpp = false;
	} else
		printf( "Using 8bpp.\n" );*/

	srand( time( NULL ) );

	Host_Init();
	float startTime = Sys_InitFloatTime();

	// main loop
	while ( bRunning ) {

		// calc frame time and step through a frame
		// timestep filtering is done in host
		float currentTime = Sys_FloatTime();

		// check if enough frame time has passed and process another frame
		Host_Frame( currentTime - startTime );

		startTime = currentTime;
	}

	// Host shutdown
	Host_Shutdown();

	// SDL cleanup
	SDL_Quit();

	return EXIT_SUCCESS;
}
