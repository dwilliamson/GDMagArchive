#include <math.h>
#include "Hvector.hpp"
#include "Hmatrix4x4.hpp"

float Dot_product(Hvector , Hvector );
Hvector Normalize(Hvector);
Hvector NormalVector(const Hvector &, const Hvector &, const Hvector &);
Hvector Product(const Hvector &, const Hvector &);
Hvector Product(Hmatrix4x4 &, Hvector &);