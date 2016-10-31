#ifndef OPENGL_H
#define OPENGL_H

#include <harness/PlatformSpecific.h>
#include <gl/glut.h>

// EXT_compiled_vertex_array
typedef void (APIENTRY * GLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef void (APIENTRY * GLUNLOCKARRAYSEXTPROC) ();

// ARB_multitexture
#define GL_MAX_TEXTURES_UNITS_ARB           0x84E2
#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1
typedef void (APIENTRY * GLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * GLACTIVETEXTUREARBPROC) (GLenum target);
typedef void (APIENTRY * GLCLIENTACTIVETEXTUREARBPROC) (GLenum target);

// This class handles management of OpenGL-related stuff like extensions and their function
// pointers.
class OpenGL
{
public:
  // Multitexture function pointers.
  static GLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
  static GLACTIVETEXTUREARBPROC glActiveTextureARB;
  static GLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
  
  // Compiled vertex array function pointers.
  static GLLOCKARRAYSEXTPROC glLockArraysEXT;
  static GLUNLOCKARRAYSEXTPROC glUnlockArraysEXT;

  // For support of clamp_to_edge in texturing.
  static GLenum GL_CLAMP_TO_EDGE;

  static int getNumMultiTextures();
  static bool getSupportsCompiledVertexArrays();
  static bool getSupportsClampToEdge();

  static void initExtensions();

protected:
  static int numMultiTextures;
  static bool supportsCompiledVertexArrays;
  static bool supportsClampToEdge;
};

#endif //OPENGL_H