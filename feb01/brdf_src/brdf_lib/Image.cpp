#include "global.h"
#include "Image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <assert.h>

#include <iostream.h>

Image::Image() {
    width = height = 0;
    depth = 0;
    imageMemory = (Float *)0;
    loaded = false;
    filename[0] = '\0';
}

Image::Image( int w, int h, int d ) {
    width  = w;
    height = h;
    depth = d;
    imageMemory = new Float[w*h*depth];
    loaded = true;
    filename[0] = '\0';
    clear();
}

#define BUF_LEN 300
Image::Image( char *filename ) {
    char buffer[BUF_LEN], format[80];

    strncpy( Image::filename, filename, 199 );

    // falls beim lesen was schief geht.
    imageMemory = (Float *)0;
    loaded = false;
    width  = 0;
    height = 0;
    depth = 3;

    FILE *r = fopen( filename, "rb" );
    if( !r ) {
	cerr << "File " << filename << " not found\n";
	return;
    }

    testret( fread( format, 1, 6, r ), 6 );
    buffer[0] = '#';
    while( buffer[0] == '#' ) {
	fgets( buffer, BUF_LEN, r );
    }
    sscanf( buffer, "%d%d", &width, &height );
    
    if( strncmp( format, "Image", 5 ) ) {
	cerr << "Wrong File-Format. Only supported: Image\n";
	return;
    }

    imageMemory = new Float[width*height*depth];
    testret( fread( imageMemory, depth*sizeof(Float), width*height, r ), width*height );

    fclose( r );

    loaded = true;
}

Image::~Image() {
    if( imageMemory )
        delete[] imageMemory;
}

bool
Image::is_loaded() {
    return loaded;
}

void
Image::clear() {
    int size = width * height * depth, i;
    Float *ptr = imageMemory;

    for( i = 0; i < size; i++ ) {
	*ptr++ = 0;
    }
}

bool
Image::save( char *name ) {
    FILE *w;

    if( (w = fopen( name, "wb" ) ) ) {
	fprintf( w, "Image\n%d %d\n", width, height );

	int x, y;
	Float *ptr = imageMemory;
	for( y = 0; y < height; y++ ) {
	    for( x = 0; x < width; x++ ) {
		fwrite( ptr, depth, sizeof( Float ), w );
		ptr += depth;
	    }
	}

	fclose( w );
	// return true;
    }

    return false;
}

Double Image::getMinValue() {
    Float min = 9999999.9;
    int i;
    Float *ptr = imageMemory;

    for( i = 0; i < width*height*depth; i++ ) {
	if( *ptr < min ) {
	    min = *ptr;
	}

	ptr++;
    }

    return Double(min);
}

double Image::find_max_channel_value() {
    float max = -FLT_MAX;
    float *values = imageMemory;

    int i;
    for (i = 0; i < width * height * depth; i++) {
	if (values[i] > max) {
	    max = values[i];
	}
    }

    return (double)max;
}

void Image::scale_channels(double factor) {
    float *values = imageMemory;

    int i;
    for (i = 0; i < width * height * depth; i++) {
	values[i] *= factor;
    }
}

void Image::multiplyWith( Double fac ) {
    int i;
    Float *ptr = imageMemory;

    for( i = 0; i < width*height*depth; i++ ) {
	*ptr *= Float(fac);

	ptr++;
    }

    return;
}

void Image::addAndMultiply( Double fac, Double val ) {
    int i;
    Float *ptr = imageMemory;
    Float wert;

    for( i = 0; i < width*height*depth; i++ ) {
	wert = *ptr;
	*ptr++ = Float(fac) * (wert + Float(val));
    }

    return;
}

void Image::getInterpolatedPixel( Double x, Double y, 
				  Double &r, Double &g, Double &b ) {
    Double xf = floor(x); // integral value
    Double yf = floor(y);
    Double xf1 = Int(xf+1);
    Double yf1 = Int(yf+1);
    Double xp = x - xf;  // percentage
    Double yp = y - yf;

    if( xf1 >= width )  // wrap-around in x (hopefully this is phi!)
	xf1 = 0.0;
    if( yf1 >= height ) // do not wrap around in y (hopefully theta)
	yf1 = height-1;

    // xf,yf  -- xf1,yf     rgb_up
    //   |          |
    //   |          |
    // xf,yf1 -- xf1,yf1    rgb_down

    Double r_up, g_up, b_up;
    Double r_down, g_down, b_down;

    r_up   = (1-xp)*getRPixel( xf, yf  ) + xp*getRPixel( xf1, yf );
    r_down = (1-xp)*getRPixel( xf, yf1 ) + xp*getRPixel( xf1, yf1 );
    r = (1-yp)*r_up + yp*r_down;

    g_up   = (1-xp)*getGPixel( xf, yf  ) + xp*getGPixel( xf1, yf );
    g_down = (1-xp)*getGPixel( xf, yf1 ) + xp*getGPixel( xf1, yf1 );
    g = (1-yp)*g_up + yp*g_down;

    b_up   = (1-xp)*getBPixel( xf, yf  ) + xp*getBPixel( xf1, yf );
    b_down = (1-xp)*getBPixel( xf, yf1 ) + xp*getBPixel( xf1, yf1 );
    b = (1-yp)*b_up + yp*b_down;
}

void Image::changeEndians() {
    Float *ptr = imageMemory, *p;
    //    Float wert, neu;
    int *w, wert, neu;

    if( sizeof( Float ) != 4 || sizeof( int ) != 4 ) {
	cerr << "Relying on sizeof(float) = 4" << endl;
	assert(0);
	return;
    }

    for( int i = 0; i < width*height*depth; i++ ) {
	w = (int *)ptr;
	wert = *w;
	neu = (wert&0xFF) << 24
	    | (wert&0xFF00) << 8 
	    | (wert&0xFF0000) >> 8 
	    | (wert&0xFF000000) >> 24;
	p = (Float *)&neu;
	*ptr++ = *p;
    }
}

void Image::testret( int ret, int shouldbe )
{
    if( (shouldbe != INT_MAX && ret != shouldbe) ||
	(shouldbe == INT_MAX && ret == 0) )
	perror( (char *)0 );
    
    return;
}
