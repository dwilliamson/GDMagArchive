#ifndef UNIFORMCURVETESSELLATOR_H
#define UNIFORMCURVETESSELLATOR_H

#include <curves/bezier/BezierCurveTessellator.h>

class UniformCurveTessellator : public BezierCurveTessellator
{
public:
  UniformCurveTessellator();
  UniformCurveTessellator( const UniformCurveTessellator& );
  UniformCurveTessellator& operator=( const UniformCurveTessellator& );

  virtual void tessellate( const std::vector< CurvePoint >&, 
                           const std::vector< BasisFunction >& ) const;
protected:
};

#endif //UNIFORMCURVETESSELLATOR_H