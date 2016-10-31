#include "BRDFParamAngleSphere.hh"
#include "Polygon.hh"
#include "Helper.hh"

BRDFParamAngleSphere::BRDFParamAngleSphere( BRDFInOut f ) {
    first = f;

    return;
}

BRDFParamAngleSphere::~BRDFParamAngleSphere() {
    return;
}

void BRDFParamAngleSphere::calculatePolygonTexCoords( Polygon *poly, 
			      const Vector &eye, const Vector &light ) {
    Vector v, n, f, f2, ve, vl;

    Vector coordi, coordo;

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

	
	// coord-vectors

	coordi[0] = vl|f;
	coordi[1] = vl|f2;
	coordi[2] = vl|n;

	coordo[0] = ve|f;
	coordo[1] = ve|f2;
	coordo[2] = ve|n;

	// The 2D-Vector (coordi[0],coordi[1]) is normalized
	// and then the length is set to the angle.
	// (range of angle is scaled to [0,1] of course)
	Double norm;
	Double thi = acos(coordi[2]) / M_PI_2;
	Double tho = acos(coordo[2]) / M_PI_2;
	norm = sqrt(coordi[0]*coordi[0]+coordi[1]*coordi[1]);
	if( norm > 0.001 ) {
	    coordi[0] *= (1.0/norm) * thi;
	    coordi[1] *= (1.0/norm) * thi;
	} else {
	    coordi[0] = coordi[1] = 0.0;
	}
	norm = sqrt(coordo[0]*coordo[0]+coordo[1]*coordo[1]);
	if( norm > 0.001 ) {
	    coordo[0] *= (1.0/norm) * tho;
	    coordo[1] *= (1.0/norm) * tho;
	} else {
	    coordo[0] = coordo[1] = 0.0;
	}

	// Check for backfacing polygons
	if( coordi[2] < 0.0 )
	    poly->addBackfacingInVertex();
	if( coordo[2] < 0.0 )
	    poly->addBackfacingOutVertex();

	coordi[0] = coordi[0] / 2.0 + 0.5;
	coordi[1] = coordi[1] / 2.0 + 0.5;
	coordo[0] = coordo[0] / 2.0 + 0.5;
	coordo[1] = coordo[1] / 2.0 + 0.5;

	if( first == in ) {
	    poly->setTexture1TexCoords( j, coordi[0], coordi[1] );
	    poly->setTexture2TexCoords( j, coordo[0], coordo[1] );
	} else {
	    poly->setTexture1TexCoords( j, coordo[0], coordo[1] );
	    poly->setTexture2TexCoords( j, coordi[0], coordi[1] );
	}
    }
}

void 
BRDFParamAngleSphere::calculateTexCoords( 
                                 const Vector &eye, const Vector &light,
				 const Vector &v, const Vector &n, 
				 const Vector &f, const Vector &f2,
				 Double &first_x, Double &first_y,
				 Double &second_x, Double &second_y )
{
    Vector ve, vl;
    Vector coordi, coordo;

    ve = eye - v;
    ve.normalize();
    vl = light - v;
    vl.normalize();

    coordi[0] = vl|f;
    coordi[1] = vl|f2;
    coordi[2] = vl|n;

    coordo[0] = ve|f;
    coordo[1] = ve|f2;
    coordo[2] = ve|n;

    Double norm;
    Double thi = acos(coordi[2]) / M_PI_2;
    Double tho = acos(coordo[2]) / M_PI_2;
    norm = sqrt(coordi[0]*coordi[0]+coordi[1]*coordi[1]);
    if( norm > 0.001 ) {
	coordi[0] *= (1.0/norm) * thi;
	coordi[1] *= (1.0/norm) * thi;
    } else {
	coordi[0] = coordi[1] = 0.0;
    }
    norm = sqrt(coordo[0]*coordo[0]+coordo[1]*coordo[1]);
    if( norm > 0.001 ) {
	coordo[0] *= (1.0/norm) * tho;
	coordo[1] *= (1.0/norm) * tho;
    } else {
	coordi[0] = coordi[1] = 0.0;
    }

    coordi[0] = coordi[0] / 2.0 + 0.5;
    coordi[1] = coordi[1] / 2.0 + 0.5;
    coordo[0] = coordo[0] / 2.0 + 0.5;
    coordo[1] = coordo[1] / 2.0 + 0.5;

    if( first == in ) {
	first_x = coordi[0];
	first_y = coordi[1];
	second_x = coordo[0];
	second_y = coordo[1];
    } else {
	first_x = coordo[0];
	first_y = coordo[1];
	second_x = coordi[0];
	second_y = coordi[1];
    }
}
