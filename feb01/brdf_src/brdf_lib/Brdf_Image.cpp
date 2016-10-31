#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "global.h"
#include "Brdf_Image.h"
#include "Image.h"

Brdf_Image::Brdf_Image(int w, int h) : Image( w, h ) {
    saveRange = range_m1_1;
}

#define BUF_LEN 300
Brdf_Image::Brdf_Image(char *filename) {
    ifstream in;
    in.open(filename);
    construct(in);
    in.close();
}

Brdf_Image::Brdf_Image(ifstream &in) {
    construct(in);
}

void Brdf_Image::construct(ifstream &in) {
    char buffer[BUF_LEN], format[BUF_LEN];

    // falls beim lesen was schief geht.
    imageMemory = (Float *)0;
    loaded = false;
    width  = 0;
    height = 0;
    depth = 3;

    in.getline(format, BUF_LEN);
    buffer[0] = '#';
    while( buffer[0] == '#' ) {
	in.getline( buffer, BUF_LEN );
    }
    sscanf( buffer, "%d%d", &width, &height );
    
    if (strncmp(format, "BRDFImage", 9) &&
	strncmp(format, " BRDFImage", 10)) {
	cerr << "Wrong File-Format. Only supported: BRDFImage\n";
	return;
    }

    imageMemory = new Float[width*height*depth];

    int i;
    for (i = 0; i < width * height * depth; i++) {
        Float f;
	in >> f;
	imageMemory[i] = f;
    }

    loaded = true;
}

Brdf_Image::~Brdf_Image() {
    // XXX fill this in!
}



bool Brdf_Image::save(ofstream &out) {
    out << "BRDFImage" << endl << width << " " << height << endl;

    int x, y;
    Float *ptr = imageMemory;
    for( y = 0; y < height; y++ ) {
	for( x = 0; x < width; x++ ) {
	    out.write( (char *)ptr, depth*sizeof( Float ) );
	    ptr += depth;
	}
    }

    return true;
}

#ifdef NOT    
bool Brdf_Image::ascii_save(ofstream &out) {
    out << "BRDFImage" << endl << width << " " << height << endl;

    int x, y;
    Float *ptr = imageMemory;
    for( y = 0; y < height; y++ ) {
	for( x = 0; x < width; x++ ) {
	    out << *ptr;
	    out << " ";
	}
    }

    return true;
}
#endif NOT
    
bool
Brdf_Image::save( char *name ) {
    FILE *w;

    
    if( (w = fopen( name, "wb" ) ) ) {
	fprintf( w, "Brdf_Image\n%d %d\n", width, height );

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
    

    // for testing only
    char fname[100];
    sprintf( fname, "%s.ppm", name );

    if( (w = fopen( fname, "wb" ) ) ) {
	fprintf( w, "P6\n%d %d\n255\n", width, height );

	Float *ptr = imageMemory;
	int x, y, i;
	for( y = 0; y < height; y++ ) {
	    for( x = 0; x < width; x++ ) {
		for( i = 0; i < depth; i++ ) {
		    Int val;

		    if( saveRange == range_m1_1 )
			val = (Int)((*ptr+1.0) * 127.0);
		    else
			val = (Int)((*ptr+0.0) * 255.0);

		    if( val < 0 ) val = 0;
		    if( val > 255 ) val = 255;
		    UByte v = UByte(val);
		    fwrite( &v, 1, sizeof( UByte ), w );
		    ptr++;
		}
	    }
	}

	fclose( w );
	return true;
    }
    

    return false;
}

void Brdf_Image::setSaveRange( SaveRange sr ) {
    saveRange = sr;
}
