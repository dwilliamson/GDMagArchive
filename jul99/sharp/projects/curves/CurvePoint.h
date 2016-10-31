#ifndef CURVEPOINT_H
#define CURVEPOINT_H

#include <memory.h>

class CurvePoint
{
public:
  CurvePoint( float x = 0, float y = 0, float z = 0 );
  CurvePoint( float* point );
  // No copy constructor, operator=, or destructor: we've nothing to deep copy or clean up.

  void setX( float newX );
  void setY( float newY );
  void setZ( float newZ );

  float getX() const;
  float getY() const;
  float getZ() const;

  const float* getXYZArray() const;
  void setXYZArray( const float* point );

  void operator*=(float);
  void operator/=(float);
  void operator+=(const CurvePoint&);
  void operator-=(const CurvePoint&);
  bool operator==(const CurvePoint&) const;

  float magnitude2() const;

protected:
  float xyz[ 3 ];
};

_inline CurvePoint::CurvePoint( float x, float y, float z )
{
  xyz[ 0 ] = x;
  xyz[ 1 ] = y;
  xyz[ 2 ] = z;
}

_inline CurvePoint::CurvePoint( float* point )
{
  memcpy( xyz, point, 3 * sizeof( float ) );
}

_inline void CurvePoint::setX( float newX )
{
  xyz[ 0 ]= newX;
}

_inline void CurvePoint::setY( float newY )
{
  xyz[ 1 ]= newY;
}

_inline void CurvePoint::setZ( float newZ )
{
  xyz[ 2 ]= newZ;
}

_inline float CurvePoint::getX() const
{
  return xyz[ 0 ];
}

_inline float CurvePoint::getY() const
{
  return xyz[ 1 ];
}

_inline float CurvePoint::getZ() const
{
  return xyz[ 2 ];
}

_inline const float* CurvePoint::getXYZArray() const
{
  return xyz;
}

_inline void CurvePoint::setXYZArray( const float* point )
{
  // For three values, this should be faster than a memcpy.
  xyz[0] = point[0];
  xyz[1] = point[1];
  xyz[2] = point[2];
}

_inline void CurvePoint::operator*=(float scalar)
{
  xyz[0] *= scalar;
  xyz[1] *= scalar;
  xyz[2] *= scalar;
}

_inline void CurvePoint::operator/=(float divisor)
{
  float scalar = 1.0f / divisor;
  xyz[0] *= scalar;
  xyz[1] *= scalar;
  xyz[2] *= scalar;
}

_inline void CurvePoint::operator+=(const CurvePoint& cp)
{
  xyz[0] += cp.xyz[0];
  xyz[1] += cp.xyz[1];
  xyz[2] += cp.xyz[2];
}

_inline void CurvePoint::operator-=(const CurvePoint& cp)
{
  xyz[0] -= cp.xyz[0];
  xyz[1] -= cp.xyz[1];
  xyz[2] -= cp.xyz[2];
}

_inline bool CurvePoint::operator==(const CurvePoint& cmp) const
{
  return (xyz[0] == cmp.xyz[0] && xyz[1] == cmp.xyz[1] && xyz[2] == cmp.xyz[2]);
}

_inline float CurvePoint::magnitude2() const
{
  return (xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
}

#endif //CURVEPOINT_H
