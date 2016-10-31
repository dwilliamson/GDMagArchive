#ifndef BEZIERPATCH_H
#define BEZIERPATCH_H

#include <curves/bezier/BezierTypedefs.h>
#include <curves/bezier/BezierBasis.h>
#include <curves/bezier/BezierPatchTessellator.h>
#include <curves/bezier/UniformPatchTessellator.h>
#include <math/Vector.h>
#include <vector>
#include <gl/glut.h>

class CurvePoint;

class BezierPatch
{
public:
  BezierPatch();
  ~BezierPatch();

  // No copy constructor or operator= because we've nothing to deep copy.

  // These add and remove control points from the end of the curve.
  void setControlPoints( const std::vector< std::vector< CurvePoint > >& );
  void clearControlPoints();

  // This evaluates the patch at a single point the "slow" way (no central differences, 
  // just explicit calculation.
  float getValueAt(float x, float y);
  Vector getNormalAt(float x, float y);
  Vector getGradientAt(float x, float y);
  void getLightmapAt(float x, float y, unsigned int& texName, float& u, float& v);

  // These are used to modify the points without actually changing their number.
  int getNumPointsU() const;
  int getNumPointsV() const;
  CurvePoint* getControlPoint( int u, int v );
  const CurvePoint* getControlPoint( int u, int v ) const;

  // To get a feel for how fast this'd be if we were bending the curve every frame, we
  // regenerate our curve every frame.
  void tessellate(const BezierPatchTessellator* drawer) const;
  void draw(const BezierPatchTessellator* drawer) const;

  void drawBasisFunctions() const;

  // This just asks the BezierPatch to pass its bases and control points to the tessellator
  // who then makes a lightmap out of it with OpenGL's feedback mode.
  void generateLightmap( const UniformPatchTessellator*, int size );
  void setTexture( unsigned int newVal );

  unsigned int getLightmap() const;
  unsigned int getTexture() const;

  // These can be toggled to view the model unlit or untextured.
  void setUseLightmap( bool newVal );
  void setUseTexture( bool newVal );

  bool getUseLightmap() const;
  bool getUseTexture() const;

  static void getVectorInGradientPlane(const Vector& grad, Vector& xyNeedsZ);

protected:
  // This is used to generate the basis functions.  It's called whenever setControlPoints is called..
  void generateBases();

  std::vector< std::vector< CurvePoint > > controls;
  std::vector< BasisFunction > basesU;
  std::vector< BasisFunction > basesV;

  unsigned int textureName;
  unsigned int lightmapName;

  bool useTexture;
  bool useLightmap;
};

#endif //BEZIERPATCH_H