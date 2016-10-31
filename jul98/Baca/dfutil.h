#ifndef DFUTIL_H
#define DFUTIL_H

//-----------------------------------------------------------------------------
//dfutil.h
//
//This is the header file for demo program that accompanies
//the Game Developer article on skinning
//
//From: "Poor Man's Skinning", Game Developer Magazine, May 1998
//
//Copyright 1998 Kevin Baca, All Rights Reserved.
//For educational purposes only.
//Please do not republish in electronic or print form without permission
//
//By Kevin Baca
//kbaca@sonyinteractive.com
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h> //for memcpy()

typedef float Vector[ 3 ];
typedef float Matrix[ 4 ][ 4 ];

typedef struct Plane
{
    Vector norm;
    float dist;
}
Plane;

typedef struct BoundingVolume
{
    const Plane* aPlanes;
    int nPlanes;
}
BoundingVolume;

typedef unsigned short Triangle[ 3 ];

inline void vAdd( const Vector v1, const Vector v2, Vector v3 )
{
    v3[ 0 ] = v1[ 0 ] + v2[ 0 ];
    v3[ 1 ] = v1[ 1 ] + v2[ 1 ];
    v3[ 2 ] = v1[ 2 ] + v2[ 2 ];
}

inline void vSub( const Vector v1, const Vector v2, Vector v3 )
{
    v3[ 0 ] = v1[ 0 ] - v2[ 0 ];
    v3[ 1 ] = v1[ 1 ] - v2[ 1 ];
    v3[ 2 ] = v1[ 2 ] - v2[ 2 ];
}

inline float vDot( const Vector v1, const Vector v2 )
{
    return ( ( v1[ 0 ] * v2[ 0 ] ) +
             ( v1[ 1 ] * v2[ 1 ] ) +
             ( v1[ 2 ] * v2[ 2 ] ) );
}

inline void vCross( const Vector v1, const Vector v2, Vector v3 )
{
    v3[ 0 ] = ( v1[ 1 ] * v2[ 2 ] ) -
              ( v1[ 2 ] * v2[ 1 ] );
    v3[ 1 ] = -( ( v1[ 0 ] * v2[ 2 ] ) -
                 ( v1[ 2 ] * v2[ 0 ] ) );
    v3[ 2 ] = ( v1[ 0 ] * v2[ 1 ] ) -
              ( v1[ 1 ] * v2[ 0 ] );
}

inline void vNormalize( Vector v )
{
    double sq = ( v[ 0 ] * v[ 0 ] ) +
                ( v[ 1 ] * v[ 1 ] ) +
                ( v[ 2 ] * v[ 2 ] );

    sq = sqrt( sq );

    v[ 0 ] /= ( float ) sq;
    v[ 1 ] /= ( float ) sq;
    v[ 2 ] /= ( float ) sq;
}

inline void vCopy( Vector dst, const Vector src )
{
    memcpy( dst, src, sizeof( Vector ) );
}

inline void mZero( Matrix m )
{
    memset( m, 0, sizeof( Matrix ) );

    for( int i = 4 - 1; i >= 0; i-- )
    {
        m[ i ][ i ] = 1;
    }
}

inline void mMult( const Matrix m1, const Matrix m2, Matrix m3 )
{
    for( int r = 4 - 1; r >= 0; r-- )
    {
        for( int c = 4 - 1; c >= 0; c-- )
        {
            m3[ c ][ r ] = ( m1[ 0 ][ r ] * m2[ c ][ 0 ] ) +
                           ( m1[ 1 ][ r ] * m2[ c ][ 1 ] ) +
                           ( m1[ 2 ][ r ] * m2[ c ][ 2 ] ) +
                           ( m1[ 3 ][ r ] * m2[ c ][ 3 ] );

        }
    }
}

inline void mMultV( const Matrix m, const Vector v1, Vector v2 )
{
    for( int i = 3 - 1; i >= 0; i-- )
    {
        v2[ i ] = ( m[ 0 ][ i ] * v1[ 0 ] ) +
                  ( m[ 1 ][ i ] * v1[ 1 ] ) +
                  ( m[ 2 ][ i ] * v1[ 2 ] ) +
                  m[ 3 ][ i ];
    }
}

inline void mPointOfView( const Matrix m, Matrix pov )
{
    for( int r = 3 - 1; r >= 0; r-- )
    {
        for( int c = 3 - 1; c >= 0; c-- )
        {
            pov[ c ][ r ] = m[ r ][ c ];

        }
    }

    pov[ 0 ][ 3 ] = pov[ 1 ][ 3 ] = pov[ 2 ][ 3 ] =
        pov[ 3 ][ 0 ] = pov[ 3 ][ 1 ] = pov[ 3 ][ 2 ] = 0;
    pov[ 3 ][ 3 ] = 1;

    Vector tmpV1;

    tmpV1[ 0 ] = -m[ 3 ][ 0 ];
    tmpV1[ 1 ] = -m[ 3 ][ 1 ];
    tmpV1[ 2 ] = -m[ 3 ][ 2 ];

    Vector tmpV2;

    mMultV( pov, tmpV1, tmpV2 );

    vCopy( pov[ 3 ], tmpV2 );
}

inline void makeVertex( Vector vtx, double x, double y, double z )
{
    vtx[ 0 ] = ( float ) x;
    vtx[ 1 ] = ( float ) y;
    vtx[ 2 ] = ( float ) z;
}

inline void makeTriangle( Triangle tri, int v0, int v1, int v2 )
{
    tri[ 0 ] = ( unsigned short ) v0;
    tri[ 1 ] = ( unsigned short ) v1;
    tri[ 2 ] = ( unsigned short ) v2;
}

inline void makeNormal( Vector norm,
                        const Triangle tri,
                        const Vector* aVtx )
{
    const float* v0 = aVtx[ tri[ 0 ] ];
    const float* v1 = aVtx[ tri[ 1 ] ];
    const float* v2 = aVtx[ tri[ 2 ] ];
    Vector vect0;
    Vector vect1;

    vSub( v0, v1, vect0 );
    vSub( v2, v1, vect1 );

    vCross( vect0, vect1, norm );

    vNormalize( norm );
}

#endif //DFUTIL_H

//eof