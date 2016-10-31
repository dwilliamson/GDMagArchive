#include <curves/bezier/BezierPatchTessellator.h>

BezierPatchTessellator::BezierPatchTessellator()
{
  granularity = 0.8f;
  polyMode = GL_FILL;
  fixCracks = true;
  maxDepth = 3;
}

BezierPatchTessellator::BezierPatchTessellator( const BezierPatchTessellator& source )
{
  granularity = source.granularity;
  polyMode = source.polyMode;
  fixCracks = source.fixCracks;
  maxDepth = source.maxDepth;
}

BezierPatchTessellator& BezierPatchTessellator::operator=( const BezierPatchTessellator& source )
{
  granularity = source.granularity;
  polyMode = source.polyMode;
  fixCracks = source.fixCracks;
  maxDepth = source.maxDepth;

  return *this;
}

void BezierPatchTessellator::setGranularity( float newVal )
{
  granularity = newVal;
}

float BezierPatchTessellator::getGranularity() const
{
  return granularity;
}

void BezierPatchTessellator::setMode( GLenum newVal )
{
  polyMode = newVal;
}

GLenum BezierPatchTessellator::getMode() const
{
  return polyMode;
}

void BezierPatchTessellator::setFixCracks( bool newVal )
{
  fixCracks = newVal;
}

bool BezierPatchTessellator::getFixCracks() const
{
  return fixCracks;
}

void BezierPatchTessellator::setMaxDepth( int newVal ) const
{
  maxDepth = newVal;
}

int BezierPatchTessellator::getMaxDepth() const
{
  return maxDepth;
}