#include <curves/bezier/CentralPatchTessellator.h>
#include <curves/bezier/BezierBasis.h>
#include <curves/Polynomial.h>
#include <curves/GlobalCamera.h>
#include <curves/OpenGL.h>
#include <gl/glut.h>
#include <assert.h>

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265359
#endif

#include <iostream>

// Stupid STL warning (more than 255 characters of garbage!  Truncated!)
#pragma warning (disable: 4786)

// Access functions for the points and the texture coordinates to make the code a little
// smaller and a tad more legible.
#define REALPOINT(x) (actualPoints + ((x)*3))
#define REALTEX(x) (actualTexCoords + ((x)*2))

struct PatchPoint {
  // This is the actual point -- the zeroth derivatives of u and v at the point.
  CurvePoint u0v0;

  // These are the two second derivatives.
  CurvePoint u0v2;
  CurvePoint u2v0;

  // This is the double-second-derivative.
  CurvePoint u2v2;
};

CentralPatchTessellator::CentralPatchTessellator() 
: BezierPatchTessellator()
{
  // Set stuff to better initial values (so we get a real surface instead of the minimalist
  // tessellation at startup).
  vertices = 0;

  actualVerts = new PatchPoint[1];
  actualPoints = new float[3];
  actualTexCoords = new float[2];
  vertCapacity = 1;
  vertIndex = 0;
}

CentralPatchTessellator::CentralPatchTessellator(const CentralPatchTessellator& source)
: BezierPatchTessellator(source)
{
  threshold = source.threshold;
  vertices = 0;

  actualVerts = new PatchPoint[1];
  actualPoints = new float[3];
  actualTexCoords = new float[2];
  vertCapacity = 1;
  vertIndex = 0;

  setMaxDepth(source.getMaxDepth());
}

CentralPatchTessellator& CentralPatchTessellator::operator=(const CentralPatchTessellator& source)
{
  BezierPatchTessellator::operator=(source);
  threshold = source.threshold;
  setMaxDepth(source.getMaxDepth());

  actualVerts = new PatchPoint[1];
  actualPoints = new float[3];
  actualTexCoords = new float[2];
  vertCapacity = 1;
  vertIndex = 0;

  return *this;
}

CentralPatchTessellator::~CentralPatchTessellator()
{
  delete[] vertices;
  delete[] actualVerts;
  delete[] actualPoints;
  delete[] actualTexCoords;
}

// We override this so that we can play with the value they pass us so that it makes sense for us.
// Basically, we need a threshold at which we stop recursing, so we tweak this to taste.
void CentralPatchTessellator::setGranularity(float newVal)
{
  BezierPatchTessellator::setGranularity(newVal);

  // Scale it in by 1/2 centered at 1.0
  newVal -= 1.0f;
  newVal *= 0.5f;
  newVal += 1.0f;

  // Make it exponential so things happen a bit quicker.
  newVal = pow(20.0f, newVal);

  threshold = 1.0f / newVal;
}

void CentralPatchTessellator::tessellate(const std::vector< std::vector< CurvePoint > >& controls, 
                                          const std::vector< BasisFunction >& basesU, 
                                          const std::vector< BasisFunction >& basesV) const
{
  // Make sure our arrays are initialized if this is the first time this was called.
  if (vertices == 0 || actualVerts == 0)
  {
    setMaxDepth(getMaxDepth());
  }

  // Reset the vertices so we effectively clean out last frame's allocation but also keep that
  // memory around because we'll probably need close to that amount again this frame.
  vertIndex = 0;

  // Store the camera info. for use in tessellation.
  cameraPos.setX(GlobalCamera::Instance()->getPosition().x);
  cameraPos.setY(GlobalCamera::Instance()->getPosition().y);
  cameraPos.setZ(GlobalCamera::Instance()->getPosition().z);

  tanHalfFov = tan(GlobalCamera::Instance()->getFovY() * (M_PI / 180.0f) * 0.5f);

  // Quick check: only draw bicubic patches, because that's all we can do.
  if (controls.size() != 4 || controls[0].size() != 4)
  {
    return;
  }

  // First, we need to make the basis functions for our second derivs, which just involves
  // taking the derivative of each basis function twice.
  std::vector< BasisFunction > basesUdudu(basesU);
  std::vector< BasisFunction > basesVdvdv(basesV);

  for (int i = 0; i < basesUdudu.size(); i++)
  {
    basesUdudu[i].differentiate();
    basesUdudu[i].differentiate();
  }
  for (int j = 0; j < basesVdvdv.size(); j++)
  {
    basesVdvdv[j].differentiate();
    basesVdvdv[j].differentiate();
  }

  for (i = 0; i < numEdgeVerts * numEdgeVerts; i++)
  {
    vertices[i] = -1;
  }

  int corners[4];
  corners[3] = getMaxXMaxYIndex();
  corners[2] = getMinXMaxYIndex();
  corners[1] = getMaxXMinYIndex();
  corners[0] = getMinXMinYIndex();

  for (int stepU = 0; stepU < 2; stepU++)
  {
    // Generate the parameter for this step of the curve.
    float u = stepU;

    for (int stepV = 0; stepV < 2; stepV++)
    {
      // Generate the parameter for this step of the curve.
      float v = stepV;

      // This holds the point we're working on as we add control points' contributions to it.
      float curU0V0[3] = { 0, 0, 0 };
      float curU0V2[3] = { 0, 0, 0 };
      float curU2V0[3] = { 0, 0, 0 };
      float curU2V2[3] = { 0, 0, 0 };

      // Generate a point on the curve for this step.
      for (i = 0; i < controls.size(); i++)
      {
        for (j = 0; j < controls[0].size(); j++)
        {
          // Get a few basis function values and products thereof that we'll need.
          float bu = basesU[i](u);
          float bv = basesV[j](v);
          float dbu = basesUdudu[i](u);
          float dbv = basesVdvdv[j](v);
          float bu_bv = bu * bv;
          float bu_dbv = bu * dbv;
          float dbu_bv = dbu * bv;
          float dbu_dbv = dbu * dbv;

          // Add this control point's contribution onto the current point.
          curU0V0[0] += controls[i][j].getX() * bu_bv;
          curU0V0[1] += controls[i][j].getY() * bu_bv;
          curU0V0[2] += controls[i][j].getZ() * bu_bv;

          // Add this point's contribution to our v-tangent.
          curU0V2[0] += controls[i][j].getX() * bu_dbv;
          curU0V2[1] += controls[i][j].getY() * bu_dbv;
          curU0V2[2] += controls[i][j].getZ() * bu_dbv;
          
          // Add this point's contribution to our u-tangent.
          curU2V0[0] += controls[i][j].getX() * dbu_bv;
          curU2V0[1] += controls[i][j].getY() * dbu_bv;
          curU2V0[2] += controls[i][j].getZ() * dbu_bv;

          // Add this point's contribution to our u-tangent.
          curU2V2[0] += controls[i][j].getX() * dbu_dbv;
          curU2V2[1] += controls[i][j].getY() * dbu_dbv;
          curU2V2[2] += controls[i][j].getZ() * dbu_dbv;
        }
      }

      // Initial conditions use 1/2 the second derivative and 1/4 of the double-second.
      curU0V2[0] *= 0.5f;
      curU0V2[1] *= 0.5f;
      curU0V2[2] *= 0.5f;

      curU2V0[0] *= 0.5f;
      curU2V0[1] *= 0.5f;
      curU2V0[2] *= 0.5f;

      curU2V2[0] *= 0.25f;
      curU2V2[1] *= 0.25f;
      curU2V2[2] *= 0.25f;

      // Stuff this initial point into our data structure.
      vertices[corners[stepU + stepV*2]] = addNewVertex();
      REALPOINT(vertices[corners[stepU + stepV*2]])[0] = curU0V0[0];
      REALPOINT(vertices[corners[stepU + stepV*2]])[1] = curU0V0[1];
      REALPOINT(vertices[corners[stepU + stepV*2]])[2] = curU0V0[2];
      REALTEX(vertices[corners[stepU + stepV*2]])[0] = u;
      REALTEX(vertices[corners[stepU + stepV*2]])[1] = v;
      actualVerts[vertices[corners[stepU + stepV*2]]].u0v0.setXYZArray(curU0V0);
      actualVerts[vertices[corners[stepU + stepV*2]]].u0v2.setXYZArray(curU0V2);
      actualVerts[vertices[corners[stepU + stepV*2]]].u2v0.setXYZArray(curU2V0);
      actualVerts[vertices[corners[stepU + stepV*2]]].u2v2.setXYZArray(curU2V2);
    }
  }

  // Now go!  Tessellate, starting with patch corners and working in.
  tessellateSubPatch(corners[0], corners[1], corners[2], corners[3], 0.5, 0);

  if (getFixCracks())
  {
    fixCracks(corners[0], corners[1], corners[2], corners[3]);
  }
}

// There should be 4 corners, in the following order (f(u,v)): f(0,0), f(1,0), f(0,1), f(1,1)
// Naturally, the values aren't going to always be 0 and 1, but that's the order.
void CentralPatchTessellator::tessellateSubPatch(int corner0Index, 
                                                  int corner1Index,
                                                  int corner2Index, 
                                                  int corner3Index, 
                                                  float delta, int depth) const
{
  // Quick check: if we passed the max depth, stop now.
  if (depth >= getMaxDepth())
  {
    return;    
  }

  // Precalc this; we need it.
  float delta2 = delta * delta;

  // These are the points along the edges, and the center point.
  // Note that we find their addresses by averaging the addresses of the corners.
  int u0Index = (corner0Index + corner2Index) / 2;
  int u1Index = (corner1Index + corner3Index) / 2;
  int v0Index = (corner0Index + corner1Index) / 2;
  int v1Index = (corner2Index + corner3Index) / 2;
  int centerPointIndex = (corner0Index + corner3Index) / 2;

  PatchPoint* corner0 = &actualVerts[vertices[corner0Index]];
  PatchPoint* corner1 = &actualVerts[vertices[corner1Index]];
  PatchPoint* corner2 = &actualVerts[vertices[corner2Index]];
  PatchPoint* corner3 = &actualVerts[vertices[corner3Index]];

  // These are the temporary derivatives along the edges (we store them here on the stack until we
  // decide to calculate this level at which point we allocate points and copy these in.
  CurvePoint v0u2v0;
  CurvePoint v1u2v0;
  CurvePoint u0u0v2;
  CurvePoint u1u0v2;

  // These are the non-linear terms along the edges, and the non-linear term for the center.
  CurvePoint nlu_u0;
  CurvePoint nlu_u1;
  CurvePoint nlv_v0;
  CurvePoint nlv_v1;
  CurvePoint nl_cen;

  // These are the non-linear terms for the second derivatives along the edges.
  CurvePoint nlv_u0;
  CurvePoint nlv_u1;
  CurvePoint nlu_v0;
  CurvePoint nlu_v1;

  // This holds the magnitudes (squared) for each of the non-linear terms.
  float u0Mag2;
  float u1Mag2;
  float v0Mag2;
  float v1Mag2;

  // These are computed from the u0Mag2 etc. and the distance from the camera
  // to the patch and the field-of-view angle and serve as a heuristic for tessellating
  // further (or not).
  float u0Mag2ScreenSpace;
  float u1Mag2ScreenSpace;
  float v0Mag2ScreenSpace;
  float v1Mag2ScreenSpace;

  // Find the non-linear terms of each point from the corner points.
  if (vertices[v0Index] == -1)
  {
    v0u2v0 = corner0->u2v0;
    v0u2v0 += corner1->u2v0;
    v0u2v0 *= 0.5f;
  }
  else
  {
    v0u2v0 = actualVerts[vertices[v0Index]].u2v0;
  }
  nlv_v0 = v0u2v0;
  nlv_v0 *= delta2;

  if (vertices[v1Index] == -1)
  {
    v1u2v0 = corner2->u2v0;
    v1u2v0 += corner3->u2v0;
    v1u2v0 *= 0.5f;
  }
  else
  {
    v1u2v0 = actualVerts[vertices[v1Index]].u2v0;
  }
  nlv_v1 = v1u2v0;
  nlv_v1 *= delta2;

  if (vertices[u0Index] == -1)
  {
    u0u0v2 = corner0->u0v2;
    u0u0v2 += corner2->u0v2;
    u0u0v2 *= 0.5f;
  }
  else
  {
    u0u0v2 = actualVerts[vertices[u0Index]].u0v2;
  }
  nlu_u0 = u0u0v2;
  nlu_u0 *= delta2;

  if (vertices[u1Index] == -1)
  {
    u1u0v2 = corner1->u0v2;
    u1u0v2 += corner3->u0v2;
    u1u0v2 *= 0.5f;
  }
  else
  {
    u1u0v2 = actualVerts[vertices[u1Index]].u0v2;
  }
  nlu_u1 = u1u0v2;
  nlu_u1 *= delta2;

  u0Mag2 = nlu_u0.magnitude2();
  u1Mag2 = nlu_u1.magnitude2();
  v0Mag2 = nlv_v0.magnitude2();
  v1Mag2 = nlv_v1.magnitude2();

  // Now, we roughly calculate the center point of the patch by averaging two corners.
  // We take the distance from the camera to this point as one part of our heuristic.
  CurvePoint poorCenter = corner0->u0v0;
  poorCenter += corner3->u0v0;
  poorCenter *= 0.5f;

  poorCenter -= cameraPos;
  float distance2 = poorCenter.magnitude2();

  // Now, to tessellate more when we can see the curvature, compute the dot product of
  // the vector from the eye to the patch with each of the non-linearity vectors.  Then,
  // the closer the result is to 0, the more perpendicular (and therefore more important /
  // visible / whatever the non-linearity is on the screen).  Use that to scale the chance
  // that we recurse.
  float u0DirScalar = nlu_u0.getX() * poorCenter.getX() + nlu_u0.getY() * poorCenter.getY() + nlu_u0.getZ() * poorCenter.getZ();
  float u1DirScalar = nlu_u1.getX() * poorCenter.getX() + nlu_u1.getY() * poorCenter.getY() + nlu_u1.getZ() * poorCenter.getZ();
  float v0DirScalar = nlv_v0.getX() * poorCenter.getX() + nlu_v0.getY() * poorCenter.getY() + nlu_v0.getZ() * poorCenter.getZ();
  float v1DirScalar = nlv_v1.getX() * poorCenter.getX() + nlu_v1.getY() * poorCenter.getY() + nlu_v1.getZ() * poorCenter.getZ();

  // I've taken this out because the poorCenter used can make the results blatantly inaccurate
  // and result in big popping.  It wouldn't be expensive to generate 4 better centers by
  // averaging the endpoints, so maybe that's worth a shot sometime.  I did notice some
  // decreases in quality from having taken this out in the cases where it was helping (edges
  // of mountains look more polygonal at a given threshold).
  u0DirScalar = 1.0f;// - fabs(u0DirScalar);
  u1DirScalar = 1.0f;// - fabs(u1DirScalar);
  v0DirScalar = 1.0f;// - fabs(v0DirScalar);
  v1DirScalar = 1.0f;// - fabs(v1DirScalar);

  // We'll be dividing by this, so we need to make sure it never gets too small.
  if (distance2 < 0.01f)
  {
    distance2 = 0.01f;
  }
  
  // Now we use that along with the arctangent value we have to calculate a scalar
  // that approximates the attenuation of the size of a vector on the screen.
  // The idea is that, if we draw a right triangle where one leg is the vector from the
  // camera to the center, and the hypotenuse is a ray lying on the frustum, we have a
  // triangle where the non-linear vectors lie approximately on the second leg,
  // which is a rough approximation since that would suggest the extreme case where
  // the non-linear term is perpendicular to the vector from the camera to the center,
  // but it's close enough.  Then, this value that we're calculating right now is the
  // inverse of twice the length of the third leg (so that's the inverse of the span
  // of the screen), and so by multiplying the lengths of the nonlinear terms by this,
  // we have an approximation of their size on the screen.
  //
  // The keen of eye will notice that that's kind of a lie, since it's actually the inverse
  // of twice the (span of the screen)^2, and we're using the square of the magnitude of 
  // the non-linear terms, so we have a quadratic falloff in detail over distance.  In fact,
  // this is even more wrong because the size of a triangle on the screen is roughly
  // proportional to its edge length squared, so the falloff should be based on the square
  // root of the distance from the camera.  But it looks good enough, and two square roots
  // would really cramp my style.
  //
  // Additionally, it's pretty clear that the resulting values are going to be really
  // small: most nonlinearity is all of 0.0002 of the screen.  So we scale this scalar
  // up a bit (by making the numerator 1000.0) so our results are a little bigger.
  float thirdLeg = 2.0f * distance2 * tanHalfFov;

  // After a tweak job, I want it to fall off faster, so we square the distance-based term yet again.
  float invThirdLeg = 1000.0f / (thirdLeg * thirdLeg);

  u0Mag2ScreenSpace = u0Mag2 * invThirdLeg * u0DirScalar;
  u1Mag2ScreenSpace = u1Mag2 * invThirdLeg * u1DirScalar;
  v0Mag2ScreenSpace = v0Mag2 * invThirdLeg * v0DirScalar;
  v1Mag2ScreenSpace = v1Mag2 * invThirdLeg * v1DirScalar;

//  std::cout << "invThirdLeg = " << invThirdLeg << std::endl;
//  std::cout << "u0Mag2 = " << u0Mag2 << " and u0Mag2ScreenSpace = " << u0Mag2ScreenSpace << std::endl;
//  std::cout << "u1Mag2 = " << u1Mag2 << " and u1Mag2ScreenSpace = " << u1Mag2ScreenSpace << std::endl;
//  std::cout << "v0Mag2 = " << v0Mag2 << " and v0Mag2ScreenSpace = " << v0Mag2ScreenSpace << std::endl;
//  std::cout << "v1Mag2 = " << v1Mag2 << " and v1Mag2ScreenSpace = " << v1Mag2ScreenSpace << std::endl;

  float threshold = this->threshold * 2;

//  if (u0Mag2 <= threshold && u1Mag2 <= threshold && v0Mag2 <= threshold && v1Mag2 <= threshold)
  if (u0Mag2ScreenSpace <= threshold && u1Mag2ScreenSpace <= threshold && 
       v0Mag2ScreenSpace <= threshold && v1Mag2ScreenSpace <= threshold)
  {
    return;    
  }

  // We know we're going to tessellate.
  PatchPoint* u0;
  PatchPoint* u1;
  PatchPoint* v0;
  PatchPoint* v1;
  PatchPoint* centerPoint;

  // Make sure we have this many free and then re-get our corners so we don't lose them to reallocation
  // shift.
  reserveVerts(5);
  corner0 = &actualVerts[vertices[corner0Index]];
  corner1 = &actualVerts[vertices[corner1Index]];
  corner2 = &actualVerts[vertices[corner2Index]];
  corner3 = &actualVerts[vertices[corner3Index]];

  // Now finish the points off, since we know we've gotta do all this work.
  // Find the actual points for the edge points, interpolate the other second derivatives,
  // then use those to find the cross second derivatives
  if (vertices[v0Index] == -1)
  {
    vertices[v0Index] = addNewVertex();
    v0 = &actualVerts[vertices[v0Index]];
    v0->u2v0 = v0u2v0;

    // Point itself.
    v0->u0v0 = corner0->u0v0;
    v0->u0v0 += corner1->u0v0;
    v0->u0v0 *= 0.5f;
    v0->u0v0 -= nlv_v0;

    // Double second derivative.
    v0->u2v2 = corner0->u2v2;
    v0->u2v2 += corner1->u2v2;
    v0->u2v2 *= 0.5f;
    nlu_v0 = v0->u2v2;
    nlu_v0 *= delta2;

    // Cross second derivative.
    v0->u0v2 = corner0->u0v2;
    v0->u0v2 += corner1->u0v2;
    v0->u0v2 *= 0.5f;
    v0->u0v2 -= nlu_v0;

    // U and V interpolation.
    REALTEX(vertices[v0Index])[0] = (REALTEX(vertices[corner0Index])[0] + REALTEX(vertices[corner1Index])[0]) * 0.5f;
    REALTEX(vertices[v0Index])[1] = (REALTEX(vertices[corner0Index])[1] + REALTEX(vertices[corner1Index])[1]) * 0.5f;
  }

  if (vertices[v1Index] == -1)
  {
    vertices[v1Index] = addNewVertex();
    v1 = &actualVerts[vertices[v1Index]];
    v1->u2v0 = v1u2v0;

    // Point itself.
    v1->u0v0 = corner3->u0v0;
    v1->u0v0 += corner2->u0v0;
    v1->u0v0 *= 0.5f;
    v1->u0v0 -= nlv_v1;

    // Double second derivative.
    v1->u2v2 = corner2->u2v2;
    v1->u2v2 += corner3->u2v2;
    v1->u2v2 *= 0.5f;
    nlu_v1 = v1->u2v2;
    nlu_v1 *= delta2;

    // Cross second derivative.
    v1->u0v2 = corner2->u0v2;
    v1->u0v2 += corner3->u0v2;
    v1->u0v2 *= 0.5f;
    v1->u0v2 -= nlu_v1;

    // U and V interpolation.
    REALTEX(vertices[v1Index])[0] = (REALTEX(vertices[corner2Index])[0] + REALTEX(vertices[corner3Index])[0]) * 0.5f;
    REALTEX(vertices[v1Index])[1] = (REALTEX(vertices[corner2Index])[1] + REALTEX(vertices[corner3Index])[1]) * 0.5f;
  }

  if (vertices[u0Index] == -1)
  {
    vertices[u0Index] = addNewVertex();
    u0 = &actualVerts[vertices[u0Index]];
    u0->u0v2 = u0u0v2;

    // Point itself.
    u0->u0v0 = corner2->u0v0;
    u0->u0v0 += corner0->u0v0;
    u0->u0v0 *= 0.5f;
    u0->u0v0 -= nlu_u0;

    // Double second derivative.
    u0->u2v2 = corner0->u2v2;
    u0->u2v2 += corner2->u2v2;
    u0->u2v2 *= 0.5f;
    nlv_u0 = u0->u2v2;
    nlv_u0 *= delta2;

    // Cross second derivative.
    u0->u2v0 = corner0->u2v0;
    u0->u2v0 += corner2->u2v0;
    u0->u2v0 *= 0.5f;
    u0->u2v0 -= nlv_u0;

    // U and V interpolation.
    REALTEX(vertices[u0Index])[0] = (REALTEX(vertices[corner0Index])[0] + REALTEX(vertices[corner2Index])[0]) * 0.5f;
    REALTEX(vertices[u0Index])[1] = (REALTEX(vertices[corner0Index])[1] + REALTEX(vertices[corner2Index])[1]) * 0.5f;
  }

  if (vertices[u1Index] == -1)
  {
    vertices[u1Index] = addNewVertex();
    u1 = &actualVerts[vertices[u1Index]];
    u1->u0v2 = u1u0v2;

    // Point itself.
    u1->u0v0 = corner1->u0v0;
    u1->u0v0 += corner3->u0v0;
    u1->u0v0 *= 0.5f;
    u1->u0v0 -= nlu_u1;

    // Double second derivative.
    u1->u2v2 = corner1->u2v2;
    u1->u2v2 += corner3->u2v2;
    u1->u2v2 *= 0.5f;
    nlv_u1 = u1->u2v2;
    nlv_u1 *= delta2;

    // Cross second derivative.
    u1->u2v0 = corner1->u2v0;
    u1->u2v0 += corner3->u2v0;
    u1->u2v0 *= 0.5f;
    u1->u2v0 -= nlv_u1;

    // U and V interpolation.
    REALTEX(vertices[u1Index])[0] = (REALTEX(vertices[corner1Index])[0] + REALTEX(vertices[corner3Index])[0]) * 0.5f;
    REALTEX(vertices[u1Index])[1] = (REALTEX(vertices[corner1Index])[1] + REALTEX(vertices[corner3Index])[1]) * 0.5f;
  }

  v0 = &actualVerts[vertices[v0Index]];
  v1 = &actualVerts[vertices[v1Index]];
  u0 = &actualVerts[vertices[u0Index]];
  u1 = &actualVerts[vertices[u1Index]];

  // Great!  We've filled the edge points.  Now, use those to find the center point information.
  if (vertices[centerPointIndex] == -1)
  {
    vertices[centerPointIndex] = addNewVertex();
    centerPoint = &actualVerts[vertices[centerPointIndex]];
  
    // Derivative along v.
    centerPoint->u0v2 = v0->u0v2;
    centerPoint->u0v2 += v1->u0v2;
    centerPoint->u0v2 *= 0.5f;
    nl_cen = centerPoint->u0v2;
    nl_cen *= delta2;

    // Point itself
    centerPoint->u0v0 = v0->u0v0;
    centerPoint->u0v0 += v1->u0v0;
    centerPoint->u0v0 *= 0.5f;
    centerPoint->u0v0 -= nl_cen;

    // Double second derivative
    centerPoint->u2v2 = u0->u2v2;
    centerPoint->u2v2 += u1->u2v2;
    centerPoint->u2v2 *= 0.5f;

    // Derivative along u.
    centerPoint->u2v0 = u0->u2v0;
    centerPoint->u2v0 += u1->u2v0;
    centerPoint->u2v0 *= 0.5f;

    // U and V interpolation.
    REALTEX(vertices[centerPointIndex])[0] = (REALTEX(vertices[corner0Index])[0] + REALTEX(vertices[corner3Index])[0]) * 0.5f;
    REALTEX(vertices[centerPointIndex])[1] = (REALTEX(vertices[corner0Index])[1] + REALTEX(vertices[corner3Index])[1]) * 0.5f;
  }

  REALPOINT(vertices[v0Index])[0] = v0->u0v0.getX();
  REALPOINT(vertices[v0Index])[1] = v0->u0v0.getY();
  REALPOINT(vertices[v0Index])[2] = v0->u0v0.getZ();

  REALPOINT(vertices[v1Index])[0] = v1->u0v0.getX();
  REALPOINT(vertices[v1Index])[1] = v1->u0v0.getY();
  REALPOINT(vertices[v1Index])[2] = v1->u0v0.getZ();

  REALPOINT(vertices[u0Index])[0] = u0->u0v0.getX();
  REALPOINT(vertices[u0Index])[1] = u0->u0v0.getY();
  REALPOINT(vertices[u0Index])[2] = u0->u0v0.getZ();

  REALPOINT(vertices[u1Index])[0] = u1->u0v0.getX();
  REALPOINT(vertices[u1Index])[1] = u1->u0v0.getY();
  REALPOINT(vertices[u1Index])[2] = u1->u0v0.getZ();

  REALPOINT(vertices[centerPointIndex])[0] = centerPoint->u0v0.getX();
  REALPOINT(vertices[centerPointIndex])[1] = centerPoint->u0v0.getY();
  REALPOINT(vertices[centerPointIndex])[2] = centerPoint->u0v0.getZ();

  // Recurse for the 4 sub-patches.
  tessellateSubPatch(corner0Index, v0Index, u0Index, centerPointIndex, delta*0.5f, depth+1);
  tessellateSubPatch(v0Index, corner1Index, centerPointIndex, u1Index, delta*0.5f, depth+1);
  tessellateSubPatch(u0Index, centerPointIndex, corner2Index, v1Index, delta*0.5f, depth+1);
  tessellateSubPatch(centerPointIndex, u1Index, v1Index, corner3Index, delta*0.5f, depth+1);
}

void CentralPatchTessellator::draw(bool multitexture) const
{
  // Store this for drawPatch to refer to.
  attemptMultitexture = multitexture;

  // We don't re-tessellate here, just draw what we've got.
  int corners[4];
  corners[3] = getMaxXMaxYIndex();
  corners[2] = getMinXMaxYIndex();
  corners[1] = getMaxXMinYIndex();
  corners[0] = getMinXMinYIndex();

  // Okay, draw each vertex.
  ::glPolygonMode(GL_FRONT_AND_BACK, getMode());

  // If we're multitexturing, setup our arrays accordingly.
  if (OpenGL::getNumMultiTextures() > 1 && attemptMultitexture)
  {
    OpenGL::glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, actualTexCoords);
    OpenGL::glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, actualTexCoords);
  }
  else
  {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, actualTexCoords);
  }
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, actualPoints);
//  glDisableClientState(GL_NORMAL_ARRAY);

  // If it supports compiled vertex arrays, by all means, use them.
  if (OpenGL::getSupportsCompiledVertexArrays())
  {
    OpenGL::glLockArraysEXT(0,vertIndex);
  }

  // Do the recursive drawing.
  drawPatch(corners[0], corners[1], corners[2], corners[3]);

  // Clean up after ourselves.
  if (OpenGL::getSupportsCompiledVertexArrays())
  {
    OpenGL::glUnlockArraysEXT();
  }

  // Make sure we leave it in the right state!
  if (OpenGL::getNumMultiTextures() > 1 && attemptMultitexture)
  {
    OpenGL::glClientActiveTextureARB(GL_TEXTURE0_ARB);
  }
}

void CentralPatchTessellator::drawPatch(int corner0Index, int corner1Index, 
                                        int corner2Index, int corner3Index) const
{
  int centerPointIndex = (corner0Index + corner3Index) / 2;

  if (corner0Index == corner1Index - 1 || vertices[centerPointIndex] == -1)
  {
    unsigned int indices[4] = {vertices[corner0Index], vertices[corner1Index], 
                               vertices[corner3Index], vertices[corner2Index]};

    // Draw this thing as a trifan for some tiny modicum of efficiency gain.
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, indices);

    return;
  }
  
  // These are the points along the edges, and the center point.
  int u0Index = (corner0Index + corner2Index) / 2;
  int u1Index = (corner1Index + corner3Index) / 2;
  int v0Index = (corner0Index + corner1Index) / 2;
  int v1Index = (corner2Index + corner3Index) / 2;

  // Recurse for the 4 sub-patches.
  drawPatch(corner0Index, v0Index, u0Index, centerPointIndex);
  drawPatch(v0Index, corner1Index, centerPointIndex, u1Index);
  drawPatch(u0Index, centerPointIndex, corner2Index, v1Index);
  drawPatch(centerPointIndex, u1Index, v1Index, corner3Index);
}

#define COPYPOINT(x,y) \
REALPOINT(x)[0] = REALPOINT(y)[0]; \
REALPOINT(x)[1] = REALPOINT(y)[1]; \
REALPOINT(x)[2] = REALPOINT(y)[2]; 

#define COPYTEX(x,y) \
REALTEX(x)[0] = REALTEX(y)[0]; \
REALTEX(x)[1] = REALTEX(y)[1];

void CentralPatchTessellator::fixCracks(int corner0Index, int corner1Index,
                                        int corner2Index, int corner3Index) const
{
  int centerPointIndex = (corner0Index + corner3Index) / 2;

  if (corner0Index == corner1Index - 1 || vertices[centerPointIndex] == -1)
  {
    return;
  }
  
  PatchPoint* centerPoint = &actualVerts[vertices[centerPointIndex]];

  // These are the points along the edges, and the center point.
  // Note that we find their addresses by averaging the addresses of the corners.
  int u0Index = (corner0Index + corner2Index) / 2;
  int u1Index = (corner1Index + corner3Index) / 2;
  int v0Index = (corner0Index + corner1Index) / 2;
  int v1Index = (corner2Index + corner3Index) / 2;
  PatchPoint* u0 = &actualVerts[vertices[u0Index]];
  PatchPoint* u1 = &actualVerts[vertices[u1Index]];
  PatchPoint* v0 = &actualVerts[vertices[v0Index]];
  PatchPoint* v1 = &actualVerts[vertices[v1Index]];

  // These are the center points of the neighboring subpatches
  int u0NeighborIndex = u0Index + u0Index - centerPointIndex;
  int u1NeighborIndex = u1Index + u1Index - centerPointIndex;
  int v0NeighborIndex = v0Index + v0Index - centerPointIndex;
  int v1NeighborIndex = v1Index + v1Index - centerPointIndex;
  
  // We need these to make sure our "neighbor" checks don't wrap rows.  For v, the index
  // will be either negative or greater than our array size if it "wraps", but for u, it'll
  // just be on a row above or below its neighbor.
  int u0VIndex = u0Index / numEdgeVerts;
  int u1VIndex = u1Index / numEdgeVerts;
  bool u0NeighborValid = ((u0NeighborIndex / numEdgeVerts) == u0VIndex);
  bool u1NeighborValid = ((u1NeighborIndex / numEdgeVerts) == u1VIndex);

  // Now, first make sure the center points are within our array.  If so, check to see if
  // they're valid.  If not, it means they didn't tessellate, so we have to drop the
  // corresponding vertex into the edge.
  int arraySize = numEdgeVerts * numEdgeVerts;

  if (u0NeighborValid && u0NeighborIndex >= 0 && u0NeighborIndex < arraySize &&
       vertices[u0NeighborIndex] == -1)
  {
    COPYPOINT(vertices[u0Index], vertices[corner2Index]); 
    COPYTEX(vertices[u0Index], vertices[corner2Index]);
  }
  if (u1NeighborValid && u1NeighborIndex >= 0 && u1NeighborIndex < arraySize &&
       vertices[u1NeighborIndex] == -1)
  {
    COPYPOINT(vertices[u1Index], vertices[corner1Index]);
    COPYTEX(vertices[u1Index], vertices[corner1Index]);
  }
  if (v0NeighborIndex >= 0 && v0NeighborIndex < arraySize &&
       vertices[v0NeighborIndex] == -1)
  {
    COPYPOINT(vertices[v0Index], vertices[corner1Index]);
    COPYTEX(vertices[v0Index], vertices[corner1Index]);
  }
  if (v1NeighborIndex >= 0 && v1NeighborIndex < arraySize &&
       vertices[v1NeighborIndex] == -1)
  {
    COPYPOINT(vertices[v1Index], vertices[corner2Index]);
    COPYTEX(vertices[v1Index], vertices[corner2Index]);
  }

  // Recurse for the 4 sub-patches.
  fixCracks(corner0Index, v0Index, u0Index, centerPointIndex);
  fixCracks(v0Index, corner1Index, centerPointIndex, u1Index);
  fixCracks(u0Index, centerPointIndex, corner2Index, v1Index);
  fixCracks(centerPointIndex, u1Index, v1Index, corner3Index);
}

int CentralPatchTessellator::getMinXMinYIndex() const
{
  return 0;
}

int CentralPatchTessellator::getMinXMaxYIndex() const
{
  return (numEdgeVerts*numEdgeVerts)-numEdgeVerts;
}

int CentralPatchTessellator::getMaxXMinYIndex() const
{
  return numEdgeVerts-1;
}

int CentralPatchTessellator::getMaxXMaxYIndex() const
{
  return (numEdgeVerts*numEdgeVerts)-1;
}

int CentralPatchTessellator::getNumEdgeVerts() const
{
  return numEdgeVerts;
}

void CentralPatchTessellator::fixInterPatchCracks(int x0, int y0, int x1, int y1, 
                                                  int x0That, int y0That, int x1That, int y1That,
                                                  const CentralPatchTessellator* that) const
{
  // First, find the direction we're heading.
  bool xDirection = false;
  
  if (x1 != x0)
  {
    assert(x0That != x1That);
    assert(y0 == y1);
    assert(y0That == y1That);
    xDirection = true;
  }
  else if (y1 != y0)
  {
    assert(y0That != y1That);
    assert(x0 == x1);
    assert(x0That == x1That);
    xDirection = false;
  }

  // If we've hit our limit, return.
  if (xDirection)
  {
    if (abs(x1-x0) == 1)
    {
      return;
    }
  }
  else
  {
    if (abs(y1-y0) == 1)
    {
      return;
    }
  }

  // Get absolute indices for that.
  int corner0IndexThis = x0 + (numEdgeVerts*y0);
  int corner1IndexThis = x1 + (numEdgeVerts*y1);
  int corner0IndexThat = x0That + (that->numEdgeVerts*y0That);
  int corner1IndexThat = x1That + (that->numEdgeVerts*y1That);

  // Find the middle vertices.
  int cornerMidThis = (corner0IndexThis + corner1IndexThis)/2;
  int cornerMidThat = (corner0IndexThat + corner1IndexThat)/2;

  // Possible situations for this are that either both of us have valid points,
  // neither of us do, or one of us does.

  // If neither of us do, we're done, return.
  if (vertices[cornerMidThis] == -1 && that->vertices[cornerMidThat] == -1)
  {
    return;
  }

  // If one of us does, but the other doesn't, we need to collapse the vertex of the
  // one that does into its corner.
  if (vertices[cornerMidThis] == -1 && that->vertices[cornerMidThat] != -1)
  {
    // Collapse that's vertex into its corner.
    (that->actualPoints + that->vertices[cornerMidThat]*3)[0] = (that->actualPoints + that->vertices[corner1IndexThat]*3)[0];
    (that->actualPoints + that->vertices[cornerMidThat]*3)[1] = (that->actualPoints + that->vertices[corner1IndexThat]*3)[1];
    (that->actualPoints + that->vertices[cornerMidThat]*3)[2] = (that->actualPoints + that->vertices[corner1IndexThat]*3)[2];
    (that->actualTexCoords + that->vertices[cornerMidThat]*2)[0] = (that->actualTexCoords + that->vertices[corner1IndexThat]*2)[0];
    (that->actualTexCoords + that->vertices[cornerMidThat]*2)[1] = (that->actualTexCoords + that->vertices[corner1IndexThat]*2)[1];
  }
  else if (vertices[cornerMidThis] != -1 && that->vertices[cornerMidThat] == -1)
  {
    // Collapse this's vertex into the corner.
    COPYPOINT(vertices[cornerMidThis], vertices[corner0IndexThis]);
    COPYTEX(vertices[cornerMidThis], vertices[corner0IndexThis]);
  }

  // So, at this point, either (a) one of us was valid and the other wasn't, or (b) both
  // of us were valid.  Either way, we need to recurse on the top and bottom.
  if (xDirection)
  {
    int xMidThis = (x0 + x1)/2;
    int xMidThat = (x0That + x1That) / 2;
    fixInterPatchCracks(x0, y0, xMidThis, y0, x0That, y0That, xMidThat, y0That, that);
    fixInterPatchCracks(xMidThis, y0, x1, y0, xMidThat, y0That, x1That, y0That, that);
  }
  else
  {
    int yMidThis = (y0 + y1)/2;
    int yMidThat = (y0That + y1That) / 2;
    fixInterPatchCracks(x0, y0, x0, yMidThis, x0That, y0That, x0That, yMidThat, that);
    fixInterPatchCracks(x0, yMidThis, x0, y1, x0That, yMidThat, x0That, y1That, that);
  }
}

// Here we allocate the maximum possible space needed for our new depth value.
void CentralPatchTessellator::setMaxDepth(int newVal) const
{
  BezierPatchTessellator::setMaxDepth(newVal);

  // The number of vertices along an edge is 2^depth + 1
  numEdgeVerts = 1;
  numEdgeVerts <<= getMaxDepth();
  numEdgeVerts++;

  delete[] vertices;
  vertices = new int[numEdgeVerts * numEdgeVerts]; 
  memset(vertices, -1, numEdgeVerts * numEdgeVerts * sizeof(int));
}

int CentralPatchTessellator::addNewVertex() const
{
  // Double our array when we need more room to amortize it.
  if (vertIndex >= vertCapacity)
  {
    PatchPoint* newVerts = new PatchPoint[2*vertCapacity];
    float* newPoints = new float[2*vertCapacity*3];
    float* newTexCoords = new float[2*vertCapacity*2];

    memcpy(newVerts    , actualVerts    , vertIndex * sizeof(PatchPoint));
    memcpy(newPoints   , actualPoints   , vertIndex * sizeof(float) * 3);
    memcpy(newTexCoords, actualTexCoords, vertIndex * sizeof(float) * 2);

    delete[] actualVerts;
    delete[] actualPoints;
    delete[] actualTexCoords;

    actualVerts = newVerts;
    actualPoints = newPoints;
    actualTexCoords = newTexCoords;

    vertCapacity = 2*vertCapacity;
  }

  // Increment it; return the currently available one.
  vertIndex++;
  return vertIndex - 1;
}

void CentralPatchTessellator::reserveVerts(int n) const
{
  // Double our array when we need more room to amortize it.
  while (vertIndex + n - 1 >= vertCapacity)
  {
    PatchPoint* newVerts = new PatchPoint[2*vertCapacity];
    float* newPoints = new float[2*vertCapacity*3];
    float* newTexCoords = new float[2*vertCapacity*2];

    memcpy(newVerts    , actualVerts    , vertIndex * sizeof(PatchPoint));
    memcpy(newPoints   , actualPoints   , vertIndex * sizeof(float) * 3);
    memcpy(newTexCoords, actualTexCoords, vertIndex * sizeof(float) * 2);

    delete[] actualVerts;
    delete[] actualPoints;
    delete[] actualTexCoords;

    actualVerts = newVerts;
    actualPoints = newPoints;
    actualTexCoords = newTexCoords;

    vertCapacity = 2*vertCapacity;
  }
}
