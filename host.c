#include "quakedef.h"

// timestep info
float realTime = 0, frameTime = 0;

void Host_Init( void ) {
}

// takes in a raw delta time and proceeds if enough
// has accumulated for another fixed timestep
void Host_Frame( float _t ) {
	static float ort = 0;
	realTime += _t;

	if ( realTime - ort > 1.0f / 72 ) {
		frameTime = realTime - ort;
		ort = realTime;
	} else return;

	// cap frameTime so we dont get big time steps
	if ( frameTime > 0.1f ) frameTime = 0.1f;
	// enough frame time has passed for another frame update

	// do it
	printf( "%.7f\n", realTime );
}

void Host_Shutdown( void ) {
}
