#include <curves/bezier/UniformCurveTessellator.h>
#include <curves/bezier/BezierBasis.h>
#include <curves/Polynomial.h>
#include <gl/glut.h>

#include <math.h>

UniformCurveTessellator::UniformCurveTessellator() 
: BezierCurveTessellator()
{
}

UniformCurveTessellator::UniformCurveTessellator( const UniformCurveTessellator& source )
: BezierCurveTessellator( source )
{
}

UniformCurveTessellator& UniformCurveTessellator::operator=( const UniformCurveTessellator& source )
{
  BezierCurveTessellator::operator=( source );

  return *this;
}

void UniformCurveTessellator::tessellate( const std::vector< CurvePoint >& controls, 
                                     const std::vector< BasisFunction >& bases ) const
{
  // This is done so we use 20 steps by default, but that can be scaled by the granularity.
  int numSteps = 20.0f * getGranularity();

  double invTotalSteps = 1.0f / (numSteps - 1);

  // Start drawing our curve in whatever style they specified.
  ::glBegin( getMode() );

  for ( int step = 0; step < numSteps; step++ )
  {
    // Generate the parameter for this step of the curve.
    float t = step * invTotalSteps;

    // This holds the point we're working on as we add control points' contributions to it.
    float curPt[ 3 ] = { 0, 0, 0 };

    // Generate a point on the curve for this step.
    for ( int pt = 0; pt < controls.size(); pt++ )
    {
      // Get the value of this basis function at the current parameter value.
      float basisVal = bases[ pt ]( t );

      // Add this control point's contribution onto the current point.
      curPt[ 0 ] += controls[ pt ].getX() * basisVal;
      curPt[ 1 ] += controls[ pt ].getY() * basisVal;
      curPt[ 2 ] += controls[ pt ].getZ() * basisVal;
    }

    // Draw this point.
    ::glVertex3fv( curPt );
  }

  ::glEnd();
}
