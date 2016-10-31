/*
source for Initialization class

Zinit.cpp
*/

#include "newengh.h"
#include "glide.h"
#include "ZInit.h"
#include <math.h>
#include "zrender.h"

LPZRender _rend;
LPZCamera _cam,_cam2;

void ZInit::OpenGEngine(DWORD dwHeight, DWORD dwWidth) {
	
	// Create the renderer
	//some would call this unnecesary abstraction, well tough.
	_rend = new ZRender(dwHeight, dwWidth);
}

void ZInit::CloseGEngine(void) {

	// Close 3d engine
	delete _rend;
}



