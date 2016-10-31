#ifndef BEZIERCURVETESSELLATOR_H
#define BEZIERCURVETESSELLATOR_H

#include <curves/CurvePoint.h>
#include <curves/bezier/BezierTypedefs.h>

#include <gl/glut.h>

#include <vector>

// A BezierCurveTessellator is an abstract base class that describes an object capable of taking
// information from a BezierCurve object and generating a bunch of line segments from it.  This is
// done in the tessellate(...) function.
class BezierCurveTessellator
{
public:
  BezierCurveTessellator();
  BezierCurveTessellator( const BezierCurveTessellator& );
  BezierCurveTessellator& operator=( const BezierCurveTessellator& );

  virtual void tessellate( const std::vector< CurvePoint >&, 
                           const std::vector< BasisFunction >& ) const = 0;
  
  virtual void setGranularity( float newVal );
  virtual float getGranularity() const;

  virtual void setMode( GLenum newVal );
  virtual GLenum getMode() const;

protected:
  // This can mean different things to different tessellators, but it's basically a hint about
  // how fine the tessellation should be.
  float granularity;

  // This should be GL_LINE_STRIP or GL_POINTS, it's the mode used to draw the line segments
  // of the curve.
  GLenum curveMode;
};

#endif //BEZIERCURVETESSELLATOR_H