#ifndef _COMMON_H
#define _COMMON_H

#define MAX_NUM_ARGVS 50

extern uint8_t com_argc; // com_argv0 will always be empty
extern const char* com_argv[MAX_NUM_ARGVS + 1];

uint8_t COM_CheckParm( const char* parm );

const char* va( const char* format, ... );

void COM_InitFiles( void );
uint8_t* COM_FindFile( const char* name, int32_t* size );
void COM_ShutdownFiles( void );

#endif // _COMMON_H
