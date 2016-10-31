#include <harness/OpenGL.h>
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

GLenum OpenGL::GL_TEXTURE_MAX_ANISOTROPY_EXT = 0x84fe;
GLenum OpenGL::GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0x84ff;
bool OpenGL::supportsAnisotropicTextures = false;

GLenum OpenGL::GL_TEXTURE_FILTER_CONTROL_EXT = 0x8500;
GLenum OpenGL::GL_TEXTURE_LOD_BIAS_EXT = 0x8501;
bool OpenGL::supportsTextureLodBias = false;

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

bool OpenGL::getSupportsAnisotropicTextures()
{
  return supportsAnisotropicTextures;
}

bool OpenGL::getSupportsTextureLodBias()
{
  return supportsTextureLodBias;
}

void OpenGL::initExtensions()
{
  std::string extensions = std::string((char*)glGetString(GL_EXTENSIONS));

  if (extensions.find(std::string("GL_ARB_multitexture")) < extensions.max_size())
  {
    std::cout << "Found ARB_multitexture" << std::endl;
    glMultiTexCoord2fARB = (GLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
    glActiveTextureARB = (GLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
    glClientActiveTextureARB = (GLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
    glGetIntegerv(GL_MAX_TEXTURES_UNITS_ARB, &numMultiTextures);
  }

  if (extensions.find(std::string("GL_EXT_compiled_vertex_array")) < extensions.max_size())
  {
    std::cout << "Found EXT_compiled_vertex_array" << std::endl;
    glLockArraysEXT = (GLLOCKARRAYSEXTPROC)wglGetProcAddress("glLockArraysEXT");
    glUnlockArraysEXT = (GLUNLOCKARRAYSEXTPROC)wglGetProcAddress("glUnlockArraysEXT");
    supportsCompiledVertexArrays = true;
  }

  if (extensions.find(std::string("GL_EXT_texture_edge_clamp")) < extensions.max_size())
  {
    std::cout << "Found EXT_texture_edge_clamp" << std::endl;
    supportsClampToEdge = true;
  }

  if (extensions.find(std::string("GL_EXT_texture_filter_anisotropic")) < extensions.max_size())
  {
    std::cout << "Found EXT_texture_filter_anisotropic" << std::endl;
    supportsAnisotropicTextures = true;
  }

  if (extensions.find(std::string("GL_EXT_texture_lod_bias")) < extensions.max_size())
  {
    std::cout << "Found EXT_texture_lod_bias" << std::endl;
    supportsTextureLodBias = true;
  }
}