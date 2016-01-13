#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAX_NUM_ARGVS 50

uint8_t largc = 1; // largv0 will always be empty
const char* largv[MAX_NUM_ARGVS + 1] = {NULL};



uint8_t COM_CheckParm( const char* parm ) {
	for ( uint8_t i = 1; i < largc; ++i ) {
		if ( strcmp( largv[i], parm ) == 0 ) {
			return i;
		}
	}

	return 0;
}

int main( int argc, const char** argv ) {
	largv[0] = ""; // largc - 1 will be how many actual args
	for ( uint8_t i = 1; i < argc && i < MAX_NUM_ARGVS; ++i ) {
		largv[i] = argv[i];
		largc++;
	}

	uint8_t pf = COM_CheckParm( "-fps" );
	if ( pf != 0 ) {
		if ( largv[pf + 1] )
			printf( "fps set at %d\n", atoi( largv[pf + 1] ) );
		else
			printf( "no fps value given\n" );
	}

	return EXIT_SUCCESS;
}
