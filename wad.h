#ifndef _WAD_H
#define _WAD_H

void W_LoadFile( const char* wadName );
pic_t* W_GetPicLump( const char* lumpName );
void W_CloseFile( void );

#endif // _WAD_H
