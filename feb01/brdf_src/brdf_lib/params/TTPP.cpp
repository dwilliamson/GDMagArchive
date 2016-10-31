#include "BRDFParamTTPP.hh"
#include "Polygon.hh"

static void calcThetaAndPhi( Double *theta_01, Double *phi_01, 
			     const Vector &v, const Vector &n, 
			     const Vector &f, const Vector &f2 );

BRDFParamTTPP::BRDFParamTTPP() {
    return;
}

BRDFParamTTPP::~BRDFParamTTPP() {
    return;
}

void BRDFParamTTPP::calculatePolygonTexCoords( Polygon *poly, 
				   const Vector &eye, const Vector &light ) {
    Vector v, n, f, f2, ve, vl;

    // Theta-Phi-01-Tex-Generation

    Double theta_01_i, theta_01_o;  // scaled angles to be in range [0,1]
    Double   phi_01_i,   phi_01_o;

    poly->clearBackfacingCounter();

    // Calc all angles from eye and lightsource to each vertex
    for( Cardinal j = 0; j < poly->getNumberOfVertices(); j++ ) {
	v =  poly->getVertex( j );
	n =  poly->getNormal( j );
	f =  poly->getFrame1( j );
	f2 = poly->getFrame2( j );

	// vector from vertex to eye
	ve = eye - v;
	ve.normalize();

	// vector from vertex to light
	vl = light - v;
	vl.normalize();

	// Calc angles from eye to vertex
	// TODO: backface poly's ??

	calcThetaAndPhi( &theta_01_i, &phi_01_i, vl, n, f, f2 );
	calcThetaAndPhi( &theta_01_o, &phi_01_o, ve, n, f, f2 );

	poly->setTexture1TexCoords( j, phi_01_o, phi_01_i );
	poly->setTexture2TexCoords( j, theta_01_o, theta_01_i );
    }

    /* DEBUG 
       printf( "-- j: %d\n", j );
       printf( "ve: %f %f %f\n", ve[0], ve[1], ve[2] );
       printf( "vl: %f %f %f\n", vl[0], vl[1], vl[2] );
       printf( "t_i: %.03f p_i: %.03f\n", theta_01_i, phi_01_i );
       printf( "t_o: %.03f p_o: %.03f\n", theta_01_o, phi_01_o );
    */
}

void 
BRDFParamTTPP::calculateTexCoords( const Vector &eye, const Vector &light,
				       const Vector &v, const Vector &n, 
				       const Vector &f, const Vector &f2,
				       Double &first_x, Double &first_y,
				       Double &second_x, Double &second_y ) {
    Vector ve, vl;
    Double theta_01_i, theta_01_o;
    Double   phi_01_i,   phi_01_o;

    ve = eye - v;
    ve.normalize();
    vl = light - v;
    vl.normalize();
    
    calcThetaAndPhi( &theta_01_i, &phi_01_i, vl, n, f, f2 );
    calcThetaAndPhi( &theta_01_o, &phi_01_o, ve, n, f, f2 );

    first_x = phi_01_o;
    first_y = phi_01_i;
    second_x = theta_01_o;
    second_y = theta_01_i;

    // cout << first_x << " " << first_y << " " << second_x << " " << second_y <<endl;
}

static void calcThetaAndPhi( Double *theta_01, Double *phi_01, 
			     const Vector &v, const Vector &n, 
			     const Vector &f, const Vector &f2 ) {
    const Double epsilon=0.001;
    Double costheta, theta;
    Double cosphi, phi;
    Double cosf2;
    Vector v_p;             // projected v

    // calc theta
    costheta = n | v;
    if( costheta > 1.0-epsilon ) theta = 0.0;
    else theta = acos( costheta );
    *theta_01 = theta / M_PI_2;  // [0,pi/2] -> [0,1]

    // project v onto frame plane
    v_p = f*(v|f) + f2*(v|f2);
    v_p.normalize();

    // <f;v_p> tells us the angle between both, i.e. between 0 and 180!
    // But we want an angle between 0 and 360!
    // -> Check with <f2;v_p>

    cosphi = f  | v_p;
    cosf2  = f2 | v_p;
    if( cosphi > 1.0-epsilon ) phi = 0.0;
    else phi = acos( cosphi );  // 0 - M_PI (180)

    if( cosf2 < 0.0 ) phi = (2.0*M_PI) - phi; // if cos > 90, i.e. <f2;v_p> < 0

    *phi_01 = phi / (2.0*M_PI); // [0,2*pi] -> [0,1]
}
