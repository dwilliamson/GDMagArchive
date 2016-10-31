// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : raster.cpp
// Description : Functions that initialize and use Glide
// Note : this rasterizer uses Glide 2.X I think about writting a new version which
//			will use Glide 3.X
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------
#include "raster.hpp"


int OpenRaster(void)
{
	GrHwConfiguration hwconfig;
	float					wWidth, wHeight;
	GrScreenResolution_t	screenRes;

	grGlideInit();

	if ( !grSstQueryHardware( &hwconfig ) )
	{
		grGlideShutdown();
		return false;
	}

	grSstSelect( 0 );

	wWidth = 640.f;
	wHeight = 480.f;
	screenRes = GR_RESOLUTION_640x480;

	// Open up the hardware
	if ( !grSstWinOpen( 0,
		screenRes,
		GR_REFRESH_60Hz,
		GR_COLORFORMAT_ARGB,
		GR_ORIGIN_UPPER_LEFT,
		2,
		1	// 1 buffer aux (Z ou W buffer ou alpha)
		) )

	{
		grGlideShutdown();
		return false;
	}

	grCullMode(GR_CULL_POSITIVE);

	// zbuffer
	grDepthBufferMode( GR_DEPTHBUFFER_WBUFFER );
	grDepthBufferFunction( GR_CMP_LEQUAL );
	grDepthMask( FXTRUE );

	grTexFilterMode(GR_TMU0,
				GR_TEXTUREFILTER_BILINEAR,
				GR_TEXTUREFILTER_BILINEAR);

	// gamma
	grGammaCorrectionValue((float)0.8);

	grDitherMode(GR_DITHER_DISABLE);
	return 0;
}




 
void CloseRaster(void)
{
    grGlideShutdown();
}


void ClearBuffer(unsigned long Color)
{
	grBufferClear(Color, 0, GR_WDEPTHVALUE_FARTHEST);
}
