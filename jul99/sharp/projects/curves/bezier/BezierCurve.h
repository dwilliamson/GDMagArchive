#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include <curves/bezier/BezierTypedefs.h>
#include <curves/bezier/BezierBasis.h>
#include <curves/bezier/BezierCurveTessellator.h>
#include <vector>
#include <gl/glut.h>

class CurvePoint;

class BezierCurve
{
public:
  BezierCurve();
  // No copy constructor, operator=, or destructor because we've nothing to deep copy or clean up.

  // These add and remove control points from the end of the curve.  Note that some tessellators
  // may only handle cases where we have 4 control points (cubic bezier curve) so they'll generate
  // nothing if there's anything other than 4 control points.
  void addControlPoint( float x, float y, float z );
  void removeControlPoint();
  void clearControlPoints();

  // To get a feel for how fast this'd be if we were bending the curve every frame, we
  // regenerate our curve every frame.
  void draw( const BezierCurveTessellator*, bool drawControlPoints = false ) const;

  void drawBasisFunctions() const;

protected:
  // This is used to regenerate our basis functions if the number of control points changes.
  void generateBases();

  std::vector< CurvePoint > controls;
  std::vector< BasisFunction > bases;
};

#endif //BEZIERCURVE_H