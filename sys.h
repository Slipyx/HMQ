#ifndef _SYS_H
#define _SYS_H

void Sys_Shutdown( void );
void Sys_SendKeyEvents( void );

// file api
int32_t Sys_FileOpenRead( const char* path, int32_t* fsize );
int32_t Sys_FileOpenWrite( const char* path );
void Sys_FileClose( int32_t hnd );
void Sys_FileSeek( int32_t hnd, uint32_t pos );
uint32_t Sys_FileRead( int32_t hnd, void* dst, uint32_t cnt );
uint32_t Sys_FileWrite( int32_t hnd, const void* src, uint32_t cnt );

#endif // _SYS_H
