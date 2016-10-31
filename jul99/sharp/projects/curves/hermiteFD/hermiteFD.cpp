//
//  Curves and Curved Surfaces: Hermite Curve with forward difference calculations.
//
//      Brian Sharp, 6/5/98
//      brian_sharp@cognitoy.com
//
//  This file contains functions that generate a curve from two points (the
//  curve's endpoints) and a tangent vector at each point.  Hence, it's a cubic
//  curve.  This code samples the curve by sampling points along it, and can either
//  do it in the straightforward (slower) way of just calculating each point from the
//  parametric cubic equation every time, or can use forward differencing to calculate
//  the points, which reduces the time spent on a cubic curve from 3 muls and 3 adds per 
//  coordinate per point (so 9 muls and 9 adds per point) to a per-frame initialization call
//  and a per-point cost of only costs nine additions.
//
//  The basis functions used here are derived (fairly simply, actually, though they
//  don't look intuitive) in _Geometric_Modeling_ by Michael E. Mortenson, ISBN
//  0-471-12957-7 starting on page 39, for the interested.  A brief derivation is
//  given in the header to genCubicFunction() in this file.
//
//  Usage: 
//      - and + increase or decrease the tessellation granularity.
//      [ and ] scale the magnitude of the tangent vectors at the endpoints.
//      d toggles forward differencing.  If on, the curve is sky blue.  If off, the curve is red.
//      f negates the tangents (flips them, if you will).
//      v toggles the display of the tangent vectors at either end of the curve.
//      c toggles the display of the end points
//      r generates a new (random) set of endpoints and tangent vectors.
//      p toggles between drawing the curve as a line strip or as points.
//

// The OpenGL functions are used to draw the curve.  I include glut to shield myself
// from having to include windows.h
#include <gl/glut.h>

// stdlib.h is used by rand and srand.
#include <stdlib.h>

// These arrays hold the endpoints (points[0:2] and points[3:5]) and tangents (tangents[0:2] and tangents[3:5])
float points  [6] = {0,0,0,0,0,0};
float tangents[6] = {0,0,0,0,0,0};

// This array holds the coefficients of the cubic vector function p(t) = at^3 + bt^2 + ct + d
// Note that because a, b, c, and d are xyz vectors, each needs 3 floats.
float functionCoeffs[12] = {0,0,0,  0,0,0,  0,0,0,  0,0,0};

// This array holds the forward difference information, that is, the values for
// the 0th, 1st, 2nd, and 3rd derivative forward differences
float differences[12] = {0,0,0,  0,0,0,  0,0,0,  0,0,0};

// This array holds the curve points (the result of the tessellation.)
float* curvePoints = 0;

// This is used to determine the tangents' length (each component, x, y, or z of each tangent
// is intially a random number from -0.5 to 0.5, and is then multiplied by this value.)
float tangentMagScalar = 6;

// This value is the number of steps in the tessellation.  It must be greater than 0.
int totalSteps = 20;

// This specifies whether the tangent vectors are drawn, or just the curve
bool verbose = true;

// This specifies whether the end points are drawn.
bool drawEndpoints = true;

// This determines whether we're using forward differencing to calculate points on the curve or not.
bool useForwardDifference = true;

// This is the mode used to draw the curve.  Initially, it's GL_LINE_STRIP so the
// curve is a smooth, connected curve.  'p' toggles from that to GL_POINTS.
GLenum curveMode = GL_LINE_STRIP;

// Spin stuff with < and >
float rotTheta = 0;
float spinRate = 0;

// Prototypes: These functions are helper functions for the hermite algorithm.
void genCurveArray();
void genEndpointsAndTangents();
void scaleTangents(float from, float to);
void genCubicFunction();

// Prototypes: This function is the function that calculates points on the curve
// straight from the cubic function.  Slower because it takes 9 muls per point.
void evaluateAt(float t, float* out);

// Prototypes: These functions are the functions that calculate the points on the
// curve from the forward differences.  Faster because after the once-per-frame
// initialization call, it only takes 9 adds to generate successive points.
void evaluateNext(float* out);
void initForwardDifferences(float n, float* out);

const char* appWindowName()
{
    return "Hermite Curves with Forward Differencing";
}

void appCleanup()
{
    delete[] curvePoints;
}

void appInit(int argc, char** argv)
{
    // Alocate space for the control points.
    genEndpointsAndTangents();

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

    // Decrease the magnitude of the tangents to the curve at the endpoints.
    case '[':
    case '{':
        if (tangentMagScalar >= 0.5f)
        {
            scaleTangents(tangentMagScalar, tangentMagScalar - 0.5f);
            tangentMagScalar -= 0.5f;
        }
        break;

    // Increase the magnitude of the tangents to the curve at the endpoints.
    case ']':
    case '}':
        scaleTangents(tangentMagScalar, tangentMagScalar + 0.5f);
        tangentMagScalar += 0.5f;
        break;

    // Flip the normals (negate them)
    case 'f':
    case 'F':
        scaleTangents(tangentMagScalar, -tangentMagScalar);
        break;

    // Toggle whether intermediate steps are drawn.
    case 'v':
    case 'V':
        verbose = !verbose;
        break;

    // Toggle whether control points are drawn.
    case 'c':
    case 'C':
        drawEndpoints = !drawEndpoints;
        break;

    // Generate another random set of control points.
    case 'r':
    case 'R':
        genEndpointsAndTangents();
        break;

    // Toggle between drawing the curve as a GL_LINE_STRIP or GL_POINTS.
    case 'p':
    case 'P':
        curveMode = (curveMode == GL_LINE_STRIP) ? GL_POINTS : GL_LINE_STRIP;
        break;
        
    // Toggle between using or not using forward differencing.
    case 'd':
    case 'D':
        useForwardDifference = !useForwardDifference;
        break;

    case '<':
        spinRate -= 1.0f;
        break;

    case '>':
        spinRate += 1.0f;
        break;
    }
}

void appDraw()
{
    // Add the rotation per frame to the rotation angle 
    rotTheta += spinRate;

    // Rotate.  You can control rotation with < and >
    glRotatef(rotTheta, 0, 0, 1);

    // These are our colors for endpoints, tangents, and the curve itself.
    float pointColor[3] = {0.0f, 1.0f, 0.0f};
    float lineColor[3] = {1.0f, 1.0f, 0.0f};
    
    // curveColor1 is used if forward differencing is on, else it uses curveColor2.
    float curveColor1[3] = {0.2f, 0.2f, 0.8f};
    float curveColor2[3] = {1.0f, 0.0f, 0.0f};

    // If we're drawing endpoints, draw them.
    if (drawEndpoints)
    {
        // Save whatever the size of the points was and set the point
        // size up so the endpoints are big.
        glPushAttrib(GL_POINT_BIT);
        glPointSize(4);

        // Set the color for the points.
        glColor3fv(pointColor);

        // Draw the endpoints.
        glVertexPointer( 3, GL_FLOAT, 0, points );
        glDrawArrays( GL_POINTS, 0, 2 );

        // Now pop the point size back to what it was.
        glPopAttrib();
    }

    // If we're drawing tangents, draw them.
    if (verbose)
    {
        // Set the color for the tangents.
        glColor3fv(lineColor);

        // Draw the tangents.  These aren't stored in a convenient format
        // for vertex arrays, so use the standard glBegin / glEnd.
        glBegin(GL_LINES);

        // Draw the first tangent as a line from the first endpoint to the
        // first endpoint plus the first tangent vector.
        glVertex3fv(points + 0);
        glVertex3f (tangents[ 0 ] + points[ 0 ], tangents[ 1 ] + points[ 1 ], tangents[ 2 ] + points[ 2 ]);

        // Draw the second tangent as a line from the second endpoint to the
        // second endpoint plus the second tangent vector.
        glVertex3fv(points + 3);
        glVertex3f (tangents[ 3 ] + points[ 3 ], tangents[ 4 ] + points[ 4 ], tangents[ 5 ] + points[ 5 ]);

        // We're done.
        glEnd();
    }

    // We have to declare these here as they could be used in one or both of the
    // point computation loops, and we can't initialize them in an if statement.
    int step;
    float invTotalSteps;

    // If we're using forward differencing, generate the points that way.
    if (useForwardDifference)
    {
        // Initialize the forward differences, and get the first point on the curve.
        initForwardDifferences(totalSteps - 1, curvePoints + 0);

        // Now generate the points on the curve.  Note that since we already have
        // the first point, we start at step 1, not zero.
        for (step = 1; step < totalSteps; step++)
        {
            // Generate a point on the curve for this step and 
            // store it in the array of points along the curve.
            evaluateNext(curvePoints + step*3);
        }
    }
    // Otherwise, do it the old-fashioned, simple but slower way.
    else
    {
        // This saves us a divide per step.
        invTotalSteps = 1.0f / (float)(totalSteps - 1);

        // Now generate the points on the curve.
        for (int step = 0; step < totalSteps; step++)
        {
            // Generate a point on the curve for this step and 
            // store it in the array of points along the curve.
            evaluateAt(step * invTotalSteps, curvePoints + step*3);
        }
    }

    // Set the color for the curve based on whether we're using forward differencing or not.
    if (useForwardDifference)
    {
        glColor3fv(curveColor1);
    }
    else
    {
        glColor3fv(curveColor2);
    }

    // Draw the curve from the generated points.
    glVertexPointer( 3, GL_FLOAT, 0, curvePoints );
    glDrawArrays( curveMode, 0, totalSteps );
}

void genCurveArray()
{
    // This is called when we change the number of steps.  Deallocate the old curve
    // point array and allocate a new one with the right number of steps.
    delete[] curvePoints;
    curvePoints = new float[ totalSteps*3 ];
}

void genEndpointsAndTangents()
{
    // Seed the random number generator to change the points each time we run.
    srand( timeGetTime() );

    // Get random values for each of the x, y, and z values of the endpoints and tangents.
    for (int lcv = 0; lcv < 6; lcv++)
    {
        // Get a random value from -7.5 to 7.5 (so the curve fills the viewport as much as possible).
        points[ lcv ] = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * 15;

        // Get a random value from -tangentMagScalar/2 to tangentMagScalar/2 for the tangent components.
        tangents[ lcv ] = ( ( (float) rand() / (float) RAND_MAX ) - 0.5f ) * tangentMagScalar;
    }

    // We need to generate the cubic curve function from the basis functions now, since
    // we just generated a new set of endpoints and tangent vectors.
    genCubicFunction();
}

void scaleTangents(float from, float to)
{
    // Find the scalar (to / from) so we only have to do one div
    // in this function, not 6.
    float scalar = to / from;

    // Scale each component of both normals.
    for (int lcv = 0; lcv < 6; lcv++)
    {
        tangents[ lcv ] *= scalar;
    }

    // Now, regenerate the cubic function since we've changed the normals.
    genCubicFunction();
}

// Derivation of the functions used here:
//
//  Given the vector representation of a general cubic function,
//
//      p(t) = at^3 + bt^2 + ct + d
//
//  where a, b, c, d, and p(t) are vectors (here in R3, xyz space),
//
//  and given p0 and p1, the endpoints of the curve, and dpdt0 and dpdt1, the
//  tangent vectors at either endpoint, then through simple derivatives and
//  plugging in either 0 or 1 for t, we get:
//
//      p0 = d
//      p1 = a + b + c + d
//      dpdt0 = c
//      dpdt1 = 3a + 2b + c
//
//  and by solving simultaneous equations for a, b, c, and d, we get:
//
//      a = 2p0 - 2p1 + dpdt0 + dpdt1
//      b = -3p0 + 3p1 - 2dpdt0 - dpdt1
//      c = dpdt0
//      d = p0
//
//  For a more detailed explanation, see the book mentioned in this file's
//  header comments.  Credit for this derivation goes to that book (it's just
//  a quick summary of the book's explanation.)
//
void genCubicFunction()
{
    // Do this so we can treat each endpoint and tangent vector as a separate array.
    float* p0 = points;
    float* p1 = points + 3;
    float* dpdt0 = tangents;
    float* dpdt1 = tangents + 3;

    // Do this so we can treat each vector coefficient of the function as a separate array.
    float* a = functionCoeffs;
    float* b = functionCoeffs + 3;
    float* c = functionCoeffs + 6;
    float* d = functionCoeffs + 9;

    // Now, generate each coefficient from the endpoints, tangents, and the predefined basis functions.
    // Note that we loop once each for the x, y, and z components of the vector function.
    for (int lcv = 0; lcv < 3; lcv++)
    {
        // a = 2p0 - 2p1 + dpdt0 + dpdt1
        a[ lcv ] = (p0[ lcv ] + p0[ lcv ]) - (p1[ lcv ] + p1[ lcv ]) + dpdt0[ lcv ] + dpdt1[ lcv ];

        // b = -3p0 + 3p1 - 2dpdt0 - dpdt1
        b[ lcv ] = - (p0[ lcv ] + p0[ lcv ] + p0[ lcv ]) + (p1[ lcv ] + p1[ lcv ] + p1[ lcv ]) - (dpdt0[ lcv ] + dpdt0[ lcv ]) - dpdt1[ lcv ];

        // c = dpdt0
        c[ lcv ] = dpdt0[ lcv ];

        // d = p0
        d[ lcv ] = p0[ lcv ];
    }
}

// This function simply computes at^3 + bt^2 + ct + d for a specific t and stores
// the vector result in out.  This function is used if forward differencing is disabled.
void evaluateAt(float t, float* out)
{
    // Do this so we can treat each vector coefficient of the function as a separate array.
    float* a = functionCoeffs;
    float* b = functionCoeffs + 3;
    float* c = functionCoeffs + 6;
    float* d = functionCoeffs + 9;

    // Note that we use Horner's rule for computing polynomials (which is the way
    // we nest the multiplies and adds to minimize the computation we need.)
    out[ 0 ] = ( ( ( a[ 0 ] ) * t + b[ 0 ] ) * t + c[ 0 ] ) * t + d[ 0 ];
    out[ 1 ] = ( ( ( a[ 1 ] ) * t + b[ 1 ] ) * t + c[ 1 ] ) * t + d[ 1 ];
    out[ 2 ] = ( ( ( a[ 2 ] ) * t + b[ 2 ] ) * t + c[ 2 ] ) * t + d[ 2 ];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//// Below here is the code that does the forward difference calculations. ////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// This function initializes the forward differences and computes the first
// point on the curve (when t = 0).
//
// The initial values of the forward differences for a general-form cubic
// polynomial are:
//
//      cubic = d
//      quadratic = a/n^3 + b/n^2 + c/n
//      linear = 6a / n^3 + 2b / n^2
//      constant = 6a / n^3
//      
//  Refer to Geometric Modeling by Mortenson (see heading comments for details)
//  for the derivation of these formulas.
//
void initForwardDifferences(float n, float* out)
{
    // Do this so we can treat each vector coefficient of the function as a separate array.
    float* a = functionCoeffs;
    float* b = functionCoeffs + 3;
    float* c = functionCoeffs + 6;
    float* d = functionCoeffs + 9;

    // Do this so we can treat each vector in the forward differences individually.
    float* cubic     = differences;
    float* quadratic = differences + 3;
    float* linear    = differences + 6;
    float* constant  = differences + 9;

    // Calculate the initial forward differences for x, y, and z.
    for (int lcv = 0; lcv < 3; lcv++)
    {
        // cubic = d
        cubic    [ lcv ] = d[ lcv ];
        
        // quadratic = a/n^3 + b/n^2 + c/n
        quadratic[ lcv ] = ( ( ( ( a[ lcv ] / n ) + b[ lcv ] ) / n ) + c[ lcv ] ) / n;
        
        // linear = 6a / n^3 + 2b / n^2
        linear   [ lcv ] = ( ( ( 6 * a[ lcv ] ) / n ) + b[ lcv ] + b[ lcv ] ) / ( n * n );
        
        // constant = 6a / n^3
        constant [ lcv ] = ( 6 * a[ lcv ] ) / ( n * n * n );
    }

    // Now give them the first point on the curve.
    out[ 0 ] = cubic[ 0 ];
    out[ 1 ] = cubic[ 1 ];
    out[ 2 ] = cubic[ 2 ];
}

// This function advances the forward differences and, in doing so, generates
// the next point on the curve.
//
// This is actually a bit of a gotcha.  The first time I implemented it,
// I just blindly implemented what I read.  However, when it didn't work, I
// sat down to think about it, and realized that you want to do:
//
//     cubic += quadratic
//     quadratic += linear
//     linear += constant
//
// and not...
//
//     linear += constant
//     quadratic += linear
//     cubic += quadratic
//
// Think of it this way: each forward difference is the change in the next-higher
// forward difference from the step we're at to the step we're trying to reach.
// Therefore, it's instantaneous; every forward difference should be added to the forward
// difference one higher than it, and all at the exact same instant, so it's not cumulative
// within a single call to evaluateNext. It's cumulative between calls to evaluateNext.
// If that makes any sense at all...
//
// Note that when I refer to forward difference A being "one higher" than forward difference B,
// I mean that A is the nth-derivative forward difference, and that B is the (n+1)th-derivative
// forward difference.  So cubic is the forward difference that's one higher than quadratic, for
// example.
//
void evaluateNext(float* out)
{
    // Do this so we can treat each vector in the forward differences individually.
    float* cubic     = differences;
    float* quadratic = differences + 3;
    float* linear    = differences + 6;
    float* constant  = differences + 9;

    // Add the quadratic difference to the cubic difference.
    cubic[ 0 ] += quadratic[ 0 ];
    cubic[ 1 ] += quadratic[ 1 ];
    cubic[ 2 ] += quadratic[ 2 ];

    // Add the linear difference to the quadratic difference.
    quadratic[ 0 ] += linear[ 0 ];
    quadratic[ 1 ] += linear[ 1 ];
    quadratic[ 2 ] += linear[ 2 ];

    // Add the constant difference to the linear difference.
    linear[ 0 ] += constant[ 0 ];
    linear[ 1 ] += constant[ 1 ];
    linear[ 2 ] += constant[ 2 ];

    // Now give them the new point on the curve.
    out[ 0 ] = cubic[ 0 ];
    out[ 1 ] = cubic[ 1 ];
    out[ 2 ] = cubic[ 2 ];
}