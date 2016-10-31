#include <curves/Polynomial.h>

//#define max( a, b ) ( (a) > (b) ? (a) : (b) )

template < class T >
Polynomial< T >::Polynomial()
{
}

// This evaluates this polynomial at the specified value of t.
template < class T >
double Polynomial< T >::operator()( double t ) const
{
  // We use Horner's rule for polynomial computation here -- we accumulate our value starting with
  // the highest-degree coefficient and multiply by t each time.  That way, if we're, say,
  //   3t^3 + 4t^2 + 2t + 5
  // this works out to
  // ( ( ( 3 * t ) + 4 ) * t + 2 ) * t + 5, costing only N muls and N adds for an N-degree polynomial.
  double result = 0;
  for ( int coeff = coefficients.size() - 1; coeff > 0; coeff-- )
  {
    result += coefficients[ coeff ];
    result *= t;
  }

  // Now add the constant on (we can't in the loop because we don't want it to get multiplied by t).
  if ( coefficients.size() > 0 )
  {
    result += coefficients[ 0 ];
  }

  return result;

//  float result = 0;
//
//  for ( int coeff = 0; coeff < coefficients.size(); coeff++ )
//  {
//    float thisTerm = coefficients[ coeff ];
//    for ( int power = 0; power < coeff; power++ )
//    {
//      thisTerm *= t;
//    }
//    result += thisTerm;
//  }
//
//  return result;
}

// These are various mathematical operators specified to make function arithmetic easier.

// Multiplication of two polynomials.
template < class T >
Polynomial< T > Polynomial< T >::operator* ( const Polynomial& rhs ) const
{
  std::vector< T > newCoeffs;

  // Set it as big as we'll need for the largest-order product (the product of the two largest-power
  // components of the polynomials we're multiplying.)  Note that we add one because the size is
  // one greater than the degree.
  newCoeffs.resize( degree() + rhs.degree() + 1, 0 );

  for ( int x = 0; x < coefficients.size(); x++ )
  {
    for ( int y = 0; y < rhs.coefficients.size(); y++ )
    {
      newCoeffs[ x + y ] += coefficients[ x ] * rhs.coefficients[ y ];
    }
  }

  Polynomial< T > returnVal;
  returnVal.setCoefficients( newCoeffs );
  return returnVal;
}

template < class T >
Polynomial< T >& Polynomial< T >::operator*=( const Polynomial< T >& rhs )
{
  std::vector< T > newCoeffs;

  // Set it as big as we'll need for the largest-order product (the product of the two largest-power
  // components of the polynomials we're multiplying.)  Note that we add one because the size is
  // one greater than the degree.
  newCoeffs.resize( degree() + rhs.degree() + 1, 0 );

  for ( int x = 0; x < coefficients.size(); x++ )
  {
    for ( int y = 0; y < rhs.coefficients.size(); y++ )
    {
      newCoeffs[ x + y ] += coefficients[ x ] * rhs.coefficients[ y ];
    }
  }

  setCoefficients( newCoeffs );
  return *this;
}

// Multiplication by a scalar.
template < class T >
Polynomial< T >  Polynomial< T >::operator* ( const T& rhs ) const
{
  std::vector< T > newCoeffs;

  for ( int x = 0; x < coefficients.size(); x++ )
  {
    newCoeffs.push_back( coefficients[ x ] * rhs );
  }

  Polynomial< T > returnVal;
  returnVal.setCoefficients( newCoeffs );
  return returnVal;
}

template < class T >
Polynomial< T >& Polynomial< T >::operator*=( const T& rhs )
{
  for ( int x = 0; x < coefficients.size(); x++ )
  {
    coefficients[ x ] *= rhs;
  }

  return *this;
}

// Addition of two polynomials.
template < class T >
Polynomial< T > Polynomial< T >::operator+ ( const Polynomial< T >& rhs ) const
{
  std::vector< T > newCoeffs;

  int numCoeffs = max( coefficients.size(), rhs.coefficients.size() );
  newCoeffs.resize( numCoeffs, 0 );

  for ( int x = 0; x < coefficients.size(); x++ )
  {
    newCoeffs[ x ] = coefficients[ x ];
  }

  for ( x = 0; x < rhs.coefficients.size(); x++ )
  {
    newCoeffs[ x ] += rhs.coefficients[ x ];
  }

  Polynomial< T > returnVal;
  returnVal.setCoefficients( newCoeffs );
  return returnVal;
}

template < class T >
Polynomial< T >& Polynomial< T >::operator+=( const Polynomial< T >& rhs )
{
  if ( coefficients.size() < rhs.coefficients.size() )
  {
    coefficients.resize( rhs.coefficients.size(), 0 );
  }

  for ( int x = 0; x < rhs.coefficients.size(); x++ )
  {
    coefficients[ x ] += rhs.coefficients[ x ];
  }

  return *this;
}

// Subtraction of two polynomials.
template < class T >
Polynomial< T > Polynomial< T >::operator- ( const Polynomial< T >& rhs ) const
{
  return (*this + (-rhs) );
}

template < class T >
Polynomial< T >& Polynomial< T >::operator-=( const Polynomial< T >& rhs )
{
  *this += (-rhs);
  return *this;
}

// Negation of a polynomial (negates all its coefficients).
template < class T >
Polynomial< T > Polynomial< T >::operator- () const
{
  return (*this * -1);
}

// This simply sets the polynomial to the one implied by these coefficients.  Coefficient 0 is the
// t^0 term, coefficient 1 is the t^1 term, etc.
template < class T >
void Polynomial< T >::setCoefficients( const std::vector< T >& newCoeffs )
{
  coefficients = newCoeffs;
}

template < class T >
const std::vector< T >& Polynomial< T >::getCoefficients() const
{
  return coefficients;
}

template < class T >
int Polynomial< T >::degree() const
{
  return coefficients.size() - 1;
}

// This returns the polynomial that is the value of this polynomial differentiated with respect
// to the parameter t.
template < class T >
Polynomial< T >& Polynomial< T >::differentiate()
{
  // Safety check: if we're a null polynomial, don't do anything.
  if ( coefficients.size() == 0 )
  {
    return *this;
  }

  // Standard polynomial differentiation: multiply each coefficient by the exponent of its term...
  for ( int exponent = 0; exponent < coefficients.size(); exponent++ )
  {
    coefficients[ exponent ] *= exponent;
  }

  // ... and then drop all the exponents by one.
  coefficients.erase( coefficients.begin() );

  // Now, if we were only a constant term to begin with, we'll be empty.  Fix that by adding a
  // zero-valued constant term.
  if ( coefficients.size() == 0 )
  {
    coefficients.push_back( 0 );
  }

  return *this;
}
