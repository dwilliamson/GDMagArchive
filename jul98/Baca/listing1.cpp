#include "dfutil.h"

//-----------------------------------------------------------------------------
//listing1.cpp
//
//This determines if a vertex falls inside a bounding volume.
//The volume is defined by an array of planes.  The planes should
//enclose a convex space and their normal vectors should face
//outward from the enclosed space.
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

//----------------------------------------------------------
//This function returns true if vtx lies with the
//bounding volume specified by the array of planes
//in aBP.
//----------------------------------------------------------
bool isInside( const Vector vtx,
               const Plane aBP[],
               const int nBP )
{
    //----------------------------------------------------------
    //For each plane in aBP test vtx to see if it
    //is in front of or behind the plane.
    //----------------------------------------------------------
    for( int i = nBP - 1; i >= 0; i-- )
    {
        //----------------------------------------------------------
        //Compute distance from vtx to the plane.
        //If it is negative then vtx is behind the plane.
        //----------------------------------------------------------

        //----------------------------------------------------------
        //compute the dot product of vtx with the normal
        //vector of the plane.
        //----------------------------------------------------------
        float d = vDot( vtx, aBP[ i ].norm );

        //----------------------------------------------------------
        //Subtract the distance of the plane from the origin.
        //----------------------------------------------------------
        d -= aBP[ i ].dist;

        //----------------------------------------------------------
        //The result is the distance of the vertex from the
        //plane.  If the distance is positive then the vertex
        //is in front of the plane.  If the distance is zero
        //or negative then the vertex is behinde the plane.
        //----------------------------------------------------------
        if( d > 0 )
        {
            return false;
        }
    }

    return true;
}

//eof