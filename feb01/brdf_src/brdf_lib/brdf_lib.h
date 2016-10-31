#include "Global.hh"

extern void setClamping( bool val );
extern Double gaussian( Double th_i, Double phi_i, Double th_r, Double phi_r );
extern Double phong( Double th_i, Double phi_i, Double th_o, Double phi_o );
extern Double cylinder( Double theta_in, Double phi_in, 
			Double theta_out, Double phi_out );

extern Double phongBRDF( Double th_i, Double phi_i, Double th_o, Double phi_o,
			 Double diffuse, Double specular, Double n );

extern Double nonLinearPhongBRDF( Double th_i, Double phi_i, 
				  Double th_o, Double phi_o,
				  Double Cx, Double Cy, Double Cz,
				  Double diffuse, Double specular, Double n );

extern Double gaussianBRDF( Double th_i, Double phi_i, 
			    Double th_r, Double phi_r,
			    Double f_d, Double f_s, Double ax, Double ay );

extern Double cylinderBRDF( Double theta_in, Double phi_in,
			    Double theta_out, Double phi_out,
			    Double d, Double h, Double n,
			    Double rs, Double rd );

extern bool curetInit( char *name );
extern Double curet( Double theta_in, Double phi_in,
		     Double theta_out, Double phi_out );

extern bool wardInit( char *name );
extern Double ward( Double theta_in, Double phi_in,
		    Double theta_out, Double phi_out );

extern bool curetColorInit();
extern void curetColor( Double theta_in, Double phi_in,
		 Double theta_out, Double phi_out,
		 Double *r, Double *g, Double *b );

bool phongApproxInit( char *filename );
Double phongApproxBRDF( Double theta_in, Double phi_in,
			Double theta_out, Double phi_out );

bool shapeApproxInit( char *filename );
Double shapeApproxBRDF( Double theta_in, Double phi_in,
			Double theta_out, Double phi_out );

bool shape2DApproxInit( char *filename );
Double shape2DApproxBRDF( Double theta_in, Double phi_in,
			Double theta_out, Double phi_out );

Cardinal SAgetNumberOfLobes();
void SAsetNumberOfLobes( Cardinal nr );
