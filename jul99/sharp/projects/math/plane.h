#ifndef PLANE_H
#define PLANE_H

#include <math/Vector.h>

class Plane 
{
public:
  Plane(const Vector& normal = Vector(), float offset = 0);

  float getA() const;
	float getB() const;
	float getC() const;
	float getD() const;

  void setA(float);
	void setB(float);
	void setC(float);
	void setD(float);

	bool isInside(const Vector&) const;

private:
	Vector abc;
	float d;
};

#endif //PLANE_H