#ifndef __gluquat__
#define __gluquat__


#if defined (WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>


// Quaternion Structure
// quaternion is represented as (w,[x,y,z])
// where: w       - scalar part
//        x, y, z - vector part
typedef struct tag_GL_QUAT { 
  GLfloat w, x, y, z;
} GL_QUAT;



// define APIENTRY and CALLBACK to null string if we aren't on Win32
#if !defined(WIN32)
#define APIENTRY
#define CALLBACK
#endif

#ifdef __cplusplus
extern "C" {
#endif


extern void APIENTRY gluQuatToMat_EXT(GL_QUAT *, GLfloat m[4][4]);
extern void APIENTRY gluEulerToQuat_EXT(GLfloat, GLfloat, GLfloat, GL_QUAT * );
extern void APIENTRY gluMatToQuat_EXT(GLfloat m[4][4], GL_QUAT *);
extern void APIENTRY gluQuatSlerp_EXT(GL_QUAT * , GL_QUAT * , GLfloat, GL_QUAT *);
extern void APIENTRY gluQuatLerp_EXT(GL_QUAT *, GL_QUAT *, GLfloat, GL_QUAT *);
extern void APIENTRY gluQuatNormalize_EXT(GL_QUAT *);
extern void APIENTRY gluQuatGetValue_EXT(GL_QUAT*, GLfloat*, GLfloat*, GLfloat*, GLfloat*);
extern void APIENTRY gluQuatSetValue_EXT(GL_QUAT *, GLfloat, GLfloat, GLfloat, GLfloat);
extern void APIENTRY gluQuatScaleAngle_EXT(GL_QUAT *, GLfloat);
extern void APIENTRY gluQuatInverse_EXT(GL_QUAT *);
extern void APIENTRY gluQuatSetFromAx_EXT(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, 
										  GLfloat, GL_QUAT *);
extern void APIENTRY gluQuatMul_EXT(GL_QUAT*, GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatAdd_EXT(GL_QUAT*, GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatSub_EXT(GL_QUAT*, GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatDiv_EXT(GL_QUAT*, GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatCopy_EXT(GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatSquare_EXT(GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatSqrt_EXT(GL_QUAT*, GL_QUAT*);
extern GLfloat APIENTRY gluQuatDot_EXT(GL_QUAT*, GL_QUAT*);
extern GLfloat APIENTRY gluQuatLength_EXT(GL_QUAT*);
extern void APIENTRY gluQuatNegate_EXT(GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatExp_EXT(GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatLog_EXT(GL_QUAT*, GL_QUAT*);
extern void APIENTRY gluQuatLnDif_EXT(GL_QUAT*, GL_QUAT*, GL_QUAT*);







#ifdef __cplusplus
}

#endif


#endif  // __gluquat__

