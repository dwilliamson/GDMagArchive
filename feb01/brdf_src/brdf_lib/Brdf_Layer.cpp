#include <stdio.h>
#include <math.h>
#include <fstream.h>
#include <stdio.h>
#include <assert.h>

#include "global.h"
#include "Brdf_Layer.h"

Brdf_Layer::Brdf_Layer( int width, int height ) {
    tex[TEXTURE1] = new Brdf_Image(width, height);
    tex[TEXTURE2] = new Brdf_Image(width, height);

    alpha1 = alpha2 = 1.0;
    beta1 = beta2 = 0.0;
    lambda = 1.0;
    delta = 1.0;
}

Brdf_Layer::Brdf_Layer( char *header ) {
    char filename[160];
    ifstream layer;

    sprintf( filename, "%s", header );
    layer.open( filename );
    layer >> alpha1;
    layer >> alpha2;
    layer >> beta1;
    layer >> beta2;
    layer >> lambda;
    layer >> delta;
    layer.close();

    sprintf(filename, "%s.l%d", header, 1);
    tex[TEXTURE1] = new Brdf_Image(filename);
    sprintf(filename, "%s.l%d", header, 2);
    tex[TEXTURE2] = new Brdf_Image(filename);

}

static void get_double(FILE *f, double *result) {
    char buf[BUFSIZ];
    char *s = fgets(buf, BUFSIZ, f);
    assert(s != NULL);

    double val;
    sscanf(s, "%lf", &val);
    *result = val;
}

Brdf_Layer::Brdf_Layer(ifstream &layer) {
    layer >> alpha1;
    layer >> alpha2;
    layer >> beta1;
    layer >> beta2;
    layer >> lambda;
    layer >> delta;

    // now read the '\n'
    char dummy[11];
    layer.getline( dummy, 10 );

    tex[TEXTURE1] = new Brdf_Image(layer);
    tex[TEXTURE2] = new Brdf_Image(layer);
}

Brdf_Layer::~Brdf_Layer() {
    delete tex[TEXTURE1];
    delete tex[TEXTURE2];
}

bool Brdf_Layer::is_loaded() {
    if (!tex[TEXTURE1]->is_loaded()) return false;
    if (!tex[TEXTURE2]->is_loaded()) return false;
    return true;
}

bool Brdf_Layer::save(char *header) {
    char filename[160];
    bool ret;

    ofstream layer;

    sprintf( filename, "%s", header );
    layer.open( filename );
    layer << alpha1 << endl;
    layer << alpha2 << endl;
    layer << beta1 << endl;
    layer << beta2 << endl;
    layer << lambda << endl;
    layer << delta << endl;
    layer.close();

    sprintf( filename, "%s.l%d", header, 1 );
    ret = tex[TEXTURE1]->save( filename );
    
    sprintf( filename, "%s.l%d", header, 2 );
    ret = ret & tex[TEXTURE2]->save( filename );

    return ret;
}

bool Brdf_Layer::save( ofstream &layer ) {
    bool ret;

    layer << alpha1 << endl;
    layer << alpha2 << endl;
    layer << beta1 << endl;
    layer << beta2 << endl;
    layer << lambda << endl;
    layer << delta << endl;

    ret = tex[TEXTURE1]->save( layer );
    ret = ret & tex[TEXTURE2]->save( layer );

    return ret;
}

void Brdf_Layer::convertToGLSphereLayer() {
    convertToSphereLayer( GLSphere );
}

void Brdf_Layer::convertToAngleSphereLayer() {
    convertToSphereLayer( AngleSphere );
}

void Brdf_Layer::convertToSphereLayer( SphereType st ) {
    // w/h of tex1 and tex2 are the same, see constructor

#define AUSLAUFEN

    int oldwidth  = tex[TEXTURE1]->getWidth();
    int oldheight = tex[TEXTURE1]->getHeight();
    int width = oldwidth;
    int height = oldheight;

    Brdf_Image *newtex1 = new Brdf_Image( width, height );
    Brdf_Image *newtex2 = new Brdf_Image( width, height );
    
    Double theta, phi;
    Double the_p, phi_p;
    Double r1, g1, b1;
    Double r2, g2, b2;
    Int inout;

    for( int x = 0; x < width; x++ ) {
	// Convert to -1..1
	Double xx = 2.0 * ((Double)x / (Double)(width-1) - .5);

	inout = 0;

	for( int y = 0; y < height; y++ ) {
	    Double yy = 2.0 * ((Double)y / (Double)(height-1) - .5);
	    Double xylen = xx*xx + yy*yy;

            // Because I won't hit xx = 0.0 and yy = 0.0 exactly
            // I have to be inaccurate about the length! Or I clip
            // away too much
	    if( xylen > 1.01 ) {
		if( inout ) {
		    // I actually have been inside already
		    // copy it to the next pixel 
		    for( int fy = y; fy < height; fy++ ) {
			// Wenn Auslaufen erwuenscht!
#ifdef AUSLAUFEN
			newtex1->putPixel( x, fy, r1, g1, b1 );
			newtex2->putPixel( x, fy, r2, g2, b2 );
#endif
		    }
		    break;
		}
		continue; // outside
	    }

	    // Inside the unit-circle
	    inout++;

            // But for the calculation the length of the
            // vector must not be large than 1.0 of course!
	    if( xylen > 1.0 ) xylen = 1.0;

	    Double zz = sqrt(1.0 - xylen);

	    if( st == GLSphere ) {
		// Calc theta
		// (Basically <(xx,yy,zz);(0,0,1)> = cos(theta))
		theta = acos(zz);
	    } else if( st == AngleSphere ) {
		// EXP: theta prop sqrt(xylen)
		theta = sqrt(xylen)*M_PI_2;
	    } else {
		cerr << "Brdf_Layer::SphereType " << st << " not supported!" << endl;
	    }

	    // Calc phi
	    // (Basically <(xx,yy,0);(1,0,0)> = cos(phi))
	    // DEBUG: was <(xx,yy,0);(0,1,0)>, but 
	    //  turned all by 90Degrees.
	    phi = acos(xx/sqrt(xylen));
	    if( yy < 0.0 ) phi = 2*M_PI - phi;  // 180 - 360 Degrees

	    the_p = (theta/(M_PI_2  ))*(oldwidth-1);
	    phi_p = (phi  /(2.0*M_PI))*(oldheight-1);

	    tex[TEXTURE1]->getInterpolatedPixel( phi_p, the_p, r1, g1, b1 );
	    newtex1->putPixel( x, y, r1, g1, b1 );

	    tex[TEXTURE2]->getInterpolatedPixel( phi_p, the_p, r2, g2, b2 );
	    newtex2->putPixel( x, y, r2, g2, b2 );

	    if( inout == 1 ) {
		// I'm inside the first time
		for( int fy = 0; fy < y; fy++ ) {
		    // Wenn Auslaufen erwuenscht!
#ifdef AUSLAUFEN
		    newtex1->putPixel( x, fy, r1, g1, b1 );
		    newtex2->putPixel( x, fy, r2, g2, b2 );
#endif
		}
	    }
	}
    }


    
    delete tex[TEXTURE1];
    delete tex[TEXTURE2];

    tex[TEXTURE1] = newtex1;
    tex[TEXTURE2] = newtex2;

    return;
}


// TODO: ctrl-k that

void Brdf_Layer::convertToDeltaLayer() {
    // w/h of tex1 and tex2 are the same, see constructor
    /* TODO: get rid of this
    int oldwidth  = tex[TEXTURE1]->getWidth();
    int oldheight = tex[TEXTURE1]->getHeight();
    int width = oldwidth;
    int height = oldheight;

    Brdf_Image *newtex1 = new Brdf_Image( width, height );
    Brdf_Image *newtex2 = new Brdf_Image( width, height );
    
    Double theta, phi;
    Double the_p, phi_p;
    Double delta_in, delta_out;
    Double r1, g1, b1;
    Double r2, g2, b2;

    // Convert to theta
    for( int x = 0; x < width; x++ ) {
	Double xx = 2.0 * ((Double)x / (Double)(width-1) - .5);
	delta_in = xx * M_PI; // -90 degrees to 90 degrees

	for( int y = 0; y < height; y++ ) {
	    Double yy = 2.0 * ((Double)y / (Double)(height-1) - .5);
	    delta_out = xx * M_PI; // -90 degrees to 90 degrees

	    
	}
    }

    delete tex[TEXTURE1];
    delete tex[TEXTURE2];

    tex[TEXTURE1] = newtex1;
    tex[TEXTURE2] = newtex2;
    */
    return;
}

void Brdf_Layer::calculateBiasFactors() {
    Double min1, min2, max1, max2;

    min1 = tex[TEXTURE1]->getMinValue();
    max1 = tex[TEXTURE1]->find_max_channel_value();
    min2 = tex[TEXTURE2]->getMinValue();
    max2 = tex[TEXTURE2]->find_max_channel_value();

    /* DEBUG: Was used because of a bug in
              in Image.C::getMinValue()
    Double min, max;

    if( min1 < min2 ) min = min1;
    else min = min2;
    if( max1 > max2 ) max = max1;
    else max = max2;

    alpha1 = max-min;
    alpha2 = max-min;
     beta1 = -min;
     beta2 = -min;
     */

    alpha1 = max1-min1;
    alpha2 = max2-min2;
     beta1 = -min1;
     beta2 = -min2;
     

    /* DEBUG
       printf( "min1: %.3f  max1: %.3f\n", min1, max1 );
       printf( "min2: %.3f  max2: %.3f\n", min2, max2 );
       printf( "a1: %.3f  b1: %.3f\n", alpha1, beta1 );
       printf( "a2: %.3f  b2: %.3f\n", alpha2, beta2 );
       printf( "delta: %.3f  lambda: %.3f\n", delta, lambda );
       */


    // Otherwise not valid
    if( alpha1 == 0.0 ) alpha1 = 1.0;
    if( alpha2 == 0.0 ) alpha2 = 1.0;

    delta = alpha1*alpha2*lambda;
}

void Brdf_Layer::makeBiasedLayer() {
    calculateBiasFactors();
    makeBiasedLayer( alpha1, alpha2, beta1, beta2 );
}

void Brdf_Layer::makeBiasedLayer( Double a1, Double a2, Double b1, Double b2 ) {
    alpha1 = a1;
    alpha2 = a2;
    beta1  = b1;
    beta2  = b2;
    delta = alpha1*alpha2*lambda;

    tex[TEXTURE1]->addAndMultiply( 1.0/alpha1, beta1 );
    tex[TEXTURE2]->addAndMultiply( 1.0/alpha2, beta2 );

    return;
}

/**
 * This here is intended for one layer approximation of BRDF (works
 * several layers though). It tries to multiply the Lambda value into
 * the texture map. If values in texture map woudl exceed values [-1,1],
 * only parts of Lambda is multiplied into the texture map. The whole thing
 * is done, because of the limited precision of texture maps/framebuffer.
 * NOTE: Shouldn't be mixed with biased layers. If absolutely wanted, then
 *       call this here BEFORE biasing the layer.
 */
void Brdf_Layer::shuffleLambdaToTextures() {
    Double min1, min2, max1, max2;
    Double s = 1.0;
    Double fac1 = 1.0, fac2 = 1.0;

    min1 = tex[TEXTURE1]->getMinValue();
    max1 = tex[TEXTURE1]->find_max_channel_value();
    min2 = tex[TEXTURE2]->getMinValue();
    max2 = tex[TEXTURE2]->find_max_channel_value();


    // Both all negative -> switch sign (- * - = +)
    if( max1 <= 0.00001 && max2 <= 0.00001 ) {
	max1 = -min1;
	max2 = -min2;
	s = -1.0;
    }

    // This is the maximum factor that can be moved into the texture map.
    // (No value in the texture map will be > 1.0 then)

    fac1 /= max1;
    fac2 /= max2;

    lambda = delta = lambda/(fac1*fac2);

    tex[TEXTURE1]->addAndMultiply( s*fac1, 0.0 );
    tex[TEXTURE2]->addAndMultiply( s*fac2, 0.0 );

    return;

    // No, we shuffle all into the texturemap (values > 1.0 are likely)
    // While this is actually possible, rendering quality is very poor.
    // This is because texture maps can only contain value > 1.0, as well
    // as the framebuffer. So large values will be cut off and highlight
    // looks dim, because it is attenuated by cos(a)!
    /*
      factor = sqrt(lambda);
      lambda = delta = 1.0;
    */

    // This was when I assumed, that it's not
    // possible to have different factors
    // for each texmap:
    //   factor /= max;
    //   lambda = delta = lambda/(factor*factor);

    //   tex[TEXTURE1]->addAndMultiply( s*factor, 0.0 );
    //   tex[TEXTURE2]->addAndMultiply( s*factor, 0.0 );
}
