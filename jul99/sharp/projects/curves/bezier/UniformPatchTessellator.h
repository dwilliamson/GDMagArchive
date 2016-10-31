#ifndef UNIFORMPATCHTESSELLATOR_H
#define UNIFORMPATCHTESSELLATOR_H

#include <curves/bezier/BezierPatchTessellator.h>

class UniformPatchTessellator : public BezierPatchTessellator
{
public:
  UniformPatchTessellator();
  UniformPatchTessellator( const UniformPatchTessellator& );
  UniformPatchTessellator& operator=( const UniformPatchTessellator& );
  UniformPatchTessellator::~UniformPatchTessellator();

  virtual void tessellate( const std::vector< std::vector< CurvePoint > >&, 
                           const std::vector< BasisFunction >&, 
                           const std::vector< BasisFunction >& ) const;

  virtual void draw(bool multitexture = false) const;

  int generateLightmap( int size,
                         const std::vector< std::vector< CurvePoint > >&,
                         const std::vector< BasisFunction >&,
                         const std::vector< BasisFunction >& ) const;

protected:
  void generatePoints( int, 
                       const std::vector< std::vector< CurvePoint > >&,
                       const std::vector< BasisFunction >&,
                       const std::vector< BasisFunction >& ) const;

  mutable float* verts;
  mutable float* norms;
};

#endif //UNIFORMPATCHTESSELLATOR_H