#ifndef IMAGE_H
#define IMAGE_H

#include "global.h"

#define MEMPTR(xx,yy,i) imageMemory[(xx+yy*width)*depth+i]
#define checkCoords( x, y ) { if( x >= width || y >= height ) return; }

class Image {
public:
    Image( int width, int height, int depth=3 );
    Image( char *filename );
    Image();
    ~Image();

    // Query

    bool is_loaded();
    Float *getBitmap() { return imageMemory; }
    char *getFileName() { return filename; }
    int getWidth( void ) { return width; }
    int getHeight( void ) { return height; }
    int getDepth( void ) { return depth; }

    void scale_channels(double factor);
    double find_max_channel_value();

    // Pixel - Ops

    // write

    void putPixel( int x, int y, Double r, Double g, Double b ) {
	checkCoords( x, y );
	Float *ptr = &MEMPTR(x,y,0);

	*ptr++ = Float(r);
	*ptr++ = Float(g);
	*ptr   = Float(b);
    }
    void addPixel( int x, int y, Double r, Double g, Double b ) {
	checkCoords( x, y );
	Float *ptr = &MEMPTR(x,y,0);

	*ptr++ += Float(r);
	*ptr++ += Float(g);
	*ptr   += Float(b);
    }

    // in case we have depth > 3
    void putPixel( int x, int y, int i, Double v ) { MEMPTR(x,y,i) = (Float)v; }

    void putRPixel( int x, int y, Double r ) { MEMPTR(x,y,0) = (Float)r; }
    void putGPixel( int x, int y, Double g ) { MEMPTR(x,y,1) = (Float)g; }
    void putBPixel( int x, int y, Double b ) { MEMPTR(x,y,2) = (Float)b; }

    // put Pixel in Mem (indexed by i only, i.e. i > 1*x, then we're in next row!)
    void putRPixel( int i, Double r ) { MEMPTR(i,0,0) = (Float)r; }
    void putGPixel( int i, Double g ) { MEMPTR(i,0,1) = (Float)g; }
    void putBPixel( int i, Double b ) { MEMPTR(i,0,2) = (Float)b; }

    // add Pixel in Mem (indexed by i only, i.e. i > 1*x, then we're in next row!)
    void addRPixel( int i, Double r ) { MEMPTR(i,0,0) += (Float)r; }
    void addGPixel( int i, Double g ) { MEMPTR(i,0,1) += (Float)g; }
    void addBPixel( int i, Double b ) { MEMPTR(i,0,2) += (Float)b; }

    // read

    inline Double getPixel( int x, int y, int i );
    inline void getPixel( int x, int y, 
			  Double &r, Double &g, Double &b );
    inline Double getRPixel( int x, int y );
    inline Double getGPixel( int x, int y );
    inline Double getBPixel( int x, int y );
			

    // read pixel value, but interpolated with adjacent pixels
    void getInterpolatedPixel( Double x, Double y, 
			       Double &r, Double &g, Double &b );

    // Other stuff

    Double getMinValue();

    void multiplyWith( Double fac );
    void addAndMultiply( Double fac, Double val );

    void clear();
    virtual bool save( char *name );

    // this is for other architectures...
    void changeEndians();

    Float *imageMemory;
    int width, height, depth;
protected:
    bool loaded;
    char filename[200];

    void testret( int ret, int shouldbe );
};

// -- inline functions --

inline Double 
Image::getPixel( int x, int y, int i ) {
    return MEMPTR(x,y,i);
}

inline void 
Image::getPixel( int x, int y, Double &r, Double &g, Double &b ) {
    if( x >= width || y >= width) {
        r = g = b = 0.0;
        return;
    }
    
    Float *ptr = &MEMPTR(x,y,0);
    
    r = Double(*ptr++);
    g = Double(*ptr++);
    b = Double(*ptr);
}

inline Double
Image::getRPixel( int x, int y ) {
    if( x >= width || y >= height ) {
	return 0.0;
    }
    
    return MEMPTR(x,y,0);
}

inline Double
Image::getGPixel( int x, int y ) {
    if( x >= width || y >= height ) {
	return 0.0;
    }
    
    return MEMPTR(x,y,1);
}

inline Double
Image::getBPixel( int x, int y ) {
    if( x >= width || y >= height ) {
	return 0.0;
    }
    
    return MEMPTR(x,y,2);
}

#endif
