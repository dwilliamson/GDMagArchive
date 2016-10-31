#ifndef GLOBAL_H
#define GLOBAL_H

const double M_PI = 3.141592654143;
const double M_PI_2 = 0.5 * M_PI; // XXX I think this is right!


#include "types.h"

#define round(x)   int(x+0.5)

template <class T>
inline T MIN( T a, T b ) {
    return (a < b) ? a : b;
}
template <class T>
inline T MAX( T a, T b ) {
    return (a < b) ? b : a;
}
template <class T>
inline T ABS( T a ) {
    return (0 < a) ? a : -a;
}
template <class T>
inline T SIGN( T a ) {
    return (a < 0) ? -1.0 : 1.0;
}
template <class T>
inline T CLAMP( T a, T min, T max ) {
    if( a < min ) return min;
    if( a > max ) return max;

    return a;
}
template <class T>
inline T WRAPAROUND( T a, T min, T max ) {
    if( a < min ) return max-ABS(min-a);
    if( a > max ) return min+ABS(a-max);

    return a;
}

#endif // GLOBAL_H
