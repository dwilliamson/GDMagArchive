//
//  Curves and Curved Surfaces: Bezier patch
//
//      Brian Sharp, 9/28/98
//      brian_sharp@cognitoy.com
//
//  This file is the driver program for drawing a Bezier patch.  It supports different
//  kinds of tessellators, such as the (easiest, most straightforward) uniform tessellator
//  and more can easily be added.
//
//  Usage: 
//      - and + increase or decrease the tessellation granularity.
//      v toggles the display of just the patch the patch and its basis functions.
//      f toggles the size of the basis functions -- from an inset window to fullscreen.
//      c toggles the display of control points.
//      [ and ] decrease and increase the number of control points, respectively.
//      r regenerates random points.
//      p toggles between drawing the patch as a line strip or as points.
//

// The OpenGL enums and functions are used in here to some extent.  I include glut to shield myself
// from having to include windows.h
#include <gl/glut.h>

// stdlib.h is used by rand and srand.
#include <stdlib.h>

// This is the patch itself.
#include <curves/bezier/BezierPatch.h>
#include <curves/bezier/UniformPatchTessellator.h>
#include <curves/bezier/CentralPatchTessellator.h>
#include <curves/CurvePoint.h>
#include <curves/GlobalCamera.h>
#include <curves/bezier/CameraMover.h>
#include <curves/OpenGL.h>

// Declares the prototype for loadTexture that we need.
#include <curves/main.h>

#include <iostream>
#include <mmsystem.h>

// This value specifies the number of control points.  The actual number of points is this
// squared, as the patch is numPoints * numPoints in size.
int numPoints;

// This value is the number of steps in the tessellation.  It must be greater than 0.
int totalSteps = 20;

// This specifies whether the control points are drawn.
bool drawControls = false;

// This specifies whether or not to draw the basis functions for the patch.
bool verbose = false;

// This controls whether the basis functions are drawn in an inset window or over the whole screen.
bool basesFullscreen = false;

// This is our patch instance.
BezierPatch bezierPatch;

// This points to our tessellator used to render the patch.
BezierPatchTessellator* tessellator;

// Prototypes: These functions are helper functions for the bezier algorithm.
void genPatch();

// Annoying but necessary: this eliminates the warnings about name-manglings that occur because of the
// use of STL containers.
#pragma warning (disable: 4786)

const char* appWindowName()
{
  return "Bezier Patch";
}

void appCleanup()
{
  delete tessellator;
}

void appInit(int argc, char** argv)
{
  // Get the number of control points.  Default to a bicubic patch if none is given.
  numPoints = 4;
  
  // Now set up extensions we use (multitexturing).
  OpenGL::initExtensions();

  // Setup the patch.
  genPatch();

  // Generate a texture for it.
  int textureName = loadTexture("texture00.raw");
  bezierPatch.setTexture(textureName);

  // Set up our tessellator.
  tessellator = new CentralPatchTessellator();
  tessellator->setGranularity(6.6f);

  // A little OpenGL initialization: we use vertex arrays for simplicity and speed here.
  // We don't use any of the others, though.
  glEnableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_NORMAL_ARRAY );
  glDisableClientState( GL_COLOR_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  glDisableClientState( GL_INDEX_ARRAY );
  glDisableClientState( GL_EDGE_FLAG_ARRAY );
  
  // If we draw the patch as points, we don't want them large, but we do want them visible.
  glPointSize(2);

  // We use z-buffering, so turn it on.
  glEnable( GL_DEPTH_TEST );

  // Also, we'd like this off since you can see either side of the surface.
  glDisable( GL_CULL_FACE );

  // Setup our mover so we can have mouse-driven camera movement.
  CameraMover::initCallbacks();

  // Also, make the mouse invisible since we do mlook-style movement now!
  glutSetCursor(GLUT_CURSOR_NONE);
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch ( keyHit )
  {
    // Decrease the number of control points in the patch.
  case '[':
  case '{':
    numPoints--;
    if (numPoints == 0) numPoints = 1;
    else genPatch();
    break;
    
    // Increase the number of control points in the patch.
  case ']':
  case '}':
    numPoints++;
    genPatch();
    break;
    
    // Toggle whether control points are drawn.
  case 'c':
  case 'C':
    drawControls = !drawControls;
    break;
    
    // Generate another random set of control points.
  case 'r':
  case 'R':
    genPatch();
    break;
    
    // Toggle between drawing the patch as filled, lines, or points.
  case 'p':
  case 'P':
    if ( tessellator->getMode() == GL_FILL )
    {
      tessellator->setMode( GL_LINE );
    }
    else if ( tessellator->getMode() == GL_LINE )
    {
      tessellator->setMode( GL_POINT );
    }
    else
    {
      tessellator->setMode( GL_FILL );
    }
    break;

    // Turn up the tessellation.
  case '+':
  case '=':
    tessellator->setGranularity( tessellator->getGranularity() + 0.1f );
    break;

    // Turn down the tessellation.
  case '-':
  case '_':
    tessellator->setGranularity( tessellator->getGranularity() - 0.1f );
    break;

    // Toggle drawing of basis functions for the patch.
  case 'v':
  case 'V':
    verbose = !verbose;
    break;

  case 'b':
  case 'B':
    basesFullscreen = !basesFullscreen;
    break;

  case 'f':
  case 'F':
    tessellator->setFixCracks( !tessellator->getFixCracks() );
    break;

  case '9':
  case '(':
    tessellator->setMaxDepth( max( tessellator->getMaxDepth() - 1, 0 ) );
    break;

  case '0':
  case ')':
    tessellator->setMaxDepth( tessellator->getMaxDepth() + 1 );
    break;

  case 'l':
  case 'L': 
    bezierPatch.setUseLightmap( !bezierPatch.getUseLightmap() );
    break;

  case 't':
  case 'T':
    bezierPatch.setUseTexture( !bezierPatch.getUseTexture() );
    break;
  }
}

void appDraw()
{
  // Update any mouse movement.
  CameraMover::update();

  // Unlike some of these demos, we use the z-buffer, so we need to clear it.
  ::glClear( GL_DEPTH_BUFFER_BIT );

  // Tell the patch to draw.
  bezierPatch.tessellate(tessellator);
  bezierPatch.draw(tessellator);

  // Okay, now we're just drawing points and lines.
  glDisable( GL_LIGHTING );
  
  float size[ 4 ] = { 0, 4, 0, 4 };
  float offsetX = 0.1f;
  float offsetY = 0.1f;

  if ( basesFullscreen )
  {
    size[ 1 ] = 1;
    size[ 3 ] = 1;
    offsetX = 0;
    offsetY = 0;
  }

  // If we're being verbose, tell it to draw its basis functions.
  if ( verbose )
  {
    ::glMatrixMode( GL_MODELVIEW );
    ::glPushMatrix();
    ::glLoadIdentity();

    ::glMatrixMode( GL_PROJECTION );
    ::glPushMatrix();
    ::glLoadIdentity();
    ::glOrtho( size[ 0 ], size[ 1 ], size[ 2 ], size[ 3 ], -1, 1 );
    ::glTranslatef( offsetX, offsetY, 0.0f );

    // Draw a square border around the basis functions.
    if ( !basesFullscreen )
    {
      ::glColor3f( 1, 0, 0 );
      ::glBegin( GL_LINE_LOOP );
      ::glVertex2f( 0, 0 );
      ::glVertex2f( 1, 0 );
      ::glVertex2f( 1, 1 );
      ::glVertex2f( 0, 1 );
      ::glEnd();
    }

    bezierPatch.drawBasisFunctions();

    // Clean up after ourselves.
    ::glPopMatrix();
    ::glMatrixMode( GL_MODELVIEW );
    ::glPopMatrix();
  }

  // Turn lighting back on.
  glEnable(GL_LIGHTING);
}

void genPatch()
{
  // Empty the patch so we can start over.
  bezierPatch.clearControlPoints();

  // Seed the random number generator to change the points each time we run.
  srand( timeGetTime() );

  std::vector< std::vector< CurvePoint > > pts;

  for ( int x = 0; x < numPoints; x++ )
  {
    pts.push_back( std::vector< CurvePoint >() );
  }
  
  // Get random values for each of the x, y, and z values of each control point.
  for ( int i = 0; i < numPoints; i++ )
  {
    for ( int j = 0; j < numPoints; j++ )
    {
      // We want a reasonable distribution from around -7 to 7.
      float x = ( (float)( i - (float)numPoints * 0.5 ) / (float)numPoints ) * 14;
      float y = ( (float)( j - (float)numPoints * 0.5 ) / (float)numPoints ) * 14;
      float z = 0;

      // Move them around a little (but not enough that they can intrude into other points' space).
//      x += ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 14.0f / (float)numPoints;
//      y += ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 14.0f / (float)numPoints;
      z += ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 14.0f;

      pts[ i ].push_back( CurvePoint( x, y, z ) );
    }
  }

  bezierPatch.setControlPoints( pts );

  UniformPatchTessellator lightmapper;
  long startTime = timeGetTime();
  bezierPatch.generateLightmap( &lightmapper, 4 );
  long endTime = timeGetTime();
  std::cout << "Lightmapping took " << (endTime - startTime) << std::endl;
}
