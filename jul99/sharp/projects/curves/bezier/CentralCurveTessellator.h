#ifndef CENTRALCURVETESSELLATOR_H
#define CENTRALCURVETESSELLATOR_H

#include <curves/bezier/BezierCurveTessellator.h>

#include <list>

#pragma warning ( disable: 4786 )

typedef std::pair< CurvePoint, CurvePoint > CentralPoint;
typedef std::list< CentralPoint >::const_iterator curveIter;

class CentralCurveTessellator : public BezierCurveTessellator
{
public:
  CentralCurveTessellator();
  CentralCurveTessellator( const CentralCurveTessellator& );
  CentralCurveTessellator& operator=( const CentralCurveTessellator& );

  virtual void tessellate( const std::vector< CurvePoint >&, 
                           const std::vector< BasisFunction >& ) const;

  virtual void setGranularity( float newVal );

protected:
  void generateMidpoint( curveIter& left, curveIter& right, float du ) const;

  // Note that this is a pair because central-difference calculations need
  // the value of f(u) and f''(u) at the endpoints, so we have to store two
  // points -- one's f(u), and one's f''(u).
  mutable std::list< std::pair< CurvePoint, CurvePoint > > vertices;

  float threshold;
};

#endif //CENTRALCURVETESSELLATOR_H