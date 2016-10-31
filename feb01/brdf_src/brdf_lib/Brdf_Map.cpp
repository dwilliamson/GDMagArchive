#include <stdio.h>
#include <string.h>
#include <fstream.h>
#include <assert.h>
#include <stdio.h>
#include <assert.h>

#include "global.h"
#include "Brdf_Map.h"

#include "params/all.h"





Brdf_Map::Brdf_Map(Brdf_Param bp1, Brdf_Param bp2, 
		   int w, int h, int layers ) {
    Brdf_ParamL1 = bp1;
    Brdf_ParamL2 = bp2;
    gamma = 0.0;
    width  = w;
    height = h;

    nrLayers = layers;
    layer = new Brdf_Layer *[nrLayers];

    for( int i = 0; i < nrLayers; i++ ) {
	layer[i] = new Brdf_Layer( width, height );
    }

    name[0] = '\0';

    instantiateParameterizer( Brdf_ParamL1, Brdf_ParamL2 );
}

static void get_double(FILE *f, double *result) {
    char buf[BUFSIZ];
    char *s = fgets(buf, BUFSIZ, f);
    assert(s != NULL);

    double val;
    sscanf(s, "%lf", &val);
    *result = val;
}

static void get_int(FILE *f, int *result) {
    char buf[BUFSIZ];
    char *s = fgets(buf, BUFSIZ, f);
    assert(s != NULL);
    int val;

    sscanf(s, "%d", &val);
    *result = val;
}

void clean_string(char *s) {
    int len = strlen(s);
    if ((len > 1) && (s[len - 1] == 10) && (s[len - 2] == 13)) {
        s[len - 2] = '\0';
    }
}

Brdf_Map::Brdf_Map(char *mapfilename) {
    ifstream brdfmap;
    char id[20];
    char header[200];
    char filename[200];
    int type, p1, p2;

    strcpy( filename, mapfilename );

    brdfmap.open( mapfilename );
    if( !brdfmap ) {
	// Let's try to append .brdfmap
	sprintf( filename, "%s.brdfmap", mapfilename );
	brdfmap.open( filename );
    }

    // Did it work at all?
    if( !brdfmap ) {
	cerr << "Couldn't open " << filename << endl;
	layer = new Brdf_Layer *[1];  // cause it will be deleted
	nrLayers = 0;
	name[0] = '\0';
	return;
    }

    // remember name
    strcpy( name, filename );

    brdfmap >> id;

    if( !strcmp( id, "BRDFMap" ) &&
	!strcmp( id, "BRDFMapCC" ) ) {  // CC: concatenated
	nrLayers = 0;
	return;
    }

    brdfmap.getline( header, 200 ); // get '\n'
    brdfmap.getline( header, 200 ); // get actual line
    brdfmap >> width;
    brdfmap >> height;
    brdfmap >> type;
    brdfmap >> p1;
    brdfmap >> p2;
    brdfmap >> nrLayers;
    brdfmap >> gamma;

    Brdf_ParamL1 = (Brdf_Param)p1;
    Brdf_ParamL2 = (Brdf_Param)p2;

    bool swap_images = false;
    if (Brdf_ParamL1 == tout_pout) {
        swap_images = true;
    }

    if (swap_images) {
        Brdf_Param tmp = Brdf_ParamL1;
	Brdf_ParamL1 = Brdf_ParamL2;
	Brdf_ParamL2 = tmp;
    }

    instantiateParameterizer( Brdf_ParamL1, Brdf_ParamL2 );
  
    layer = new Brdf_Layer *[nrLayers];

    if( !strcmp( id, "BRDFMap" ) ) {
	for( Cardinal i = 0; i < nrLayers; i++ ) {
	    sprintf( filename, "%s.brdflayer.%02d", header, i );
	    layer[i] = new Brdf_Layer( filename );
	    
	    if (swap_images) {
 	        Brdf_Image *i0 = layer[i]->get_image(Brdf_Layer::TEXTURE1);
 	        Brdf_Image *i1 = layer[i]->get_image(Brdf_Layer::TEXTURE2);

		layer[i]->tex[0] = i1;
		layer[i]->tex[1] = i0;
	    }
	}
	
    } else {
	// Concatenated
	for( Cardinal i = 0; i < nrLayers; i++ ) {
	    layer[i] = new Brdf_Layer( brdfmap );
	}
    }

    brdfmap.close();


    Brdf_Layer *top_layer = layer[0];
    assert(layer != NULL);

    Brdf_Image *source = top_layer->get_image(Brdf_Layer::TEXTURE1);
    int width, height;
    width = source->getWidth();
    height = source->getHeight();

    Brdf_Image *dest = new Brdf_Image(width, height);
    top_layer->tex[Brdf_Layer::TEXTURE_AMBIENT] = dest;

    float sum_r = 0.0f;
    float sum_g = 0.0f;
    float sum_b = 0.0f;
    float scale = 1.0 / (double)(width * height);

    int k;
    int i, j;
    for (k = 0; k < nrLayers; k++) {
        Brdf_Layer *this_layer = layer[0];
	Brdf_Image *source = this_layer->get_image(Brdf_Layer::TEXTURE1);

	for (j = 0; j < height; j++) {
	    for (i = 0; i < width; i++) {
	        sum_r += source->getRPixel(i, j);
		sum_g += source->getGPixel(i, j);
		sum_b += source->getBPixel(i, j);
	    }
	}
    }

    sum_r *= scale;
    sum_g *= scale;
    sum_b *= scale;

    float max_rgb = MAX(sum_r, sum_g);
    max_rgb = MAX(max_rgb, sum_b);
    float rescale = 1.0f;

    if (max_rgb > 0.0f) {
        rescale = 1.0f / max_rgb;
    }

    sum_r *= rescale;
    sum_g *= rescale;
    sum_b *= rescale;

    double lambda = top_layer->getLambda();
    top_layer->ambient_lambda = lambda / rescale;

    double r, g, b;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
	    r = source->getRPixel(i, j);
	    g = source->getGPixel(i, j);
	    b = source->getBPixel(i, j);

	    r *= sum_r;
	    g *= sum_g;
	    b *= sum_b;

	    dest->putPixel(i, j, r, g, b);
	}
    }
}

Brdf_Map::Brdf_Map(char *mapfilename, int NEW_CODE) {
    char id[20];
    char header[200];
    char filename[200];
    int p1, p2;

    strcpy( filename, mapfilename );

    ifstream map_file;
    map_file.open(filename);

    if (!map_file) {
	// Let's try to append .brdfmap
	sprintf(filename, "%s.brdfmap", mapfilename);
	map_file.open(filename);
    }

    // Did it work at all?
    if (!map_file) {
	cerr << "Couldn't open " << filename << endl;
	layer = new Brdf_Layer *[1];  // cause it will be deleted
	nrLayers = 0;
	assert(0);
	name[0] = '\0';
	return;
    }

    // remember name
    strcpy(name, filename);

    map_file >> id;

    if( !strcmp( id, "BRDFMap" ) &&
	!strcmp( id, "BRDFMapCC" ) ) {  // CC: concatenated
	nrLayers = 0;
	return;
    }

    int junk;

    char *s;
    map_file.getline(header, 200);  // get '\n'
    map_file.getline(header, 200);  // get actual line

    map_file >> width;
    map_file >> height;
    map_file >> junk;
    map_file >> p1;
    map_file >> p2;
    map_file >> nrLayers;
    map_file >> gamma;

    Brdf_ParamL1 = (Brdf_Param)p1;
    Brdf_ParamL2 = (Brdf_Param)p2;

    instantiateParameterizer( Brdf_ParamL1, Brdf_ParamL2 );
  
    layer = new Brdf_Layer *[nrLayers];

    if( !strcmp( id, "BRDFMap" ) ) {
	for( int i = 0; i < nrLayers; i++ ) {
	    sprintf( filename, "%s.Brdf_Layer.%02d", header, i );
	    layer[i] = new Brdf_Layer( filename );
	}
    } else {
	// Concatenated
	for( int i = 0; i < nrLayers; i++ ) {
	    layer[i] = new Brdf_Layer(map_file);
	}
    }

    map_file.close();
}

Brdf_Map::~Brdf_Map() {
    for( int i = 0; i < nrLayers; i++ ) {
	delete layer[i];
    }

    delete[] layer;
}

bool Brdf_Map::save( char *header ) {
    ofstream Brdf_Map;
    char filename[80];

    sprintf( filename, "%s.Brdf_Map", header );

    // remember name
    strcpy( name, filename );

    Brdf_Map.open( filename );

    Brdf_Map << "Brdf_MapCC" << endl;
    Brdf_Map << "One Line Comment here..." << endl;
    Brdf_Map << width << endl;
    Brdf_Map << height << endl;
    Brdf_Map << int(0) << endl;
    Brdf_Map << int(Brdf_ParamL1) << endl;
    Brdf_Map << int(Brdf_ParamL2) << endl;
    Brdf_Map << nrLayers << endl;
    Brdf_Map << gamma << endl;

    for( int i = 0; i < nrLayers; i++ ) {
	layer[i]->save( Brdf_Map );
    }

    Brdf_Map.close();

    return true;
}

Brdf_Layer *
Brdf_Map::get_layer( int l ) {
    return layer[l];
}

Brdf_Image *
Brdf_Map::get_image( int l, Brdf_Layer::Brdf_Texture tex ) {
    return layer[l]->get_image( tex );
}

void Brdf_Map::convertToGLSphereMap() {
    if ((getParam1() != tin_pin) && (getParam1() != th_ph_gs)) {
        assert(0);
    }

    int i;
    for (i = 0; i < nrLayers; i++) {
	layer[i]->convertToGLSphereLayer();
    }

    if (getParam1() == tin_pin) {
	setParam1(gl_sphere_in);
	setParam2(gl_sphere_out);
    } else if (getParam1() == th_ph_gs) {
	setParam1(xh_yh);
	setParam2(xd_yd);
    } else {
        assert(0);
    }

    delete parameterizer;
    instantiateParameterizer(getParam1(), getParam2());

    // While converting the layers, size may change
    width = layer[0]->get_image(Brdf_Layer::TEXTURE1)->getWidth();
    height = layer[0]->get_image(Brdf_Layer::TEXTURE1)->getHeight();

    return;
}

void Brdf_Map::convertToAngleSphereMap() {
    if( getParam1() != tin_pin &&
	getParam1() != tout_pout &&
	getParam1() != th_ph &&
	getParam1() != th_ph_a ) {
	cerr << "Can only convert tin_pin/tout_pout/th_ph/th_ph_a to AngleSphereMap!" << endl;
	return;
    }

    for( int i = 0; i < nrLayers; i++ ) {
	layer[i]->convertToAngleSphereLayer();
    }

    if( getParam1() == tin_pin ) {
	setParam1( angle_sphere_in );
	setParam2( angle_sphere_out );
    } else if( getParam1() == tout_pout ) {
	setParam1( angle_sphere_out );
	setParam2( angle_sphere_in );
    } else if( getParam1() == th_ph ) {
	setParam1( angle_sphere_tph );
	setParam2( angle_sphere_tpd );
    } else if( getParam1() == th_ph_a ) {
	setParam1( angle_sphere_tph_a );
	setParam2( angle_sphere_tpd_a );
    } else {
	cerr << "Internal error: Brdf_Map.C::convertToAngleSphereLayer()\n";
    }

    delete parameterizer;
    instantiateParameterizer( getParam1(), getParam2() );

    return;
}

void Brdf_Map::makeBiasedMap() {
    int i, j;

    return;
}

void Brdf_Map::shuffleLambdasToTextures() {
    int i;

    // Now shuffle lambdas around
    for( i = 0; i < nrLayers; i++ ) {
	layer[i]->shuffleLambdaToTextures();
    }

    return;
}

// Works only for One Layer!!!
void Brdf_Map::scaleLambdasAndDeltas( Double lambda ) {
    if( nrLayers > 1 ) {
	cerr << "scaleLambdasAndDeltas() works only for ONE layer!" << endl;
	return;
    }

    // lambda == delta for a One Layer BRDF

    Double fac = sqrt(layer[0]->getLambda()/lambda);

    Double asym1 = 1.0, asym2 = 1.0;

#ifdef O2
    // TODO: this is kind of a hack. Usually the first
    //       factor is bighter, and the second one
    //       is darker (but also contains a few high values).
    //       This avoids overemphasis on the second factor.
    asym1 = 0.5; asym2 = 2.0;
#endif

    layer[0]->get_image( Brdf_Layer::TEXTURE1 )->
	addAndMultiply( fac*asym1, 0.0 );
    layer[0]->get_image( Brdf_Layer::TEXTURE2 )->
	addAndMultiply( fac*asym2, 0.0 );

    // set lambda and delta
    layer[0]->setLambda( lambda );
}

void Brdf_Map::rescale_for_target_lambda(double target_lambda,
					 double first_intermediate) {
    int i;
    for (i = 0; i < nrLayers; i++) {
        Brdf_Layer *l = layer[i];

	Brdf_Image *i0 = l->get_image(Brdf_Layer::TEXTURE1);
	Brdf_Image *i1 = l->get_image(Brdf_Layer::TEXTURE2);

	//	double k0 = i0->find_max_channel_value();
	//	double k1 = i1->find_max_channel_value();

	double lambda = l->lambda;
	double factor_squared = lambda / target_lambda;
	double factor = sqrt(factor_squared);

	double f0 = factor;
	double f1 = factor;

	double fac = first_intermediate;
	f0 *= fac;
	f1 *= (1.0 / fac);

	i0->scale_channels(f0);
	i1->scale_channels(f1);

	l->lambda = target_lambda;
	l->delta = target_lambda;
    }
}


void Brdf_Map::instantiateParameterizer(Brdf_Param bp1, Brdf_Param bp2) {
    Param *param;

    switch (bp1) {
      case tin_pin:
	param = new Param__Theta_Phi_In__Theta_Phi_Out();
	break;
      case th_ph_gs:
	param = new Param__Theta_Phi_In__Theta_Phi_Out();
	break;
      case xh_yh:
	param = new Param__X_Y_Half__X_Y_Diff();
	break;
      case gl_sphere_in:
	param = new Param__Theta_Phi_In__Theta_Phi_Out__Spheremap();
	break;
      default:
	assert(0);
	break;
    }

    parameterizer = param;

    /*
    else if( bp1 == gl_sphere_in ) {
	Brdf_Parameterizer = new Brdf_ParamGLSphere( Brdf_Parameterizer::in );
    }
    else if( bp1 == gl_sphere_out ) {
	Brdf_Parameterizer = new Brdf_ParamGLSphere( Brdf_Parameterizer::out );
    }
    else if( bp1 == angle_sphere_in ) {
	Brdf_Parameterizer = new Brdf_ParamAngleSphere( Brdf_Parameterizer::in );
    }
    else if( bp1 == angle_sphere_out ) {
	Brdf_Parameterizer = new Brdf_ParamAngleSphere( Brdf_Parameterizer::out );
    }
    else if( bp1 == angle_sphere_tph ) {
	Brdf_Parameterizer = new Brdf_ParamHalfDiff();
    }
    else if( bp1 == angle_sphere_tph_a ) {
	Brdf_Parameterizer = new Brdf_ParamHalfDiffA();
    }
    else if( bp1 == th_ph ) {
	Brdf_Parameterizer = new Brdf_Parameterizer();
	// Just the standard empty param., because it needs to
	// be converted into something useful (angle)!
    }
    else if( bp1 == th_ph_a ) {
	Brdf_Parameterizer = new Brdf_Parameterizer();
	// Just the standard empty param., because it needs to
	// be converted into something useful (angle)!
    }
    else if( bp1 == th_ph_gs || bp1 == td_pd_gs ) {
	Brdf_Parameterizer = new Brdf_Parameterizer();
	// Just the standard empty param., because it needs to
	// be converted into something useful (GLSphere (i.e. xy-param))!
    }
    else if( bp1 == pin_pout ) {
	Brdf_Parameterizer = new Brdf_ParamTTPP();
    }
    else if( bp1 == d2in_d2out ) {
	Brdf_Parameterizer = new Brdf_ParamD1D2();
    }
    else if( bp1 == d2h_d2d ) {
	Brdf_Parameterizer = new Brdf_ParamHDD1D2();
    }
    else {
	cerr << "Unknown Parametrization in Brdf_Map.C::instantiateP(): " << bp1 << endl;
    }
    */
    return;
}

bool Brdf_Map::isUnparametrized() {
    return ((Brdf_ParamL1 == tin_pin) || 
	    (Brdf_ParamL1 == tout_pout) ||
	    // (Brdf_ParamL1 == tin_tout) ||
	    // (Brdf_ParamL1 == pin_pout) ||
	    (Brdf_ParamL1 == td_pd_a) ||
	    (Brdf_ParamL1 == th_ph_a) ||
	    (Brdf_ParamL1 == td_pd_gs) ||
	    (Brdf_ParamL1 == th_ph_gs) ||
	    (Brdf_ParamL1 == td_pd) ||
	    (Brdf_ParamL1 == th_ph)
	    );
}

/**
 * Calculate the BRDF at the specified position (position already
 * in the BRDF parametrization format). It sums up all the textures
 * at the specified position - and possibly subtracts the chunk-textures,
 * if Brdf_Map is biased.
 * @in map and position
 * @in nr of Layers to be used (makes sense only for unbiased Brdf_Maps)
 * @out r, g, b
 */
void Brdf_Map::calculateBRDFValue( Double first_x, Double first_y,
				  Double second_x, Double second_y,
				  Double &r, Double &g, Double &b,
				  int nrLayers ) {
    Double r1,r2,g1,g2,b1,b2;
    r = g = b = 0.0;

    if( nrLayers == 0 || nrLayers > getNumberOfLayers() )
	nrLayers = getNumberOfLayers();

    for( int l = 0; l < nrLayers; l++ ) {
	Brdf_Image *image1 = get_image(l, Brdf_Layer::TEXTURE1);
	Brdf_Image *image2 = get_image(l, Brdf_Layer::TEXTURE2);

	image1->getInterpolatedPixel(first_x,first_y,r1,g1,b1);
	image2->getInterpolatedPixel(second_x,second_y,r2,g2,b2);

	double lambda = get_layer(l)->getLambda();
	r += lambda * r1 * r2;
	g += lambda * g1 * g2;
	b += lambda * b1 * b2;
    }

    return;
}
