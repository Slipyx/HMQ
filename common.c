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

	static char buf[1024];
	vsnprintf( buf, 1024, format, vl );

	va_end( vl );

	return buf;
}
