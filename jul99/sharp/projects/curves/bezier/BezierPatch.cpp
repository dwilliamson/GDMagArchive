#include <curves/bezier/BezierPatch.h>
#include <curves/CurvePoint.h>
#include <curves/OpenGL.h>

#include <iostream>

// Ugly STL warnings, go away!
#pragma warning (disable: 4786)

BezierPatch::BezierPatch()
{
  textureName = 0;
  lightmapName = 0;
  useLightmap = true;
  useTexture = true;
}

BezierPatch::~BezierPatch()
{
  ::glDeleteTextures(1, &lightmapName);
}

void BezierPatch::setControlPoints(const std::vector< std::vector< CurvePoint > >& pts)
{
  // Set our control points to this.
  controls = pts;

  // We have to regenerate our basis functions to take the new points into account.
  generateBases();
}

void BezierPatch::clearControlPoints()
{
  // Wipe us out (presumably, they'll fill us up again, or else we're not too useful).
  controls.clear();
  basesU.clear();
  basesV.clear();
}

// This explicitly calculates a value of the patch.
float BezierPatch::getValueAt(float x, float y)
{
  if (controls.size() == 0)
  {
    return 0;
  }

  float curPt[3] = {0,0,0};
  float u = x - controls[0][0].getX();
  float v = y - controls[0][0].getY();
  u /= controls[3][0].getX() - controls[0][0].getX();
  v /= controls[0][3].getY() - controls[0][0].getY();

  // Generate a point on the curve for this step.
  for (int i=0; i<controls.size(); i++)
  {
    for (int j=0; j<controls[0].size(); j++)
    {
      // Get a few basis function values and products thereof that we'll need.
      float bu = basesU[i](u);
      float bv = basesV[j](v);
      float bu_bv = bu * bv;

      // Add this control point's contribution onto the current point.
      curPt[2] += controls[i][j].getZ() * bu_bv;
    }
  }

  return curPt[2];
}

// This explicitly calculates a normal on the patch.
Vector BezierPatch::getNormalAt(float x, float y)
{
  if (controls.size() == 0)
  {
    return 0;
  }

  // First, we need to make the basis functions for our normals, which just involves
  // taking the derivative of each basis function.
  std::vector< BasisFunction > basesUdu(basesU);
  std::vector< BasisFunction > basesVdv(basesV);

  for (int i = 0; i < basesUdu.size(); i++)
  {
    basesUdu[i].differentiate();
  }
  for (int j = 0; j < basesVdv.size(); j++)
  {
    basesVdv[j].differentiate();
  }

  // This holds the point we're working on as we add control points' contributions to it.
  float curNorm[3] = { 0, 0, 0 };
  float u = x - controls[0][0].getX();
  float v = y - controls[0][0].getY();
  u /= controls[3][0].getX() - controls[0][0].getX();
  v /= controls[0][3].getY() - controls[0][0].getY();

  float curUTan[3] = { 0, 0, 0 };
  float curVTan[3] = { 0, 0, 0 };

  // Generate a point on the curve for this step.
  for (i = 0; i < controls.size(); i++)
  {
    for (j = 0; j < controls[0].size(); j++)
    {
      // Get a few basis function values and products thereof that we'll need.
      float bu = basesU[i](u);
      float bv = basesV[j](v);
      float dbu = basesUdu[i](u);
      float dbv = basesVdv[j](v);
      float bu_dbv = bu * dbv;
      float dbu_bv = dbu * bv;

      // Add this point's contribution to our u-tangent.
      curUTan[0] += controls[i][j].getX() * dbu_bv;
      curUTan[1] += controls[i][j].getY() * dbu_bv;
      curUTan[2] += controls[i][j].getZ() * dbu_bv;

      // Add this point's contribution to our v-tangent.
      curVTan[0] += controls[i][j].getX() * bu_dbv;
      curVTan[1] += controls[i][j].getY() * bu_dbv;
      curVTan[2] += controls[i][j].getZ() * bu_dbv;
    }
  }

  // Now get our normal as the cross-product of the u and v tangents.
  curNorm[0] = curUTan[1] * curVTan[2] - curUTan[2] * curVTan[1];
  curNorm[1] = curUTan[2] * curVTan[0] - curUTan[0] * curVTan[2];
  curNorm[2] = curUTan[0] * curVTan[1] - curUTan[1] * curVTan[0];

  // Normalize our normal (ouch!)
  float rInv = 1.0f / sqrt(curNorm[0] * curNorm[0] + curNorm[1] * curNorm[1] + curNorm[2] * curNorm[2]);
  curNorm[0] *= rInv;
  curNorm[1] *= rInv;
  curNorm[2] *= rInv;

  return Vector(curNorm[0], curNorm[1], curNorm[2]);
}

// This explicitly calculates a gradient on the patch.
Vector BezierPatch::getGradientAt(float x, float y)
{
  if (controls.size() == 0)
  {
    return 0;
  }

  // First, we need to make the basis functions for our normals, which just involves
  // taking the derivative of each basis function.
  std::vector< BasisFunction > basesUdu(basesU);
  std::vector< BasisFunction > basesVdv(basesV);

  for (int i = 0; i < basesUdu.size(); i++)
  {
    basesUdu[i].differentiate();
  }
  for (int j = 0; j < basesVdv.size(); j++)
  {
    basesVdv[j].differentiate();
  }

  // This holds the point we're working on as we add control points' contributions to it.
  float curNorm[3] = { 0, 0, 0 };
  float u = x - controls[0][0].getX();
  float v = y - controls[0][0].getY();
  u /= controls[3][0].getX() - controls[0][0].getX();
  v /= controls[0][3].getY() - controls[0][0].getY();

  float curUTan[2] = { 0, 0 };
  float curVTan[2] = { 0, 0 };

  // Generate a point on the curve for this step.
  for (i = 0; i < controls.size(); i++)
  {
    for (j = 0; j < controls[0].size(); j++)
    {
      // Get a few basis function values and products thereof that we'll need.
      float bu = basesU[i](u);
      float bv = basesV[j](v);
      float dbu = basesUdu[i](u);
      float dbv = basesVdv[j](v);
      float bu_dbv = bu * dbv;
      float dbu_bv = dbu * bv;

      // Add this point's contribution to our u-tangent.  
      // We only care about x and z here.
      curUTan[0] += controls[i][j].getX() * dbu_bv;
      curUTan[1] += controls[i][j].getZ() * dbu_bv;

      // Add this point's contribution to our v-tangent.
      // We only care about y and z here.
      curVTan[0] += controls[i][j].getY() * bu_dbv;
      curVTan[1] += controls[i][j].getZ() * bu_dbv;
    }
  }

  // Now scale them so they're unit in x and y.
  curUTan[1] *= 1.0f / curUTan[0];
  curVTan[1] *= 1.0f / curVTan[0];

  // Now we have the gradient in either direction.  Build our gradient out of that.
  float gradMag = curUTan[1]*curUTan[1] + curVTan[1]*curVTan[1];
  return Vector(curUTan[1], curVTan[1], gradMag);
}

void BezierPatch::getVectorInGradientPlane(const Vector& grad, Vector& xyNeedsZ)
{
  xyNeedsZ.z = grad.x*xyNeedsZ.x + grad.y*xyNeedsZ.y;
}

void BezierPatch::getLightmapAt(float x, float y, unsigned int& texName, float& u, float& v)
{
  if (controls.size() == 0)
  {
    return;
  }

  u = x - controls[0][0].getX();
  v = y - controls[0][0].getY();
  u /= controls[3][0].getX() - controls[0][0].getX();
  v /= controls[0][3].getY() - controls[0][0].getY();

  texName = lightmapName;
}

// These are used to modify the points without actually changing their number.
int BezierPatch::getNumPointsU() const
{
  return controls.size();
}

int BezierPatch::getNumPointsV() const
{
  if (controls.size() > 0)
  {
    return controls[0].size();
  }

  return 0;
}

CurvePoint* BezierPatch::getControlPoint(int u, int v)
{
  if (0 <= u && u < controls.size() && 0 <= v && v < controls[0].size())
  {
    return &(controls[u][v]);
  }
  
  return 0;
}

const CurvePoint* BezierPatch::getControlPoint(int u, int v) const
{
  if (0 <= u && u < controls.size() && 0 <= v && v < controls[0].size())
  {
    return &(controls[u][v]);
  }
  
  return 0;
}

// To get a feel for how fast this'd be if we were bending the curve every frame, we
// regenerate our curve every frame.
void BezierPatch::tessellate(const BezierPatchTessellator* drawer) const
{
  // Don't tessellate if we're degenerate
  if (controls.size() == 0)
  {
    return;
  }

  drawer->tessellate(controls, basesU, basesV);
}

void BezierPatch::draw(const BezierPatchTessellator* drawer) const
{
  float pointColor[3] = {0.0, 0.5, 0.0};
  float curveColor[3] = {1.0, 1.0, 1.0};
  
  // Preliminary stuff:
  glDisable(GL_LIGHTING);

  // If we're drawing both of them, do it in one pass if we have multitexturing.
  if (OpenGL::getNumMultiTextures() > 1 && getUseLightmap() && getUseTexture())
  {
    glDisable(GL_BLEND);

    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, lightmapName);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Tell the patch to draw.  Tell it to multitexture.
    drawer->draw(true);
  }

  // Make sure we don't leave the second unit on, as that can elicit a bug in nVidia's driver.
  if (OpenGL::getNumMultiTextures() > 1)
  {
    OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
  }

  // Now, if we can't multitexture, or we only want to draw one of the passes, do that here.
  if (OpenGL::getNumMultiTextures() <= 1 || getUseLightmap() == false || getUseTexture() == false)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Turn on texturing and load in the lightmap and draw the surface.
    if (getUseLightmap()) {
      glEnable(GL_TEXTURE_2D);

      // Draw us in two passes.  First pass (lightmap):
      glBindTexture(GL_TEXTURE_2D, lightmapName);
      glDepthFunc(GL_LESS);

      // Tell the patch to draw.
      drawer->draw(false);

      // Now set up the blending and z-buffering for the base texture pass, assuming we're
      // doing two-pass drawing.
      glEnable(GL_BLEND);
      glBlendFunc(GL_DST_COLOR, GL_ZERO);
      glDepthFunc(GL_EQUAL);
    }

    // Now we're drawing the base texture, either as a second pass or by itself.
    if (getUseTexture()) {
      glEnable(GL_TEXTURE_2D);

      glBindTexture(GL_TEXTURE_2D, textureName);

      // Again, draw.
      drawer->draw(false);
    }

    // Now clean up.
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthFunc(GL_LESS);
  }
}


// We only draw our U bases, since most often we'll be a square patch anyway.
void BezierPatch::drawBasisFunctions() const
{
  ::glColor3f(1, 1, 1);

  for (int x = 0; x < basesU.size(); x++)
  {
    ::glBegin(GL_LINE_STRIP);
    for (int step = 0; step <= 80; step++)
    {
      double t = ((double)step) / 80.0;

      ::glVertex2f(t, basesU[x](t));
    }
    ::glEnd();
  }
}

// This is used to regenerate our basis functions if the number of control points changes.
void BezierPatch::generateBases()
{
  // Quick check: are we a degenerate patch?  If so, return now.
  if (controls.size() == 0)
  {
    return;
  }

  // This is straightforward: just generate the bezier bases b(i.n) where i is in the range
  // [0,n], and n is (one less than) the number of control points (one less because it's indexed
  // from 0).
  basesU.clear();
  int n = controls.size() - 1;

  for (int i = 0; i <= n; i++)
  {
    basesU.push_back(BasisFunction(i, n));
  }

  basesV.clear();
  n = controls[0].size() - 1;

  for (i = 0; i <= n; i++)
  {
    basesV.push_back(BasisFunction(i, n));
  }
}

void BezierPatch::generateLightmap(const UniformPatchTessellator* tess, int size)
{
  glDeleteTextures(1, &lightmapName);
  lightmapName = tess->generateLightmap(size, controls, basesU, basesV);
}
void BezierPatch::setTexture(unsigned int newVal)
{
  glDeleteTextures(1, &textureName);
  textureName = newVal;
}

unsigned int BezierPatch::getLightmap() const
{
  return lightmapName;
}

unsigned int BezierPatch::getTexture() const
{
  return textureName;
}

void BezierPatch::setUseLightmap(bool newVal)
{
  useLightmap = newVal;
}

void BezierPatch::setUseTexture(bool newVal)
{
  useTexture = newVal;
}

bool BezierPatch::getUseLightmap() const
{
  return useLightmap;
}

bool BezierPatch::getUseTexture() const
{
  return useTexture;
}
