#ifndef _WAD_H
#define _WAD_H

void W_LoadWadFile( const char* fileName );
uint8_t* W_GetLumpName( const char* name );
void W_CloseWadFile( void );

#endif // _WAD_H
