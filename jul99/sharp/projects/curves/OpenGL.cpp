#include <curves/OpenGL.h>
#include <gl/glut.h>
#include <string>
#include <iostream>

GLMULTITEXCOORD2FARBPROC OpenGL::glMultiTexCoord2fARB = NULL;
GLACTIVETEXTUREARBPROC OpenGL::glActiveTextureARB = NULL;
GLCLIENTACTIVETEXTUREARBPROC OpenGL::glClientActiveTextureARB = NULL;
int OpenGL::numMultiTextures = 1;

GLLOCKARRAYSEXTPROC OpenGL::glLockArraysEXT = NULL;
GLUNLOCKARRAYSEXTPROC OpenGL::glUnlockArraysEXT = NULL;
bool OpenGL::supportsCompiledVertexArrays = false;

GLenum OpenGL::GL_CLAMP_TO_EDGE = 0x812F;
bool OpenGL::supportsClampToEdge = false;

int OpenGL::getNumMultiTextures()
{
  return numMultiTextures;
}

bool OpenGL::getSupportsCompiledVertexArrays()
{
  return supportsCompiledVertexArrays;
}

bool OpenGL::getSupportsClampToEdge()
{
  return supportsClampToEdge;
}

void OpenGL::initExtensions()
{
  std::string extensions = std::string((char*)glGetString(GL_EXTENSIONS));

  if (extensions.find(std::string("GL_ARB_multitexture")) < extensions.max_size())
  {
    std::cout << "Using ARB_multitexture" << std::endl;
    glMultiTexCoord2fARB = (GLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
    glActiveTextureARB = (GLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
    glClientActiveTextureARB = (GLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
    glGetIntegerv(GL_MAX_TEXTURES_UNITS_ARB, &numMultiTextures);
  }

  if (extensions.find(std::string("GL_EXT_compiled_vertex_array")) < extensions.max_size())
  {
    std::cout << "Using EXT_compiled_vertex_array" << std::endl;
    glLockArraysEXT = (GLLOCKARRAYSEXTPROC)wglGetProcAddress("glLockArraysEXT");
    glUnlockArraysEXT = (GLUNLOCKARRAYSEXTPROC)wglGetProcAddress("glUnlockArraysEXT");
    supportsCompiledVertexArrays = true;
  }

  if (extensions.find(std::string("GL_EXT_texture_edge_clamp")) < extensions.max_size())
  {
    std::cout << "Using EXT_texture_edge_clamp" << std::endl;
    supportsClampToEdge = true;
  }
}