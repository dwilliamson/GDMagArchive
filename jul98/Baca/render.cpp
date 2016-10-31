#include "dfutil.h"

#include <GL/glut.h>

//-----------------------------------------------------------------------------
//render.cpp
//
//This function transforms the vertices contained in each vertex
//group by the appropriate transform.  It then culls away back
//facing polygons and renders only those that face the viewer.
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

void render( unsigned short* const vGroups[],   //Groups of vertex indices
             const int vgCount[],               //Count of vertices in ea. grp.
             Matrix vGroupPos [],               //Group transforms
             const int nGroups,                 //Number of groups
             const Vector aOrigVtx[],           //Un-transformed vertices
             Vector aDeformedVtx[],             //Transformed vertices
             Vector aDeformedNorm[],            //re-computed normals
             const Triangle aTri[],             //Triangles
             const int nTriangles )             //number of triangles
{
    //For each bounding volume transform the vertices contained
    //within it into camera space.
    for( int gIdx = nGroups - 1; gIdx >= 0; gIdx-- )
    {
        for( int vIdx = vgCount[ gIdx ] - 1; vIdx >= 0; vIdx-- )
        {
            mMultV( vGroupPos[ gIdx ],
                    aOrigVtx[ vGroups[ gIdx ][ vIdx ] ],
                    aDeformedVtx[ vGroups[ gIdx ][ vIdx ] ] );
        }
    }

    //Re-calculate the normal vectors for each triangle.
    for( int tIdx = nTriangles - 1; tIdx >= 0; tIdx-- )
    {
        makeNormal( aDeformedNorm[ tIdx ],
                    aTri[ tIdx ],
                    aDeformedVtx );
    }

    //Draw the triangles

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glVertexPointer( 3, GL_FLOAT, sizeof( Vector ), aDeformedVtx );

    for( int i = nTriangles - 1; i >= 0; i-- )
    {
        //Use dot product to cull away backfaces.
        if( 0 > vDot( aDeformedNorm[ i ],
                      aDeformedVtx[ aTri[ i ][ 0 ] ] ) )
        {
            glNormal3fv( aDeformedNorm[ i ] );
            glDrawElements( GL_TRIANGLES,
                            3,
                            GL_UNSIGNED_SHORT,
                            &aTri[ i ] );
        }
    }

    glFlush ();

    glutSwapBuffers();
}

//eof