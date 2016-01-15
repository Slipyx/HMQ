#ifndef _Q_STL_H
#define _Q_STL_H

void Q_strcpy( char* dest, const char* src );
void Q_strncpy( char* dest, const char* src, uint32_t count );
uint32_t Q_strlen( const char* str );
int8_t Q_strcmp( const char* s1, const char* s2 );
int32_t Q_atoi( const char* str );

#endif // _Q_STL_H
