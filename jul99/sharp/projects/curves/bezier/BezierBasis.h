#ifndef BEZIERBASIS_H
#define BEZIERBASIS_H

#include <curves/Polynomial.h>

template < class T >
class BezierBasis : public Polynomial< T >
{
public:
  BezierBasis();
  BezierBasis( const BezierBasis& );

  // This evaluates this polynomial at the specified value of t.
  virtual double operator()( double t ) const { return Polynomial< T >::operator()(t); }

  // This creates basis i out of n+1 bases, where i is [0,n] (so BezierBasis(3,5) would mean
  // that this was the 4th basis in a curve with 6 control points.
  BezierBasis( int i, int n );

  // This is used in normal generation.
  BezierBasis< T >& differentiate();

protected:
  // This generates, basically, a row of Pascal's triangle -- a set of binomial coefficients
  // (n choose k for 0 <= k <= n)
  int calculateBinomial( int row, int col );
};

// Yuck: the ugliness of templates
#include <curves/bezier/BezierBasis.cpp>

#endif //BEZIERBASIS_H