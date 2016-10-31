#include <curves/bezier/BezierCurve.h>
#include <curves/CurvePoint.h>

#include <iostream>

BezierCurve::BezierCurve()
{
}

void BezierCurve::addControlPoint( float x, float y, float z )
{
  // Add a new control point with those values to the curve.
  CurvePoint c( x, y, z );
  controls.push_back( c );

  // We have to regenerate our basis functions to take this new point into account.
  generateBases();
}

void BezierCurve::removeControlPoint()
{
  // Remove a point from the end of the curve.
  controls.pop_back();

  // We have to regenerate our basis functions to take the loss of this point into account.
  generateBases();
}

void BezierCurve::clearControlPoints()
{
  // Wipe us out (presumably, they'll fill us up again, or else we're not too useful).
  controls.clear();
  bases.clear();
}

// To get a feel for how fast this'd be if we were bending the curve every frame, we
// regenerate our curve every frame.
void BezierCurve::draw( const BezierCurveTessellator* drawer, bool drawControlPoints ) const
{
  float pointColor[3] = {0.0, 1.0, 0.0};
  float curveColor[3] = {1.0, 0.0, 0.0};
  
  // If we're to draw the points, do so now.
  if ( drawControlPoints )
  {
    // Save whatever the size of the points was and set the point
    // size up so the control points are big.
    glPushAttrib(GL_POINT_BIT);
    glPointSize(4);
    
    // Set the color for the points.
    glColor3fv(pointColor);
    

    // Draw the curve's control points.
    ::glBegin( GL_POINTS );
    for ( int x = 0; x < controls.size(); x++ )
    {
      glVertex3f( controls[ x ].getX(), controls[ x ].getY(), controls[ x ].getZ() );
    }
    ::glEnd();

    // Now pop the point size back to what it was.
    glPopAttrib();
  }
  
  // Now set the color to the curve's color and generate the curve, drawing it piece by piece.
  glColor3fv(curveColor);

  // Ask for the tessellator to render us.
  drawer->tessellate( controls, bases );

  // We're done!
  ::glEnd();
}

void BezierCurve::drawBasisFunctions() const
{
  ::glColor3f( 1, 1, 1 );

  for ( int x = 0; x < bases.size(); x++ )
  {
    ::glBegin( GL_LINE_STRIP );
    for ( int step = 0; step <= 80; step++ )
    {
      double t = ((double)step) / 80.0;

      // If you go over 23 control points, accuracy just dies.  This stops you when the error gets
      // bad if you're debugging.
//      if ( bases[ x ]( t ) < -0.001 || bases[ x ]( t ) > 1.001 )
//      {
//        float val = bases[ x ]( t );
//        for ( int j = 0; j < bases[ x ].coefficients.size(); j++ )
//        {
//          std::cout << bases[ x ].coefficients[ j ] << ", ";
//        }
//
//        _asm { int 3h }
//      }

      ::glVertex2f( t, bases[ x ]( t ) );
    }
    ::glEnd();
  }
}

// This is used to regenerate our basis functions if the number of control points changes.
void BezierCurve::generateBases()
{
  // This is straightforward: just generate the bezier bases b(i.n) where i is in the range
  // [0,n], and n is (one less than) the number of control points (one less because it's indexed
  // from 0).
  bases.clear();

  int n = controls.size() - 1;

  for ( int i = 0; i <= n; i++ )
  {
    bases.push_back( BasisFunction( i, n ) );
  }
}