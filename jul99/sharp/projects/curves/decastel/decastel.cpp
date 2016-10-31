//
//  Curves and Curved Surfaces: De Casteljau recursive curve algorithm
//
//      Brian Sharp, 6/2/98
//      brian_sharp@cognitoy.com
//
//  This file contains functions that generate a curve from randomly generated
//  control points using De Casteljau's original recursive algorithm.
//
//  Usage: 
//      - and + increase or decrease the tessellation granularity.
//      v toggles the display of just the curve or all the levels of recursion.
//      c toggles the display of control points.
//      [ and ] decrease and increase the number of control points, respectively.
//      r regenerates random points.
//      p toggles between drawing the curve as a line strip or as points.
//

// The OpenGL functions are used to draw the curve.  I include glut to shield myself
// from having to include windows.h
#include <gl/glut.h>

// stdlib.h is used by rand and srand.
#include <stdlib.h>

// This value specifies the number of control points.
int numPoints;

// This array holds the control points as xyz triplets.
float** points = 0;

// This array holds the curve points (the result of all the recursion).
float* curvePoints = 0;

// This value is the number of steps in the tessellation.  It must be greater than 0.
int totalSteps = 20;

// This specifies whether the intermediate steps are drawn, or just the final curve.
bool verbose = false;

// This specifies whether the control points are drawn.
bool drawControls = true;

// This is the mode used to draw the curve.  Initially, it's GL_LINE_STRIP so the
// curve is a smooth, connected curve.  'p' toggles from that to GL_POINTS.
GLenum curveMode = GL_LINE_STRIP;

// Rotation with the > and < keys.
float rotTheta = 0.0f;
float spinRate = 0.0f;

// Prototypes: These functions are helper functions for the de casteljau algorithm.
void genPoints();
void genCurveArray();
void genPointArrays();
void evaluateAt(int level, float t);

const char* appWindowName()
{
    return "De Casteljau Curves";
}

void appCleanup()
{
    delete[] points;
    delete[] curvePoints;
}

void appInit(int argc, char** argv)
{
    // Get the number of control points.  Default to 5 if no command line arg is given.
    numPoints = 5;

    // Alocate space for the control points.
    genPointArrays();

    // Now allocate space for the curve points (the final tessellation).
    genCurveArray();

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
    // Decrease the number of tessellations in the curve.
    case '-':
    case '_':
        totalSteps--;
        if (totalSteps == 1) totalSteps = 2;
        genCurveArray();
        break;

    // Increase the number of tessellations in the curve.
    case '+':
    case '=':
        totalSteps++;
        genCurveArray();
        break;

    // Decrease the number of control points in the curve.
    case '[':
    case '{':
        numPoints--;
        if (numPoints == 2) numPoints = 3;
        genPointArrays();
        break;

    // Increase the number of control points in the curve.
    case ']':
    case '}':
        numPoints++;
        genPointArrays();
        break;

    // Toggle whether intermediate steps are drawn.
    case 'v':
    case 'V':
        verbose = !verbose;
        break;

    // Toggle whether control points are drawn.
    case 'c':
    case 'C':
        drawControls = !drawControls;
        break;

    // Generate another random set of control points.
    case 'r':
    case 'R':
        genPoints();
        break;

    // Toggle between drawing the curve as a GL_LINE_STRIP or GL_POINTS.
    case 'p':
    case 'P':
        curveMode = (curveMode == GL_LINE_STRIP) ? GL_POINTS : GL_LINE_STRIP;
        break;

    // Spinning.
    case '<':
        spinRate += 1.0f;
        break;

    case '>':
        spinRate -= 1.0f;
        break;
    }
}

void appDraw()
{
    // Add the rotation per frame to the rotation angle 
    rotTheta += spinRate;

    // Rotate.  You can control rotation with < and >
    glRotatef(rotTheta, 0, 0, 1);

    // These are our colors for control points, intermediate lines, and the curve itself.
    float pointColor[3] = {0.0, 1.0, 0.0};
    float lineColor[3] = {0.0, 0.0, 1.0};
    float curveColor[3] = {1.0, 0.0, 0.0};

    // If we're drawing control points, draw them.
    if (drawControls)
    {
        // Save whatever the size of the points was and set the point
        // size up so the control points are big.
        glPushAttrib(GL_POINT_BIT);
        glPointSize(4);

        // Set the color for the points.
        glColor3fv(pointColor);

        // Draw the curve's control points.
        glVertexPointer( 3, GL_FLOAT, 0, points[ 0 ] );
        glDrawArrays( GL_POINTS, 0, numPoints );

        // Now pop the point size back to what it was.
        glPopAttrib();
    }

    // Set the color for the intermediate steps and recurse to generate the points.
    glColor3fv(lineColor);

    // This saves us a divide per step.
    float invTotalSteps = 1.0f / (float)(totalSteps - 1);

    // Now generate the points on the curve.
    for (int step = 0; step < totalSteps; step++)
    {
        // Generate a point on the curve for this step.
        evaluateAt(0, step * invTotalSteps);

        // Take that point out of the array of control points and into our curvePoints array.
        curvePoints[ step*3 + 0 ] = points[ numPoints - 1 ][ 0 ];
        curvePoints[ step*3 + 1 ] = points[ numPoints - 1 ][ 1 ];
        curvePoints[ step*3 + 2 ] = points[ numPoints - 1 ][ 2 ];
    }

    // Set the color for the curve.
    glColor3fv(curveColor);

    // Draw the curve from the generated points.
    glVertexPointer( 3, GL_FLOAT, 0, curvePoints );
    glDrawArrays( curveMode, 0, totalSteps );

    // Turn lighting back on.
    glEnable(GL_LIGHTING);
}

void genPointArrays()
{
    // We need an array for each level of recursion, and there's numPoints of them.
    points = new float*[ numPoints ];

    // The largest array (the first one) has numPoints xyz triplets, so make the arrays
    // that big.
    for (int lcv = 0; lcv < numPoints; lcv++)
    {
        points[ lcv ] = new float[ numPoints*3 ];
    }

    // Now make points for the new arrays.
    genPoints();
}

void genCurveArray()
{
    // This is called when we change the number of steps.  Deallocate the old curve
    // point array and allocate a new one with the right number of steps.
    delete[] curvePoints;
    curvePoints = new float[ totalSteps*3 ];
}

void genPoints()
{
    // Seed the random number generator to change the points each time we run.
    srand( timeGetTime() );

    // Get random values for each of the x, y, and z values of each control point.
    for (int lcv = 0; lcv < numPoints * 3; lcv++)
    {
        // Get a random value from -7.5 to 7.5;
        points[ 0 ][ lcv ] = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;
    }
}

void evaluateAt(int level, float t)
{
    // Temporary values used to generate each point.
    float x, y, z;

    // Our base case: are we at the bottom level?  If so, we've
    // just generated the point on the curve, so return.
    if (level == numPoints - 1)
    {
        return;
    }

    // Take each pair of points on the current level and interpolate them
    // using t and (1 - t) to generate a point for the next level.
    for (int lcv = 0; lcv < numPoints - level - 1; lcv++)
    {
        x = t * points[ level ][ lcv*3 + 0 ] + (1.0 - t) * points[ level ][ lcv*3 + 3 ];
        y = t * points[ level ][ lcv*3 + 1 ] + (1.0 - t) * points[ level ][ lcv*3 + 4 ];
        z = t * points[ level ][ lcv*3 + 2 ] + (1.0 - t) * points[ level ][ lcv*3 + 5 ];

        points[ level + 1 ][ lcv*3 + 0 ] = x;
        points[ level + 1 ][ lcv*3 + 1 ] = y;
        points[ level + 1 ][ lcv*3 + 2 ] = z;
    }

    // If we're drawing intermediate steps, show the lines we just generated.
    if ( verbose )
    {
        glVertexPointer( 3, GL_FLOAT, 0, points[ level ] );
        glDrawArrays( GL_LINE_STRIP, 0, numPoints - level );
    }

    // Generate the next level of points.
    evaluateAt(level + 1, t);
}
