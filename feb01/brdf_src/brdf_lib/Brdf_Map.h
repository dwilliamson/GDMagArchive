#ifndef BRDF_MAP_H
#define BRDF_MAP_H

#include "global.h"
#include "brdf_layer.h"
#include "brdf_parameterizer.h"

class Brdf_Map {
  public:
    enum Brdf_Param { tin_pin=0, tout_pout=1,  // 1st layer: w_i, 2nd layer: w_o
		     tin_tout=2, pin_pout=3,  // 1st layer: theta's 2nd: phi's.
		     gl_sphere_in=4, gl_sphere_out=5,
		     angle_sphere_in=6, angle_sphere_out=7,
		     delta_theta=8, delta_phi=9,  // later on: ctrl-k that
		     d1in_d2in=10, d1out_d2out=11,
                     d1in_d1out=12, d2in_d2out=13,
		     th_ph=14, td_pd=15,
		     angle_sphere_tph=16, angle_sphere_tpd=17,
		     xd_yd = 18, xh_yh = 19,
		     th_ph_a=20, td_pd_a=21,
		     angle_sphere_tph_a=22, angle_sphere_tpd_a=23,
		     th_ph_gs=24, td_pd_gs=25, 
		      d1h_d1d=26, d2h_d2d=27 };
    
    Brdf_Map(Brdf_Param bp1, Brdf_Param bp2, 
             int width, int height, int layers );
    Brdf_Map(char *);
    Brdf_Map(char *, int NEW_CODE);
    ~Brdf_Map();
    
    // ----- Types (query and set)

    void setParam1( Brdf_Param p ) { Brdf_ParamL1 = p; }
    void setParam2( Brdf_Param p ) { Brdf_ParamL2 = p; }

    Brdf_Param getParam1() { return Brdf_ParamL1; }
    Brdf_Param getParam2() { return Brdf_ParamL2; }
    
    // check if Brdf_Map is still unparam (ie. tp_in/out)
    bool isUnparametrized();
    
    // ----- Query - Misc
	       
    bool isLoaded() const { return (nrLayers != 0); }
    int getNumberOfLayers() const { return nrLayers; }
    Brdf_Layer *get_layer( int l );
    Brdf_Image *get_image(int layer, Brdf_Layer::Brdf_Texture t);
    int getWidth() { return get_image(0,Brdf_Layer::TEXTURE1)->getWidth(); }
    int getHeight() { return get_image(0,Brdf_Layer::TEXTURE1)->getHeight(); }
    Param *getBrdf_Parameterizer() { return parameterizer; }
    const char *getName() { return name; }

    // gamma is only set if biased!
    Double getGamma()     const { return gamma; }

    // ----- BRDF Values

    void calculateBRDFValue( Double first_x, Double first_y,
			     Double second_x, Double second_y,
			     Double &r, Double &g, Double &b, 
			     int nrLayers = 0 );

    // ----- converting the Brdf_Map

    // convert to a gl-like sphere-map
    void convertToGLSphereMap();

    // convert to a angle-preserving sphere-map
    void convertToAngleSphereMap();

    // Convert all values to a proper range [0,1] by biasing all images of the map.
    void makeBiasedMap();

    // Use maximum possible precision in texture maps, by moving parts
    // of Lambda into the textures! (Makes sense only if Brdf_Map will not
    // be biased, as with one-layer-approximations).
    void shuffleLambdasToTextures();

    // Scale Lambdas and Deltas of a OneLayer Brdf_Map
    void scaleLambdasAndDeltas( Double lambda );

    void rescale_for_target_lambda(double target_lambda,
				   double first_intermediate = 0.5);
    bool save( char *filename );

protected:

    Brdf_Param Brdf_ParamL1;  // Parametrization of Layer 1
    Brdf_Param Brdf_ParamL2;  //                 of Layer 2
    int width, height;        // all layers MUST have same size

    Param *parameterizer; // Used to calc tex-coords

    Brdf_Layer **layer;
    int nrLayers;

    Double gamma;

    char name[80];

    void instantiateParameterizer( Brdf_Param bp1, Brdf_Param bp2 );
};

#endif // BRDF_MAP_H
