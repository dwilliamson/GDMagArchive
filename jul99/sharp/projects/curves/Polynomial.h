#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>

// This is templated so you can get exact precision by using ints if you know you don't need
// floats.  (Coincidentally, that's exactly the reason I changed it to a template).
template < class T > 
class Polynomial
{
public:
  Polynomial();
  // No copy constructor, operator=, or destructor because we've nothing to deep copy or clean up.

  // This evaluates this polynomial at the specified value of t.
  virtual double operator()( double t ) const;

  // These are various mathematical operators specified to make function arithmetic easier.

  // Multiplication of two polynomials.
  Polynomial< T >  operator* ( const Polynomial< T >& ) const;
  Polynomial< T >& operator*=( const Polynomial< T >& );

  // Multiplication by a scalar.
  Polynomial< T >  operator* ( const T& ) const;
  Polynomial< T >& operator*=( const T& );

  // Addition of two polynomials.
  Polynomial< T >  operator+ ( const Polynomial< T >& ) const;
  Polynomial< T >& operator+=( const Polynomial< T >& );

  // Subtraction of two polynomials.
  Polynomial< T >  operator- ( const Polynomial< T >& ) const;
  Polynomial< T >& operator-=( const Polynomial< T >& );

  // Negation of a polynomial (negates all its coefficients).
  Polynomial< T >  operator- () const;

  // This simply sets the polynomial to the one implied by these coefficients.  Coefficient 0 is the
  // t^0 term, coefficient 1 is the t^1 term, etc.
  void setCoefficients( const std::vector< T >& newCoeffs );
  const std::vector< T >& getCoefficients() const;

  // This returns the degree of the polynomial.
  int degree() const;

  // This differentiates this polynomial with respect to t and returns *this
  Polynomial< T >& differentiate();

protected:
  std::vector< T > coefficients;  
};

// Yuck: the ugliness of templates.
#include <curves/Polynomial.cpp>

#endif //POLYNOMIAL_H