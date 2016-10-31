#include "BRDFParamBase.hh"
#include "Polygon.hh"

#include "VisTransform.hh"
#include "Helper.hh"

void BRDFParamBase::calculatePolygonTexCoords( Polygon *poly, 
					       const Vector &eye,
					       const Vector &light ) {
    Vector v, n, f, b, ve, vl;

    Double first_x, first_y;
    Double second_x, second_y;

    poly->clearBackfacingCounter();

    // Calc all angles from eye and lightsource to each vertex
    for( Cardinal j = 0; j < poly->getNumberOfVertices(); j++ ) {
	v = poly->getVertex( j );
	n = poly->getNormal( j );
	f = poly->getFrame1( j );
	b = poly->getFrame2( j );

	// vector from vertex to eye
	ve = eye - v;
	ve.normalize();

	// vector from vertex to light
	vl = light - v;
	vl.normalize();

	// Check for backfacing polygons
	if( (vl|n) < 0.0 )
	    poly->addBackfacingInVertex();
	if( (ve|n) < 0.0 )
	    poly->addBackfacingOutVertex();

	// Now calc the actual coordinates (virtual)
	internalCalcTexCoords( ve, vl, n, f, b, 
			       first_x, first_y,
			       second_x, second_y );

	poly->setTexture1TexCoords( j, first_x, first_y );
	poly->setTexture2TexCoords( j, second_x, second_y );
    }  
}

void 
BRDFParamBase::calculateTexCoords( const Vector &eye, const Vector &light,
				   const Vector &v, const Vector &n, 
				   const Vector &f, const Vector &b,
				   Double &first_x, Double &first_y,
				   Double &second_x, Double &second_y ) {
    Vector ve, vl;
    Vector coordi, coordo;

    ve = eye - v;
    ve.normalize();

    vl = light - v;
    vl.normalize();

    internalCalcTexCoords( ve, vl, n, f, b, 
			   first_x, first_y,
			   second_x, second_y );
}

void 
BRDFParamBase::internalCalcTexCoords( const Vector &ve, const Vector &vl,
				      const Vector &n, 
				      const Vector &f, const Vector &b,
				      Double &first_x, Double &first_y,
				      Double &second_x, Double &second_y ) {
    cerr << "BRDFParamBase is partly abstract!" << endl;
}
