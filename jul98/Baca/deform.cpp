#include "dfutil.h"

#include <GL/glut.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
//deform.cpp
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

//-----------------------------------------------------------------------------
//This program demonstrates the poor man's skinning technique.
//The demo displays a polygonal ball in which the top quarter and the
//bottom quarter extrude out of the ball and then begin to twist and
//bend.
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

//This function creates a 3D ball made of triangles.
extern void makeBall( int nLat,
                      int nLong,
                      double radius,
                      Vector** paVtx,
                      Triangle** paTri,
                      Vector** paNorm,
                      int* pnVert,
                      int* pnTri );

//This function tests a vertex against an array of planes.
//Returns true if the vertex is in front of every plane.
extern bool isInside( const Vector vtx,
                      const Plane aBP[],
                      const int nBP );

//This function transforms vertices, performs backface culling, and
//renders the triangles of a mesh.
void render( unsigned short* const vGroups[],
             const int vgCount[],
             Matrix vGroupPos[],
             const int nGroups,
             const Vector aOrigVtx[],
             Vector aDeformedVtx[],
             Vector aDeformedNorm[],
             const Triangle aTri[],
             const int nTriangles );

static Vector* _aOrigBallVtx = 0;       //Un-transformed vertices
static Vector* _aOrigBallNorm = 0;      //Un-transformed normals
static Vector* _aDeformedBallVtx = 0;   //Transformed vertices
static Vector* _aDeformedBallNorm = 0;  //Transformed normals
static Triangle* _aBallTri = 0;         //Triangles

static const int _nLat = 40;            //Latitudinal sections in ball
static const int _nLong = 40;           //Longitudinal sections in ball
static const double _ballRadius = 1;    //Radius of ball
static int _nVertices = 0;              //Number of vertices in ball
static int _nTriangles = 0;             //Number of triangles in ball

static const int _nGroups = 3;                  //Number of vertex groups
static unsigned short* _vGroups[ _nGroups ];    //Vertex indices (the groups)
static int _vgCount[ _nGroups ];                //Number of verts in ea. group
static Matrix _vGroupPos[ _nGroups ];           //Positions of each group

//Define some bounding volumes

static const int _nPlanesInVolume = 6;      //Number of planes in ea. volume

//Planes defining bounding volumes
static Plane _aaPlanes[ _nGroups ][ _nPlanesInVolume ] =
{
    //This volume contains the top of the ball.
    {
        { { 1, 0, 0 }, 1 },
        { { -1, 0, 0 }, 1 },
        { { 0, 1, 0 }, 1 },
        { { 0, -1, 0 }, -0.5f },
        { { 0, 0, 1 }, 1 },
        { { 0, 0, -1 }, 1 }
    },

    //This volume contains the center of the ball.
    {
        { { 1, 0, 0 }, 1 },
        { { -1, 0, 0 }, 1 },
        { { 0, 1, 0 }, 0.5f },
        { { 0, -1, 0 }, 0.5f },
        { { 0, 0, 1 }, 1 },
        { { 0, 0, -1 }, 1 }
    },

    //This volume contains the bottom quarter of the ball.
    {
        { { 1, 0, 0 }, 1 },
        { { -1, 0, 0 }, 1 },
        { { 0, 1, 0 }, -0.5f },
        { { 0, -1, 0 }, 1 },
        { { 0, 0, 1 }, 1 },
        { { 0, 0, -1 }, 1 }
    }
};

//The bounding volumes
static BoundingVolume _aBVolumes[ _nGroups ];

static void buildVertexGroups( unsigned short* aVGroups[],
                               int aVGCount[],
                               const Vector* aVtx,
                               const int nVtx,
                               const BoundingVolume aBVolumes[],
                               const int nGroups ) 
{
    //This array keeps track of which vertices have been bound to
    //a vertex group.  This ensures that each vertex is included in
    //at most one vertex group.
    char* aMarks = new char[ nVtx ];
    memset( aMarks, 0, sizeof( char ) * nVtx );

    for( int gIdx = nGroups - 1; gIdx >= 0; gIdx-- )
    {
        int vIdx;

        //Count the number of vertices that fall within the bounding
        //volume.
        aVGCount[ gIdx ] = 0;
        for( vIdx = nVtx - 1; vIdx >= 0; vIdx-- )
        {
            //If the vertex is not marked and it's contained within
            //the bounding volume then count it.
            if( !aMarks[ vIdx ] &&
                isInside( aVtx[ vIdx ],
                          aBVolumes[ gIdx ].aPlanes,
                          aBVolumes[ gIdx ].nPlanes ) )
            {
                ++aVGCount[ gIdx ];
            }
        }

        //Allocate an array to hold vertex indices.
        aVGroups[ gIdx ] = new unsigned short[ aVGCount[ gIdx ] ];

        //Store the index of each vertex that falls inside the
        //bounding volume.
        int vgIdx = 0;
        for( vIdx = nVtx - 1; vIdx >= 0; vIdx-- )
        {
            //If the vertex is not marked and it's contained within
            //the bounding volume then include it in the vertex group.
            if( !aMarks[ vIdx ] &&
                isInside( aVtx[ vIdx ],
                          aBVolumes[ gIdx ].aPlanes,
                          aBVolumes[ gIdx ].nPlanes ) )
            {
                //Mark the vertex so it is not included in more than
                //one group.
                aMarks[ vIdx ] = 1;
                aVGroups[ gIdx ][ vgIdx++ ] = vIdx;
            }
        }
    }

    delete [] aMarks;
}

//Forward declaration
static void idle();

static void initDemo()
{
    //Set up a light.
    GLfloat light_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_position[] = { 2.0f, 2.0f, 1.0f, 0.0f };

    glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    glLightfv( GL_LIGHT0, GL_AMBIENT, light_ambient );
   
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glEnable( GL_DEPTH_TEST );

    //Set up the view frustum
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( 1, -1, -1, 1, 1.0, 10.0);

    //Initialize the modelview matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    //We will use vertex arrays to render the triangles.
    glEnableClientState( GL_VERTEX_ARRAY );

    //Create a 3D ball made of triangles.  the _aOrig*
    //arrays hold the original vertices and normals.
    //The _aDeformed* arrays will be used to hold the
    //deformed vertices and normals.
    makeBall( _nLat,
              _nLong,
              _ballRadius,
              &_aOrigBallVtx,
              &_aBallTri,
              &_aOrigBallNorm,
              &_nVertices,
              &_nTriangles );

    _aDeformedBallVtx = new Vector[ _nVertices ];
    _aDeformedBallNorm = new Vector[ _nTriangles ];

    //Initialize the bounding volumes.
    for( int i = _nGroups - 1; i >= 0; i-- )
    {
        _aBVolumes[ i ].aPlanes = _aaPlanes[ i ];
        _aBVolumes[ i ].nPlanes = _nPlanesInVolume;
    }

    //Create vertex groups by determining which volumes
    //contain the vertices.
    buildVertexGroups( _vGroups,
                       _vgCount,
                       _aOrigBallVtx,
                       _nVertices,
                       _aBVolumes,
                       _nGroups );

    //Execute one idle to make sure everything is set up properly.
    idle();
}

static void moveGroup0()
{
    static float angle = 0;
    static float trans = 0;
    static bool incr = true;
    const float kMaxAngle = 45;
    const float kMaxTrans = 1;

    //Let OpenGL do our transforms for us.
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0, trans, 0 );
    glTranslatef( 0, 0, -4 );
    glRotatef( angle, 0, 1, 0 );
    glRotatef( angle, 0, 0, 1 );
    glGetFloatv( GL_MODELVIEW_MATRIX, ( GLfloat* ) _vGroupPos[ 0 ] );
    glPopMatrix();

    //Translate up (vertically) until we reach kMaxTrans.  After
    //reaching kMaxTrans, rotate left to right, right to left.
    if( trans < kMaxTrans )
    {
        trans += 0.01f;
    }
    else if( incr )
    {
        angle += 2;
        if( angle > kMaxAngle )
        {
            incr = false;
        }
    }
    else
    {
        angle -= 2;
        if( angle < -kMaxAngle )
        {
            incr = true;
        }
    }
}

static void moveGroup1()
{
    //Let OpenGL do our transforms for us.
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0, 0, -4 );
    glGetFloatv( GL_MODELVIEW_MATRIX, ( GLfloat* ) _vGroupPos[ 1 ] );
    glPopMatrix();
}

static void moveGroup2()
{
    static float angle = 0;
    static float trans = 0;
    static bool incr = true;
    const float kMaxAngle = 45;
    const float kMaxTrans = 1;

    //Let OpenGL do our transforms for us.
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0, -trans, 0 );
    glTranslatef( 0, 0, -4 );
    glRotatef( angle, 0, 1, 0 );
    glRotatef( -angle, 0, 0, 1 );
    glGetFloatv( GL_MODELVIEW_MATRIX, ( GLfloat* ) _vGroupPos[ 2 ] );
    glPopMatrix();

    //Translate down (vertically) until we reach kMaxTrans.  After
    //reaching kMaxTrans, rotate left to right, right to left.
    if( trans < kMaxTrans )
    {
        trans += 0.01f;
    }
    else if( incr )
    {
        angle += 2;
        if( angle > kMaxAngle )
        {
            incr = false;
        }
    }
    else
    {
        angle -= 2;
        if( angle < -kMaxAngle )
        {
            incr = true;
        }
    }
}

static void display()
{
    //Render the deformed mesh.
    render( _vGroups,
            _vgCount,
            _vGroupPos,
            _nGroups,
            _aOrigBallVtx,
            _aDeformedBallVtx,
            _aDeformedBallNorm,
            _aBallTri,
            _nTriangles );
}

static void reshape( int w, int h )
{
    glViewport( 0, 0, w, h );
}

static void keyboard( unsigned char key, int x, int y )
{
    switch( key )
    {
    case 27:        //Escape
        exit(0);
        break;
    }
}

static void idle()
{
    //Update the positions of each vertex group.
    moveGroup0();
    moveGroup1();
    moveGroup2();

    glutPostRedisplay();
}

static void freeMem()
{
    //Free all the memory we allocated.

    delete [] _aOrigBallVtx;
    _aOrigBallVtx = 0;

    delete [] _aDeformedBallVtx;
    _aDeformedBallVtx = 0;

    delete [] _aBallTri;
    _aBallTri = 0;

    delete [] _aOrigBallNorm;
    _aOrigBallNorm = 0;

    delete [] _aDeformedBallNorm;
    _aDeformedBallNorm = 0;

    for( int i = _nGroups - 1; i >= 0; i-- )
    {
        delete [] _vGroups[ i ];
        _vGroups[ i ] = 0;
    }
}

int main( int argc, char* argv[] )
{
    //This ensures the freeMem() function gets called when we
    //exit the program.
    atexit( freeMem );

    //Initialize the GLUT library.
    glutInit( &argc, argv );

    //Set up the OpenGL rendering context.
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );

    //Create a window and set its width and height.
    glutCreateWindow( "Deform" );
    glutReshapeWindow( 640, 480 );

    //Initialize our data structures.
    initDemo();

    //The keyboard function gets called whenever we hit a key.
    glutKeyboardFunc( keyboard );

    //The display function gets called whenever the window
    //requires an update or when we explicitly call
    //glutPostRedisplay()
    glutDisplayFunc( display );

    //The reshape function gets called whenever the window changes
    //shape.
    glutReshapeFunc( reshape );

    //The idle function gets called when there are no window
    //events to process.
    glutIdleFunc( idle );

    //Get the ball rolling!
    glutMainLoop();

    return 0;
}

//eof