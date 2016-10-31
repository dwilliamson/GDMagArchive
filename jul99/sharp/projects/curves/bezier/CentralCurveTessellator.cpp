#include <curves/bezier/CentralCurveTessellator.h>
#include <curves/bezier/BezierBasis.h>
#include <curves/Polynomial.h>
#include <gl/glut.h>

#include <math.h>

CentralCurveTessellator::CentralCurveTessellator() 
: BezierCurveTessellator()
{
  threshold = 1.0f;
}

CentralCurveTessellator::CentralCurveTessellator( const CentralCurveTessellator& source )
: BezierCurveTessellator( source )
{
  threshold = source.threshold;
}

CentralCurveTessellator& CentralCurveTessellator::operator=( const CentralCurveTessellator& source )
{
  BezierCurveTessellator::operator=( source );
  threshold = source.threshold;

  return *this;
}

// We override this so that we can play with the value they pass us so that it makes sense for us.
// Basically, we need a threshold at which we stop recursing, so we tweak this to taste.
void CentralCurveTessellator::setGranularity( float newVal )
{
  BezierCurveTessellator::setGranularity( newVal );

  // Scale it in by 1/2 centered at 1.0
  newVal -= 1.0f;
  newVal *= 0.5f;
  newVal += 1.0f;

  // Make it exponential so things happen a bit quicker.
  newVal = pow( 20.0f, newVal );

  threshold = 1.0f / newVal;
}

void CentralCurveTessellator::tessellate( const std::vector< CurvePoint >& controls, 
                                     const std::vector< BasisFunction >& bases ) const
{
  // This is a cubic-only central differencing scheme, so it only works for 
  // 4-control-point curves.
  if ( controls.size() != 4 )
  {
    return;
  }

  // Empty out the list of vertices, first of all.
  vertices.clear();

  // Now, find the values of f(0) & f''(0)
  float startDeriv2[ 3 ] = { 0,0,0 };
  float endDeriv2[ 3 ] = { 0,0,0 };

  float startBasisVal;
  float endBasisVal;
  std::vector< BasisFunction > basesDuDu( bases );
  for( int i = 0; i < controls.size(); i++ )
  {
    // Grab the basis in question...
    BasisFunction& basis = basesDuDu[ i ];

    // Take the second derivative of the basis...
    basis.differentiate();
    basis.differentiate();

    // And plug the values in.
    startBasisVal = basis( 0 );
    endBasisVal = basis( 1 );

    startDeriv2[ 0 ] += controls[ i ].getX() * startBasisVal;
    startDeriv2[ 1 ] += controls[ i ].getY() * startBasisVal;
    startDeriv2[ 2 ] += controls[ i ].getZ() * startBasisVal;

    endDeriv2[ 0 ] += controls[ i ].getX() * endBasisVal;
    endDeriv2[ 1 ] += controls[ i ].getY() * endBasisVal;
    endDeriv2[ 2 ] += controls[ i ].getZ() * endBasisVal;
  }

  // Initial conditions for this use half of the second 
  // derivative at the endpoints, hence the mul by 0.5f
  startDeriv2[ 0 ] *= 0.5f;
  startDeriv2[ 1 ] *= 0.5f;
  startDeriv2[ 2 ] *= 0.5f;
  endDeriv2[ 0 ] *= 0.5f;
  endDeriv2[ 1 ] *= 0.5f;
  endDeriv2[ 2 ] *= 0.5f;

  // That's our starting point.
  curveIter startPoint = vertices.insert( vertices.end(), CentralPoint( CurvePoint( startDeriv2[0], startDeriv2[1], startDeriv2[2] ),
                                                                        controls[ 0 ] ) );
  // That's our end point.
  curveIter endPoint = vertices.insert( vertices.end(), CentralPoint( CurvePoint( endDeriv2[0], endDeriv2[1], endDeriv2[2] ),
                                                                      controls[ 3 ] ) );

  // Now, build the list of vertices!  Note that du is only 0.5f, not 1.0f, because it's the distance
  // from the potential midpoint to either of the points we're passing it.
  generateMidpoint( startPoint, endPoint, 0.5f );


  // Start drawing our curve in whatever style they specified.
  ::glBegin( getMode() );

  curveIter iter;
  for ( iter = vertices.begin(); iter != vertices.end(); iter++ )
  {
    // Draw this point.
    ::glVertex3fv( iter->second.getXYZArray() );
  }

  ::glEnd();
}

void CentralCurveTessellator::generateMidpoint( curveIter& left, curveIter& right, float du ) const
{
  // For a quick explanation of this formula, check out Watt & Watt, 
  // Advanced Animation and Rendering Techniques, page 87.  This uses the Taylor
  // polynomial of the curve to do what are called "central differences", which
  // are just generally cool and handy.  They allow us to get most of the speed
  // of forward differencing but also are recursive, not iterative, so we can
  // decide when to stop recursing and thus tessellate dynamically.

  // First, calculate the non-linearity term so we can quickly decide whether to recurse
  // or not.
  float du2 = du * du;

  float nlx = ( left->first.getX() + right->first.getX() ) * 0.5f;
  float nly = ( left->first.getY() + right->first.getY() ) * 0.5f;
  float nlz = ( left->first.getZ() + right->first.getZ() ) * 0.5f;

  // Check to see whether it passes the threshold for recursion.
  if ( ( nlx * nlx + nly * nly + nlz * nlz ) * du2 * du2 < threshold )
  {
    return;
  }

  // Okay, we passed.  Now, calculate the actual point.
  float x = ( left->second.getX() + right->second.getX() ) * 0.5f;
  float y = ( left->second.getY() + right->second.getY() ) * 0.5f;
  float z = ( left->second.getZ() + right->second.getZ() ) * 0.5f;

  x -= nlx * du2;
  y -= nly * du2;
  z -= nlz * du2;

  curveIter middle = vertices.insert( right, CentralPoint( CurvePoint( nlx, nly, nlz ), 
                                                           CurvePoint( x, y, z ) ) );

  // Now, recurse!
  generateMidpoint( left, middle, du * 0.5f );
  generateMidpoint( middle, right, du * 0.5f );
}
