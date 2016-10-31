#ifndef BEZIERPATCHTESSELLATOR_H
#define BEZIERPATCHTESSELLATOR_H

#include <curves/CurvePoint.h>
#include <curves/bezier/BezierTypedefs.h>

#include <gl/glut.h>

#include <vector>

// A BezierPatchTessellator is an abstract base class that describes an object capable of taking
// information from a BezierPatch object and generating a bunch of triangles from it.  This is done
// in the tessellate(...) function.
class BezierPatchTessellator
{
public:
  BezierPatchTessellator();
  BezierPatchTessellator( const BezierPatchTessellator& );
  BezierPatchTessellator& operator=( const BezierPatchTessellator& );

  virtual ~BezierPatchTessellator() { }

  // Used to break the patch into triangles.
  virtual void tessellate( const std::vector< std::vector< CurvePoint > >&, 
                           const std::vector< BasisFunction >&, 
                           const std::vector< BasisFunction >& ) const = 0;

  // Used to draw the patch as it was last tessellated.
  virtual void draw(bool multitexture = false) const = 0;

  
  virtual void setGranularity( float newVal );
  virtual float getGranularity() const;

  virtual void setMode( GLenum newVal );
  virtual GLenum getMode() const;

  virtual void setFixCracks( bool newVal );
  virtual bool getFixCracks() const;

  virtual void setMaxDepth( int newVal ) const;
  virtual int getMaxDepth() const;

protected:
  // This can mean different things to different tessellators, but it's basically a hint about
  // how fine the tessellation should be.
  float granularity;

  // This should be GL_FILL, GL_LINE, or GL_POINT, ala glPolygonMode -- it's the mode used in
  // drawing the polygons from the patch.
  GLenum polyMode;

  // This is used to determine whether cracks produced in tessellation should be fixed or not.
  // If an implementation doesn't produce any cracks, this is pretty irrelevant to it.
  bool fixCracks;

  // This is used to bound the recursion of the patch to some depth so we can limit the
  // number of polygons generated.
  mutable int maxDepth;
};

#endif //BEZIERPATCHTESSELLATOR_H