//
//  Curves and Curved Surfaces: Bezier curve algorithm (non-recursive de Casteljau)
//
//      Brian Sharp, 9/28/98
//      brian_sharp@cognitoy.com
//
//  This file contains functions that generate a curve from randomly generated
//  control points using Bezier's curve algorithm, an adaptation on de Casteljau's
//  original algorithm.  This algorithm uses basis functions instead of recursion and
//  so speeds the algorithm from factorial time to linear time with respect to the number
//  of control points.
//
//  Usage: 
//      - and + increase or decrease the tessellation granularity.
//      v toggles the display of just the curve the curve and its basis functions.
//      f toggles the size of the basis functions -- from an inset window to fullscreen.
//      c toggles the display of control points.
//      [ and ] decrease and increase the number of control points, respectively.
//      r regenerates random points.
//      p toggles between drawing the curve as a line strip or as points.
//

// The OpenGL enums and functions are used in here to some extent.  I include glut to shield myself
// from having to include windows.h
#include <gl/glut.h>

// stdlib.h is used by rand and srand.
#include <stdlib.h>

// This is the curve itself.
#include <curves/bezier/BezierCurve.h>
#include <curves/CurvePoint.h>
#include <curves/bezier/CentralCurveTessellator.h>
#include <curves/bezier/UniformCurveTessellator.h>

// This value specifies the number of control points.
int numPoints;

// This specifies whether the control points are drawn.
bool drawControls = true;

// This specifies whether or not to draw the basis functions for the curve.
bool verbose = false;

// This controls whether the basis functions are drawn in an inset window or over the whole screen.
bool basesFullscreen = false;

// This is the mode used to draw the curve.  Initially, it's GL_LINE_STRIP so the
// curve is a smooth, connected curve.  'p' toggles from that to GL_POINTS.
GLenum curveMode = GL_LINE_STRIP;

// This is our curve instance.
BezierCurve bezierCurve;

// This is the tessellator we use to render the curve.
BezierCurveTessellator* tessellator;

// Prototypes: These functions are helper functions for the bezier algorithm.
void genCurve();
void incCurveLength();
void decCurveLength();

const char* appWindowName()
{
  return "Bezier Curves";
}

void appCleanup()
{
  delete tessellator;
}

void appInit(int argc, char** argv)
{
  // Get the number of control points.  Default to 5 if no command line arg is given.
  numPoints = 4;
  
  // Setup the curve.
  genCurve();

  // Instantiate a tessellator.
  tessellator = new CentralCurveTessellator;
  
  // A little OpenGL initialization: we use vertex arrays for simplicity and speed here.
  // We don't use any of the others, though.
  glEnableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_NORMAL_ARRAY );
  glDisableClientState( GL_COLOR_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  glDisableClientState( GL_INDEX_ARRAY );
  glDisableClientState( GL_EDGE_FLAG_ARRAY );

  // No lighting necessary.
  glDisable(GL_LIGHTING);
  
  // If we draw the curve as points, we don't want them large, but we do want them visible.
  glPointSize(2);
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch ( keyHit )
  {
    // Turn up the tessellation.
  case '+':
  case '=':
    tessellator->setGranularity( tessellator->getGranularity() + 0.05f );
    break;

    // Turn down the tessellation.
  case '-':
  case '_':
    tessellator->setGranularity( tessellator->getGranularity() - 0.05f );
    break;

    // Decrease the number of control points in the curve.
  case '[':
  case '{':
    numPoints--;
    if (numPoints == 0) numPoints = 1;
    else decCurveLength();
    break;
    
    // Increase the number of control points in the curve.
  case ']':
  case '}':
    numPoints++;
    incCurveLength();
    break;
    
    // Toggle whether control points are drawn.
  case 'c':
  case 'C':
    drawControls = !drawControls;
    break;
    
    // Generate another random set of control points.
  case 'r':
  case 'R':
    genCurve();
    break;
    
    // Toggle between drawing the curve as a GL_LINE_STRIP or GL_POINTS.
  case 'p':
  case 'P':
    tessellator->setMode( tessellator->getMode() == GL_LINE_STRIP ? GL_POINTS : GL_LINE_STRIP );
    break;

    // Toggle drawing of basis functions for the curve.
  case 'v':
  case 'V':
    verbose = !verbose;
    break;

  case 'f':
  case 'F':
    basesFullscreen = !basesFullscreen;
    break;
  }
}

void appDraw()
{
  // Tell the curve to draw.
  bezierCurve.draw( tessellator, drawControls );

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

    bezierCurve.drawBasisFunctions();

    // Clean up after ourselves.
    ::glPopMatrix();
    ::glMatrixMode( GL_MODELVIEW );
    ::glPopMatrix();
  }
}

void genCurve()
{
  // Empty the curve so we can start over.
  bezierCurve.clearControlPoints();

  // Seed the random number generator to change the points each time we run.
  srand( timeGetTime() );
  
  // Get random values for each of the x, y, and z values of each control point.
  for (int lcv = 0; lcv < numPoints; lcv++)
  {
    // Get a random value from -7.5 to 7.5;
    float x = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;
    float y = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;
    float z = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;

    bezierCurve.addControlPoint( x, y, z );
  }
}

void incCurveLength()
{
  // Add a random point.
  float x = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;
  float y = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;
  float z = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;

  bezierCurve.addControlPoint( x, y, z );
}

void decCurveLength()
{
  bezierCurve.removeControlPoint();
}
