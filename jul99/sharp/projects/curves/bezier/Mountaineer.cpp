#include <curves/bezier/Mountaineer.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <gl/glut.h>
#include <mmsystem.h>

Mountaineer::Mountaineer()
{
  numControls = 0;
  controls = 0;
  validities = 0;
  grain = 0;
}

Mountaineer::~Mountaineer()
{
  // Just to be safe...
  delete[] validities;
}

#define INDEX(x,y) ((x) + ((y)*numControls)) 
#define RAND_NEG1_TO_POS1 ((2.0f * rand() / (float)RAND_MAX) - 1.0f)
//#define SCALECONST ((1.0f/grain) - (span/12))
#define SCALECONST (1.0f/grain)
//#define SCALECONST ((1.0f/grain) - (span/2.5f))

void Mountaineer::generateFractalTerrain(CurvePoint* controlsArray, int controlCt, float granularity, float initRange)
{
  // Initialize our data.
  numControls = controlCt;

  delete[] validities;
  validities = new bool[numControls*numControls];
  for (int x=0; x<numControls*numControls; x++)
  {
    validities[x] = false;
    controlsArray[x].setZ(0);
  }

  controls = controlsArray;
  grain = granularity;

  // Get the span of the controls.  Here we assume they form a square grid.
  float span = (float)fabs(controls[INDEX(numControls-1,numControls-1)].getX() - controls[INDEX(0,0)].getX());
  
  // Initialize our random number generator.
  srand(timeGetTime());

  // Initialize the four corners
  controls[INDEX(0            ,            0)].setZ(RAND_NEG1_TO_POS1 * span * initRange);
  controls[INDEX(numControls-1,            0)].setZ(RAND_NEG1_TO_POS1 * span * initRange);
  controls[INDEX(0            ,numControls-1)].setZ(RAND_NEG1_TO_POS1 * span * initRange);
  controls[INDEX(numControls-1,numControls-1)].setZ(RAND_NEG1_TO_POS1 * span * initRange);
  validities[INDEX(0            ,            0)] = true;
  validities[INDEX(numControls-1,            0)] = true;
  validities[INDEX(0            ,numControls-1)] = true;
  validities[INDEX(numControls-1,numControls-1)] = true;

  // Now go!
  subdivide(0,numControls-1,0,numControls-1);

  // Clean up.
  delete[] validities;
  validities = 0;
}

void Mountaineer::subdivide(int x0, int x1, int y0, int y1)
{
  assert(controls != 0);
  assert(validities != 0);
  assert(numControls != 0);

  // We're done if x0 and x1 are next to each other *and* y0 and y1 are next to each other
  // (since the grid is square, both should occur at the same time).
  if (abs(x1-x0) <= 1 && abs(y1-y0) <= 1)
  {
    return;
  }

  // Get the heights at our corners.
  float corner00 = controls[INDEX(x0,y0)].getZ();
  float corner01 = controls[INDEX(x0,y1)].getZ();
  float corner10 = controls[INDEX(x1,y0)].getZ();
  float corner11 = controls[INDEX(x1,y1)].getZ();

  // Get the span of this subdividision.
  float span = (float)fabs(controls[INDEX(x0,y0)].getX() - controls[INDEX(x1,y0)].getX());

  // Get our midpoints.
  int xMid = (x0 + x1) / 2;
  int yMid = (y0 + y1) / 2;

  // Now for the ones that are invalid, generate z values for them.
  if (validities[INDEX(xMid,y0)] == false)
  {
    // Average the parents' heights...
    float result = (corner00 + corner10) * 0.5f;

    // Now find a random value for the point.
    result += (RAND_NEG1_TO_POS1 * span)/SCALECONST;

    // Set that height!
    controls[INDEX(xMid,y0)].setZ(result);

    // Validate it.
    validities[INDEX(xMid,y0)] = true;
  }
  if (validities[INDEX(xMid,y1)] == false)
  {
    // Average the parents' heights...
    float result = (corner01 + corner11) * 0.5f;

    // Now find a random value for the point.
    result += (RAND_NEG1_TO_POS1 * span)/SCALECONST;

    // Set that height!
    controls[INDEX(xMid,y1)].setZ(result);

    // Validate it.
    validities[INDEX(xMid,y1)] = true;
  }
  if (validities[INDEX(x0,yMid)] == false)
  {
    // Average the parents' heights...
    float result = (corner00 + corner01) * 0.5f;

    // Now find a random value for the point.
    result += (RAND_NEG1_TO_POS1 * span)/SCALECONST;

    // Set that height!
    controls[INDEX(x0,yMid)].setZ(result);

    // Validate it.
    validities[INDEX(x0,yMid)] = true;
  }
  if (validities[INDEX(x1,yMid)] == false)
  {
    // Average the parents' heights...
    float result = (corner10 + corner11) * 0.5f;

    // Now find a random value for the point.
    result += (RAND_NEG1_TO_POS1 * span)/SCALECONST;

    // Set that height!
    controls[INDEX(x1,yMid)].setZ(result);

    // Validate it.
    validities[INDEX(x1,yMid)] = true;
  }
  if (validities[INDEX(xMid,yMid)] == false)
  {
    // Average the edge values.
    float result = 0;
    result += controls[INDEX(xMid,y0)].getZ();
    result += controls[INDEX(xMid,y1)].getZ();
    result += controls[INDEX(x0,yMid)].getZ();
    result += controls[INDEX(x1,yMid)].getZ();
    result *= 0.25f;

    // Set that height!
    controls[INDEX(xMid,yMid)].setZ(result);

    // Validate it.
    validities[INDEX(xMid,yMid)] = true;
  }


  // Now, recurse!
  subdivide(x0,xMid,y0,yMid);
  subdivide(x0,xMid,yMid,y1);
  subdivide(xMid,x1,y0,yMid);
  subdivide(xMid,x1,yMid,y1);
}

#undef RAND_NEG1_TO_POS1
#undef INDEX
