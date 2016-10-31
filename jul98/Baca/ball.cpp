#include "dfutil.h"

//-----------------------------------------------------------------------------
//ball.cpp
//
//This function constructs a polygonal ball.
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
static const double _myPi = 3.14159265359;

void makeBall( int nLat,
               int nLong,
               double radius,
               Vector** paVtx,
               Triangle** paTri,
               Vector** paNorm,
               int* pnVert,
               int* pnTri )
{
    const int nVertices = 2 + ( nLong * nLat );
    const int nTriangles = nLat + nLat + ( nLat * 2 * ( nLong - 1 ) );

    *pnVert = nVertices;
    *pnTri = nTriangles;

    Vector* aVtx = new Vector[ nVertices ];
    Vector* aNorm = new Vector[ nTriangles ];
    Triangle* aTri = new Triangle[ nTriangles ];

    *paVtx = aVtx;
    *paNorm = aNorm;
    *paTri = aTri;

    int nVtx = 0;

    makeVertex( aVtx[ nVtx ], 0, 0.5, 0 );

    ++nVtx;

    int i, j;
    double phi, theta;
    const double phiInc = _myPi / ( nLong + 1 );
    const double thetaInc = 2 * _myPi / nLat;

    for( j = 0, phi = phiInc; j < nLong; phi += phiInc, j++ )
    {
        const double r = radius * sin( phi );
        const double y = radius * cos( phi );

        for( i = 0, theta = 0; i < nLat; theta += thetaInc, i++ )
        {
            const double x = r * cos( theta );
            const double z = r * sin( theta );

            makeVertex( aVtx[ nVtx ], x, y, z );

            ++nVtx;
        }
    }

    makeVertex( aVtx[ nVtx ], 0, -0.5, 0 );

    ++nVtx;

    int nTri = 0;
    int vIdx = 1;

    for( i = 0; i < nLat; i++ )
    {
        int lastVIdx;

        if( i < ( nLat - 1 ) )
        {
            lastVIdx = vIdx + 1;
        }
        else
        {
            lastVIdx = vIdx - ( nLat - 1 );
        }

        makeTriangle( aTri[ nTri ],  0, vIdx, lastVIdx );
        makeNormal( aNorm[ nTri ], aTri[ nTri ], aVtx );

        ++nTri;
        ++vIdx;
    }

    vIdx = 1;
    for( j = 0; j < nLong - 1; j++ )
    {
        for( i = 0; i < nLat; i++ )
        {
            int lastVIdx;

            if( i < ( nLat - 1 ) )
            {
                lastVIdx = vIdx + 1 + nLat;
            }
            else
            {
                lastVIdx = vIdx - ( nLat - 1 ) + nLat;
            }

            makeTriangle( aTri[ nTri ],  vIdx, vIdx + nLat, lastVIdx );
            makeNormal( aNorm[ nTri ], aTri[ nTri ], aVtx );

            ++nTri;
            ++vIdx;

            int firstVIdx = ( i < ( nLat - 1 ) ) ? vIdx : vIdx - nLat;
            int secondVIdx = ( i < ( nLat - 1 ) ) ? firstVIdx - 1 :
                                                    firstVIdx + ( nLat - 1 );

            makeTriangle( aTri[ nTri ],  firstVIdx, secondVIdx, firstVIdx + nLat );
            makeNormal( aNorm[ nTri ], aTri[ nTri ], aVtx );

            ++nTri;
        }
    }

    for( i = 0; i < nLat; i++ )
    {
        int lastVIdx;

        if( i < ( nLat - 1 ) )
        {
            lastVIdx = vIdx + 1;
        }
        else
        {
            lastVIdx = vIdx - ( nLat - 1 );
        }

        makeTriangle( aTri[ nTri ],  nVtx - 1, vIdx, lastVIdx );
        makeNormal( aNorm[ nTri ], aTri[ nTri ], aVtx );

        ++nTri;
        ++vIdx;
    }
}

//eof