#include <curves/bezier/BezierCurveTessellator.h>

BezierCurveTessellator::BezierCurveTessellator()
{
  granularity = 1.0f;
  curveMode = GL_LINE_STRIP;
}

BezierCurveTessellator::BezierCurveTessellator( const BezierCurveTessellator& source )
{
  granularity = source.granularity;
  curveMode = source.curveMode;
}

BezierCurveTessellator& BezierCurveTessellator::operator=( const BezierCurveTessellator& source )
{
  granularity = source.granularity;
  curveMode = source.curveMode;

  return *this;
}

void BezierCurveTessellator::setGranularity( float newVal )
{
  granularity = newVal;
}

float BezierCurveTessellator::getGranularity() const
{
  return granularity;
}

void BezierCurveTessellator::setMode( GLenum newVal )
{
  curveMode = newVal;
}

GLenum BezierCurveTessellator::getMode() const
{
  return curveMode;
}

