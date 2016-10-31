#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "demo.h"
//#include "glext.h"
#include "vec3f.h"
#include "psdparse.h"
#include "include/shared/nvparse.h"
#include "noise.h"

//#define PREMUL

#define GLH_EXT_SINGLE_FILE
#include "glh_extensions.h"

#pragma warning(disable: 4305)
#pragma warning(disable: 4244)



const char *last_error;
void check(void)
{
   GLenum z = glGetError();
   if (z != GL_NO_ERROR) {
      const char *s = glGetString(z);
      last_error = s;
   }
}

void nvcheck(void)
{
   char const * const *q = nvparse_get_errors();
   if (q && q[0]) {
      assert(0);
   }
}

void square_tex0(void)
{
   nvparse("!!RC1.0\n"
       "{ rgb { tex0 = tex0 * tex0; } }\n"
       "out.rgb = tex0 * col0; out.a = col0;"
       ,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
}


#define LERP(res,a,b,c)  "{ discard = "   #a   " * unsigned_invert("   #c   ");" \
                         "  discard = "   #b   " * "   #c   "; " \
                          #res   " = sum(); }"

#define ALERP(res,a,b,c) "{ discard = "   #a   ";" \
                         "  discard = "   #b   " * "   #c   "; " \
                          #res   " = sum(); }"

enum
{
   LM_none,
   LM_diffuse,
   LM_specular,
} lm;

void bumpmap_diffuse(void)
{
   char *z;
   lm = LM_diffuse;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 1: base texture
      "texture_2d();"        // surface map 2: bomb decal texture
      "texture_2d();"        // surface map 3: bomb control texture
      "texture_cube_map();"  // normalized L/H
      ,0);

   nvcheck();

   z = 
      "!!RC1.0\n"
      /* bomb N from tex0         */ "{ rgb "   LERP(tex0,tex0,tex1,tex2.a)   " }\n"
      /* prepare to normalize N'  */ "{ rgb { spare0 = expand(tex0) . expand(tex0); } }\n"
      /* finish normalizing N'    */ "{ rgb { discard = expand(tex0); discard = half_bias(tex0) * unsigned_invert(spare0); tex0 = sum(); } }\n"
      /* compute N' dot L         */ "{ rgb { tex0 = tex0 . expand(tex3); } }\n"
      /* multiply by light color  */ "out.rgb = tex0 * col0; out.a = col0;"
      ;
   nvparse(z,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

int normalize;
void bumpmap_diffuse2(void)
{
   char *z;
   lm = LM_diffuse;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 1: bump texture 1
      "texture_2d();"        // surface map 2: bump texture 2
      "texture_2d();"        // surface map 3: notused
      "texture_cube_map();"  // normalized L/H
      ,0);

   nvcheck();

   z = 
      "!!RC1.0\n"

      "{   \n"
      "   const0 = ( 0.5,0.5,0.5,0.5); \n"
      "   rgb "  LERP(tex0,tex0,tex1,const0.a)  " \n"  // average bump textures
      "   alpha { discard = tex0; discard = -tex1; spare0 = sum(); } \n " //spare0 = tex0 - tex1
      "}\n"

      "{  \n"
      "   const0 = ( 0.5,0.5,0.5,0.5); \n"
      "   rgb   { spare0 = expand(tex0) . expand(tex0); }\n" // prepare to normalize N'
      "   alpha { discard = spare0; discard = const0; spare0 = sum(); }\n" // spare0 = tex0 - tex1 + 0.5
      "}\n"

      "{  \n"      
               /* finish normalizing N'    */ 
      "   rgb { discard = expand(tex0); discard = half_bias(tex0) * unsigned_invert(spare0); tex0 = sum(); } \n"
      "   alpha { discard = tex0; discard = tex1; tex0 = mux(); }\n"  // get min of tex0 and tex1
      "}\n"

      "{  \n"
      "   const0 = (0.5,0.5,0.5,1.0); \n"
      "   rgb { tex0 = tex0 . expand(tex3); } \n"  // N' dot L
      "   alpha { discard = const0 * tex0; discard = unsigned_invert(const0); tex0 = sum(); }\n"
      "}\n"
      /* shadow by height         */ "final_product = tex0 * tex0.a;"
      /* multiply by light color  */ "out.rgb = final_product * col0; out.a = col0;"
      ;

   nvparse(z,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

void bumpmap_specular(void)
{
   char *z;
   lm = LM_specular;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 1: base texture
      "texture_2d();"        // surface map 2: bomb decal texture
      "texture_2d();"        // surface map 3: bomb control texture
      "texture_cube_map();"  // normalized L/H
      ,0);

   nvcheck();

   z = 
      "!!RC1.0\n"
//      /* bomb N from tex0         */ "{ rgb "   LERP(tex0,tex0,tex1,tex2.a)   " }\n"
//      /* prepare to normalize N'  */ "{ rgb { spare0  = expand(tex0) . expand(tex0); } }\n"
//      /* finish normalizing N'    */ "{ rgb { discard = expand(tex0); discard = half_bias(tex0) * unsigned_invert(spare0); tex0 = sum(); } }\n"
      /* compute N' dot H         */ "{ rgb { tex0    = expand(tex0) . expand(tex3); } }\n"
      /* square                   */ "{ rgb { tex0    = unsigned(tex0) * unsigned(tex0); } }\n"
      /* square                   */ "{ rgb { tex0    = unsigned(tex0) * unsigned(tex0); } }\n"
      /* square                   */ "{ rgb { tex0    = unsigned(tex0) * unsigned(tex0); } }\n"
                                     "final_product = tex0 * tex0;\n"
      /* multiply by light color  */ "out.rgb = final_product * col0; out.a = col0;"
      ;
   nvparse(z,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

void colorize_brick(void)
{
   char *z;

   lm = LM_none;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 0: base brick surface texture
      "texture_2d();"        // surface map 1: mortar texture
      "texture_2d();"        // surface map 2: control texture
      ,0);

   nvcheck();

   z = 
      "!!RC1.0\n"
      "{ const0 = ( 0.8, 0.8, 0.8, 1.0 ); rgb " LERP(tex0,tex0,const0,tex2.a) " alpha { tex1 = tex1 * const0; } }\n"   // desaturate, fade out mortar for testing
      "{ rgb { tex0 = tex0 * tex2; } }\n"             // colorize/darken
      "{ rgb " LERP(tex0,tex0,tex1,tex1.a) " }\n"     // apply mortar
      "out.rgb = tex0 * col0; out.a = col0;\n"
      ;
   nvparse(z,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

void decal_overlay(void)
{
   char *z;

   lm = LM_none;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 0: base
      "texture_2d();"        // surface map 1: decal
      "texture_2d();"        // surface map 2: decal control
      ,0);

   nvcheck();

   z = 
      "!!RC1.0\n"
      "{ rgb { tex0 = tex0 * col0; } alpha  { tex1 = tex2 * tex1; } } \n"
      "{ rgb "  LERP(tex0,tex0,tex1,tex1.a)  " }\n"
      "out.rgb = tex0; out.a = col0;\n"
      ;
   nvparse(z,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

void special_decal_overlay(void)
{
   char *z;

   lm = LM_none;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 0: base
      "texture_2d();"        // surface map 1: decal
      "texture_2d();"        // surface map 2: decal control
      "texture_2d();"        // surface map 3: decal alpha map
      ,0);

   nvcheck();

   z = 
      "!!RC1.0\n"
      "{ rgb { tex0 = tex0 * col0; } alpha  { tex3 = tex2 * tex3; } } \n"
      "{ rgb "  LERP(tex0,tex0,tex1,tex3.a)  " }\n"
      "out.rgb = tex0; out.a = col0;\n"
      ;
   nvparse(z,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

void parking_lot(void)
{
   lm = LM_none;

   nvparse(
      "!!TS1.0"
      "texture_2d();"        // surface map 0: base asphalt surface texture
      "texture_2d();"        // surface map 1: painted lines texture
      "texture_2d();"        // surface map 2: control texture
      "texture_2d();"        // surface map 2: large-scale noise texture
      ,0);

   nvcheck();

   nvparse(
      "!!RC1.0\n"
      "{ rgb { tex2 = tex1 . tex2; } alpha { tex0 = tex1 * tex2; } }\n"  // compute decals
      "{ const0 = ( 0.8,0.8,0.8,0.8); rgb { tex0 = tex0 * tex3; } alpha { tex2.a = tex2.b * const0; } }\n"
      "{ rgb { tex0 = tex0 * unsigned_invert(tex0.a); } }\n"
      "{ rgb { discard = tex0 * unsigned_invert(tex2.a); discard = tex2.a; tex0 = sum(); } }\n"
      "out.rgb = tex0 * col0; out.a = col0;\n"
      ,0);

   nvcheck();

   glEnable(GL_REGISTER_COMBINERS_NV);
   glEnable(GL_TEXTURE_SHADER_NV);
}

void disable_advanced(void)
{
   glDisable(GL_REGISTER_COMBINERS_NV);
   glDisable(GL_TEXTURE_SHADER_NV);
   lm = LM_none;
}


#define NUM_RANDOM   77

static unsigned short rand15(void)
{
   static int remain=0;
   static unsigned short data[NUM_RANDOM];
   int which, result;

   if (remain == 0) {
      while (remain < NUM_RANDOM)
         data[remain++] = rand();
   }
   which = (rand() >> 3) % remain;
   result = data[which];
   data[which] = data[--remain];
   return result;
}

static unsigned int rand32(void)
{
   return (rand15() << 16) ^ (rand15() << 8) ^ rand15();
}

float frand(void)
{
   return rand15() / 32768.0;
}


#define PI     3.141592

float fov   = 90;
float znear = 0.5;  // feet?
float zfar  = 4000;

static int scr_w, scr_h;

void demoResizeViewport(int width, int height )
{
  scr_w = width;
  scr_h = height;

  glViewport(0, 0, width, height);
  glScissor(0,0,width,height);
}

void setProjection(void)
{
   double  aspect, view_width;
   float zoom = 1.0;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   aspect = (GLdouble)scr_w/(GLdouble)scr_h;

   view_width = tan(fov/2.0*(PI/180))/zoom;
   gluPerspective(atan(view_width/aspect)*2*(180/PI), aspect, znear, zfar); 
   glMatrixMode(GL_MODELVIEW);
   check();
}

extern void dummyTexture(void);

//typedef void *(APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (GLint size, float, float, float);

//static PFNGLACTIVETEXTUREARBPROC        glActiveTextureARB;
//static PFNGLCLIENTACTIVETEXTUREARBPROC  glClientActiveTextureARB;
//static PFNGLMULTITEXCOORD2FARBPROC      glMultiTexCoord2fARB;
//static PFNGLMULTITEXCOORD2FVARBPROC     glMultiTexCoord2fvARB;
//static PFNGLMULTITEXCOORD3FARBPROC      glMultiTexCoord3fARB;
//static PFNGLMULTITEXCOORD3FVARBPROC     glMultiTexCoord3fvARB;
//static PFNGLTEXIMAGE3DPROC              glTexImage3DEXT;

#define GPA(x)   do_gpa((void (**)(void)) &x, #x)

static void do_gpa(void (**func)(void), char *str)
{
   if (*func == NULL)
      *func = (void (*)(void)) wglGetProcAddress(str);
   assert(*func != NULL);
}

void openglInit(void)
{
   glClearColor(0.2,0,0,0);
   glClearDepth(1.0);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glShadeModel(GL_SMOOTH);

   glAlphaFunc(GL_GEQUAL, 0.1f);
   glDisable(GL_ALPHA_TEST);

   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glEnable(GL_DEPTH_TEST);
   glDepthMask(TRUE);
   glDepthFunc(GL_LEQUAL);

   glEnable(GL_TEXTURE_2D);

   glFrontFace(GL_CW);
   glEnable(GL_CULL_FACE);

   GPA(glActiveTextureARB);
   GPA(glClientActiveTextureARB);
   GPA(glMultiTexCoord2fARB);
   GPA(glMultiTexCoord2fvARB);
   GPA(glMultiTexCoord3fARB);
   GPA(glMultiTexCoord3fvARB);

   GPA(glTexImage3DEXT);
   check();
}

void screenInit(int width, int height)
{
   openglInit();
   demoResizeViewport(width, height);
   check();
}

void setupCamera(vec3f *loc, vec3f *ang)
{
   setProjection();
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   // we start looking along negative z, but we want to look along positive y
   glRotatef(-90, 1,0,0);
   glRotatef(-ang->y,  0,1,0);
   glRotatef(-ang->x,  1,0,0);
   glRotatef(-ang->z,  0,0,1);
   glTranslatef(-loc->x, -loc->y, -loc->z);
   check();
}

vec3f camera_loc = { 12,12,102 };/*/{ 2,-460,200 };/**/
vec3f camera_ang = { -10,0,180+2 }; /*/{ 0,0,180-35 };/**/
vec3f camera_lod_loc;

void loadTexture(int x, char *name, bool invert_alpha)
{
   PSDbitmap *res = loadPSD(name);
   int i;
   if (invert_alpha)
      for (i=0; i < res->w * res->h; ++i)
         res->data[i] = (res->data[i] & 0xffffff) + (0xff000000 - (res->data[i] & 0xff000000));
   glBindTexture(GL_TEXTURE_2D, x);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, res->w, res->h, GL_RGBA, GL_UNSIGNED_BYTE, res->data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
   check();
   free(res);
}

void makeTexture(int x, int w, int h, uint32 *data)
{
   glBindTexture(GL_TEXTURE_2D, x);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
}

void loadNormalTexture(int t, char *name, int scale, bool hack)
{
   PSDbitmap *res = loadPSD(name);
   int x,y;
   uint32 *data = malloc(sizeof(*data) * res->w * res->h);

   for (y=0; y < res->h; ++y) {
      for (x=0; x < res->w; ++x) {
         int h00,h01,h10,r,g,b;
         vec3f v;
         h00 = res->data[y*res->w+x] & 255;
         h01 = res->data[y*res->w + ((x+1) & (res->w-1))] & 255;
         h10 = res->data[((y+1) & (res->h-1))*res->w + x] & 255;
         v.x = h01 - h00;
         v.y = h10 - h00;
         v.z = scale;
         vec3f_normeq(&v);
         if (hack) v.z -= 1; // hack it so that it produces vector of length 1 if added to (0,0,1)
         r = 128 + 127 * v.x;
         g = 128 + 127 * v.y;
         b = 128 + 127 * v.z;
         data[y*res->w+x] = (b << 16) + (g << 8) + r;
      }
   }
   glBindTexture(GL_TEXTURE_2D, t);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, res->w, res->h, GL_RGBA, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
   check();
   free(res);
   free(data);
}

// load a normal texture, but include a weighting for how far down it is in the alpha channel
void loadNormalTextureHeight(int t, char *name, int scale, bool hack)
{
   PSDbitmap *res = loadPSD(name);
   int x,y;
   uint32 *data = malloc(sizeof(*data) * res->w * res->h);

   for (y=0; y < res->h; ++y) {
      for (x=0; x < res->w; ++x) {
         int h00,h01,h10,r,g,b;
         vec3f v;
         h00 = res->data[y*res->w+x] & 255;
         h01 = res->data[y*res->w + ((x+1) & (res->w-1))] & 255;
         h10 = res->data[((y+1) & (res->h-1))*res->w + x] & 255;
         v.x = h01 - h00;
         v.y = h10 - h00;
         v.z = scale;
         vec3f_normeq(&v);
         if (hack) v.z -= 1; // hack it so that it produces vector of length 1 if added to (0,0,1)
         r = 128 + 127 * v.x;
         g = 128 + 127 * v.y;
         b = 128 + 127 * v.z;
         data[y*res->w+x] = (b << 16) + (g << 8) + r + (h00 << 24);
      }
   }
   glBindTexture(GL_TEXTURE_2D, t);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, res->w, res->h, GL_RGBA, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
   check();
   free(res);
   free(data);
}

#if 0
void loadExcitedNormalTexture(int t, char *name, int scale)
{
   PSDbitmap *res = loadPSD(name);
   int x,y;
   uint32 *data = malloc(sizeof(*data) * res->w * res->h);

   // load a specially formatted normal texture:
   //   H = slope angle
   //   S = slope steepness
   //   A = overlaid height field (trenches)

   for (y=0; y < res->h; ++y) {
      for (x=0; x < res->w; ++x) {
         int h00,h01,h10,r,g,b;
         vec3f n;
         {
            float h,s;
            int q,v;
            r = res->data[y*res->w+x] & 255;
            g = (res->data[y*res->w+x] >> 8) & 255;
            b = (res->data[y*res->w+x] >> 16) & 255;

            q = (r < g ? r : g); if (b < q) q = b;
            v = (r > g ? r : g); if (b > v) v = b;
            if (v == q) {
               n.x=0;
               n.y=0;
               n.z=1;
            } else {
               float angle;
               int f = (r==q) ? g-b : (g==q) ? b-r : r-g;
               int i = (r==q) ? 3   : (g==q) ? 5   : 1;
               h = i - (float) f / (v-q);
               s = (float) (v-q)/v;
               angle = -h*360/6 + 180;
               angle = angle/180 * 3.141592;
               n.x = s;
               n.y = 0;
               n.z = 2.5;
               vec3f_normeq(&n);
               n.y = sin(angle) * n.x;
               n.x = cos(angle) * n.x;
            }
         }

         h00 = res->data[y*res->w+x] >> 24;
         if (h00 != 255) {
            vec3f m;
            h01 = res->data[y*res->w + ((x+1) & (res->w-1))] >> 24;
            h10 = res->data[((y+1) & (res->h-1))*res->w + x] >> 24;
            m.x = h01 - h00;
            m.y = h10 - h00;
            m.z = scale;
            vec3f_normeq(&m);
            #define THRESH 220
            if (h00 < THRESH)
               n = m;
            else {
               vec3f_interpolate(&n, &m, &n, (h00 - THRESH) / (float) (255-THRESH));
               vec3f_normeq(&n);
            }
         }
         assert(n.z > 0);
         r = 128 + 127 * n.x;
         g = 128 + 127 * n.y;
         b = 128 + 127 * n.z;
         data[y*res->w+x] = (b << 16) + (g << 8) + r;
      }
   }
   glBindTexture(GL_TEXTURE_2D, t);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, res->w, res->h, GL_RGBA, GL_UNSIGNED_BYTE, data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
   check();
   free(res);
   free(data);
}
#endif

float pgauss(void)
{
   // faux gaussian
   return frand() - frand();
}

void randomBricks(int n, bool bump)
{
   uint32 (*data)[256] = malloc(sizeof(uint32) * 256*256);
   int x,y;
   for (y=0; y < 256; ++y) {
      for (x=(y&1); x < 256; x += 2) {
         int r,g,b,a;
         // pick a basic color...
         float wacky = rand15() / 32768.0 * 100;
         if (bump) {
            r=g=b=a=0;
            if (wacky < 18)
               a = 80 + 20 * (pgauss()+1);
            //r=g=b=a;
         } else {
            if (wacky < 1) {
               r = g = b = 128; a=40;
            } else if (wacky < 1.5) {
               r = g = b = 255-frand()*20; a=0; // full bright!
            } else {
               float lum = pgauss() * 0.1 + 0.8;
               r = (pgauss() * 0.03 + 1) * lum * 255;
               g = (pgauss() * 0.03 + 1) * lum * 255;
               b = (pgauss() * 0.03 + 1) * lum * 255;
               a = fabs(pgauss()) * 64;
               if (wacky < 2) a = frand() * 128;
            }
         }
         assert(0 <= r && r < 256);
         assert(0 <= g && g < 256);
         assert(0 <= b && b < 256);
         assert(0 <= a && a < 256);
         data[y][x] = data[y][(x+1)&255] = 
            (a << 24) + (g << 16) + (b << 8) + r;
      }
   }
   makeTexture(n, 256, 256, data[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // !!!
   free(data);
}

void randomParkingLot(int n)
{
   int x0 = 0;
   int w  = 195;
   int spacing = 18;
   int x1 = x0 + (w/spacing)*spacing;
   int y_base = 64;
   int y_extent = 36;
   uint8 (*data)[256][4] = malloc(sizeof(uint32) * 128*256);
   uint8 (*data2)[512][4] = malloc(sizeof(uint32) * 256*512);
   int x,y,i;
   memset(data, 0, sizeof(uint32)*128*256);
   #if 0
   for (y=0; y < 128; ++y)
      for (x=0; x < 256; ++x) {
         data[y][x][0] = 0;
         data[y][x][1] = 0;
         if (((y & 7) == 3 || (y & 7) == 4) && (x & 1)==0)
            data[y][x][2] = 0;
         else
            data[y][x][2] = 0;
         data[y][x][3] = 0;
      }
   //memset(data, 50, sizeof(uint32)*128*256);
   #endif

   // paint the horizontal lines
   for (x=x0; x <= x1; ++x) {
      data[y_base][x][1] = 255;
   }
   for (x=x0; x <= x1; x += spacing) {
      for (y=-y_extent; y < y_extent; ++y) {
         data[y_base+y][x][0] = 255;
      }
      #if 0
      // paintline terminators... this is done specially in the supersampled version
      data[y_base-y_extent][x][2] = 255;
      data[y_base+y_extent-1][x][2] = 255;
      #endif
   }
   // now add random blobs of white (paint?)
   for (y=0; y < 128; ++y)
      for (x=0; x < 256; ++x)
         // don't use paintline terminators
         if (((y & 7) != 3 && (y & 7) != 4) || (x & 1)) {
            // even squares have high probability;
            // odd squares have low
            if (((x^y)&1)==0 ? rand() < 1500 : rand() < 800)
                data[y][x][2] = 230 * (pgauss() + 1)/2;
         }
   // now add blobs of black (oil etc.)
   // this should be done more intelligently, by hand, so
   // that they're in parking spaces and there are clumps
   // of them in convincing patterns, etc.
   for (y=0; y < 64; ++y)
      for (x=0; x < 128; ++x)
         if (((x^y)&1)==0 ? rand() < 400 : rand() < 1500) {
            int c = 210*frand();
            data[y*2+0][x*2+0][3] = c;
            data[y*2+0][x*2+1][3] = c;
            data[y*2+1][x*2+0][3] = c;
            data[y*2+1][x*2+1][3] = c;
         }  
   #if 0
   makeTexture(n, 256, 128, (uint32 *) data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // !!!
   #else
   // now make it 2x as large, with smooth edges; without doing
   // this, it aliases horribly, even disabling the nearest filter above
   for (y=0; y < 128; ++y)
      for (x=0;x < 256; ++x)
         for (i=0; i < 4; ++i) {
            data2[y*2+0][x*2+0][i] = data[y][x][i];
            data2[y*2+0][x*2+1][i] = data[y][x][i];
            data2[y*2+1][x*2+0][i] = data[y][x][i];
            data2[y*2+1][x*2+1][i] = data[y][x][i];
         }
   for (x=x0; x <= x1; x += spacing) {
      int y0 = y_base - y_extent-1;
      int y1 = y_base + y_extent;

      data2[y0*2+2][x*2+0][2] = 85;
      data2[y0*2+2][x*2+1][2] = 85;
      data2[y0*2+1][x*2+0][2] = 170;
      data2[y0*2+1][x*2+1][2] = 170;
      data2[y0*2+0][x*2+0][2] = 254;
      data2[y0*2+0][x*2+1][2] = 254;

      data2[y1*2-1][x*2+0][2] = 85;
      data2[y1*2-1][x*2+1][2] = 85;
      data2[y1*2+0][x*2+0][2] = 170;
      data2[y1*2+0][x*2+1][2] = 170;
      data2[y1*2+1][x*2+0][2] = 254;
      data2[y1*2+1][x*2+1][2] = 254;
   }
   for (y=1; y < 255; ++y)
      for (x=1;x < 511; ++x)
         for (i=0; i < 4; ++i) {
            if (data2[y][x][i] == 0) {
               if (i == 1) {
                  // smooth horizontals out vertically
                  if (data2[y+1][x][i]==255 || data2[y-1][x][i] == 255)
                     data2[y][x][i] = 128;
                  //else if (data2[y][x+1][i]=255 || data2[y][x-1][i]==255)
                  //   data2[y][x][i] = 254;
               } else if (i == 0) 
                  // smooth verticals out horizontally
                  if (data2[y][x+1][i]==255 || data2[y][x-1][i]==255)
                     data2[y][x][i] = 128;
                  else if (data2[y+1][x][i] == 255 || data2[y-1][x][i]==255 ||
                           data2[y+1][x-1][i] == 255 || data2[y+1][x+1][i] == 255 ||
                           data2[y-1][x-1][i] == 255 || data2[y-1][x-1][i] == 255)
                     data2[y][x][i] = 192;
            }
         }
   makeTexture(n, 512,256, (uint32 *) data2);
   #endif
   free(data);
   free(data2);
}

// this indicates what type of flower is in each square
// if adjacent numbers are identical, they are meant to
// be continuous (but can still do them however)
unsigned char flower_map[8][8] =
{
   { 1,1,0,0, 0,0,0,0, },
   { 1,1,0,0, 0,0,2,0, },
   { 0,0,0,0, 3,3,0,0, },
   { 0,0,4,4, 0,0,0,5, },
   { 0,0,0,0, 0,0,0,0, },
   { 6,0,7,0, 0,0,0,0, },
   { 0,0,0,0, 8,8,0,0, },
   { 0,0,0,0, 8,0,0,9, },
};

float noise3W(float x, float y, float z, int wrap)
{
   return ((noise3(x,y,z,wrap)/1.15)+1)/2;
}

void randomFlowers(int tex)
{
   int x,y,i;
   uint32 (*data)[128] = malloc(sizeof(uint32)*128*128);
   memset(data,0,sizeof(uint32)*128*128);
   for (y=0; y < 128; ++y) {
      for (x=0; x < 128; ++x) {
         if (flower_map[y & 7][x & 7] == 0) {
            // dandelion fields
            int i;
            float octave_weight[8] = { 3,3,3,3,1,6,0,0 };
            float freq = 1;
            float val=0;
            for (i=0; i < 6; ++i) {
               val += octave_weight[i] * noise3(x*freq+2.5,y*freq+0.5,i*2+3,128 >> i);
               freq /= 2;
            }
            if (val > 0)
               data[y][x] = 0xffffffff;
         } else {
            // random other flowers
            if (frand() < 0.02)
               data[y][x] = 0xffffffff;
         }
      }
   }
   // now place flowers that are larger than 1x1
   for (i=0; i < 81; ++i) {
      int n = (i % 9)+1;
      int x0,y0;

      // place jittered so they're not too near each other
      x0 = 128 * (i / 9 % 3) / 3;
      y0 = 128 * (i / 27   ) / 3;
      x0 += (rand() >> 3) % (128 / 5) + n * 11;
      y0 += (rand() >> 3) % (128 / 5) + n * 3;
      x0 &= 127 & ~7;
      y0 &= 127 & ~7;
      for (y=0; y < 8; ++y)
         for (x=0; x < 8; ++x)
            if (flower_map[y][x] == n)
               data[y0+y][x0+x] = 0xffffffff;
   }

   makeTexture(tex, 128, 128, data[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // !!!
   free(data);
}

void randomNoise(int tex)
{
   #define NS 64
   uint32 (*data)[NS] = malloc(sizeof(uint32)*NS*NS);
   int x,y;
   for(y=0; y < NS; ++y)
      for(x=0;x < NS; ++x) {
         float v = 1- 0.15 * noise3W(x*0.25,y*0.25,1.8,NS/4)
                 - 0.25*noise3W(x*0.125,y*0.125,13.5,NS/8);
         unsigned char r = (255 - 12 * noise3W(x*0.5,y*0.5,4.7,NS/2))*v;
         unsigned char g = (255 - 12 * noise3W(x*0.5,y*0.5,14.2,NS/2))*v;
         unsigned char b = (255 - 12 * noise3W(x*0.5,y*0.5,21.8,NS/2))*v;
         data[y][x] = 0xff000000 + (b << 16) + (g << 8) + r;
      }
   makeTexture(tex,NS,NS,(uint32*)data);
   free(data);
   #undef NS
}

void perlinNoise(int tex)
{
   const float T = 2;
   const float W = 3;
   uint32 (*data)[256] = malloc(sizeof(uint32) * 256*256);
   int x,y,i;
   for (y=0; y < 256; ++y) {
      for (x=0; x < 256; ++x) {
         unsigned char r,g,b;
         float octave_weight[8] = { 0,0,0,0,4,15,7,0 };
         float freq = 1;
         float val=0;
         for (i=0; i < 8; ++i) {
            val += octave_weight[i] * noise3(x*freq,y*freq,i*2+3,256 >> i);
            freq /= 2;
         }
         if (val < T) r = g = b = 200;
         else if (val > W) r = 255, g=b=0;
         else { val -= T; val /= (W-T); r = 200 + 55*val; g = 200*(1-val); b = 200*(1-val); }
         data[y][x] = 0xff000000 + (b << 16) + (g << 8) + r;
      }
   }
   makeTexture(tex,256,256,data[0]);
   free(data);
}

void rawPerlinNoise(float data[256][256], float seed)
{
   int x,y,i;
   for (y=0; y < 256; ++y) {
      for (x=0; x < 256; ++x) {
         float octave_weight[8] = { 0,0,0,0,4,15,7,0 };
         float freq = 1;
         float val=0;
         for (i=0; i < 8; ++i) {
            val += octave_weight[i] * noise3(x*freq,y*freq,i*2+seed,256 >> i);
            freq /= 2;
         }
         data[y][x] = val;
      }
   }
}

void makeNoiseTexture(int tex, float raw_data[256][256], float trans)
{
   uint32 (*data)[256] = malloc(sizeof(uint32) * 256*256);
   int x,y;
   for (y=0; y < 256; ++y)
      for (x=0; x < 256; ++x)
         if (raw_data[y][x] > trans)
            data[y][x] = 0xffffffff;
         else
            data[y][x] = 0;
   makeTexture(tex, 256,256,data[0]);
   free(data);
}

void makeLines(int tex)
{
   uint32 data[16][16];
   int x,y;
   for (y=0; y < 16; ++y)
      for (x=0; x < 16; ++x)
         if ((x ^ y) & 8)
            data[y][x] = 0xffffffff;
         else
            data[y][x] = 0xff000000;
   makeTexture(tex,16,16,data[0]);
}

#ifdef PREMUL
void makeGoofy(int tex1, int tex2,int tex3, int tex4)
{
   static uint32 data2[4][8][8];
   static uint32 data[4][4][4] = 
   {
      {
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 },
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 },
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 },
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 }
      },
      {
         { 0x1810ff10, 0x1810ff10, 0x1810ff10, 0x1810ff10 },
         { 0x1810ff10, 0x1810ff10, 0x1810ff10, 0x1810ff10 },
         { 0x1810ff10, 0x1810ff10, 0x1810ff10, 0x1810ff10 },
         { 0x1810ff10, 0x1810ff10, 0x1810ff10, 0x1810ff10 },
      },
      {
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 },
         { 0xff808080, 0x1810ff10, 0x1010ff10, 0xff808080 },
         { 0xff808080, 0x1810ff10, 0x1010ff10, 0xff808080 },
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 }
      },
      {
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 },
         { 0xff808080, 0x18021802, 0x18021802, 0xff808080 },
         { 0xff808080, 0x18021802, 0x18021802, 0xff808080 },
         { 0xff808080, 0xff808080, 0xff808080, 0xff808080 }
      },
   };
   int i,x,y;
   for (i=0; i < 4; ++i)
      for (y=0; y < 8; ++y)
         for (x=0; x < 8; ++x)
            data2[i][y][x] = data[i][y >> 1][x >> 1];
   makeTexture(tex1, 8, 8, data2[0][0]);
   makeTexture(tex2, 8, 8, data2[1][0]);
   makeTexture(tex3, 8, 8, data2[2][0]);
   makeTexture(tex4, 8, 8, data2[3][0]);
}

void makeFourColor(int tex)
{
   static uint32 data[2][2] =
   {
      { 0xff0000ff, 0xff00ff00 },
      { 0xff0000ff, 0x00000000 },
   };
   makeTexture(tex, 2,2, data[0]);
}

#endif

static float data1[256][256];
static float data2[256][256];

float a_trans= 0,b_trans=0;
void makeNoiseTextures(void)
{
   makeNoiseTexture(18,data1,a_trans);
   makeNoiseTexture(19,data2,b_trans);
}

static void buildNormalizingCubeMap(GLuint which);

void demoInit(void)
{
   loadTexture(1, "tex0.psd", TRUE);
   loadTexture(2, "tex1.psd", TRUE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

//   buildSpots(3);
//   buildLineSegs(4);

   // brick wall
   loadNormalTexture(5, "mortar_bump.psd", 80, FALSE);
   loadNormalTexture(8, "brick_bump_decal.psd", 20, FALSE);
   loadTexture(6, "brickbase3.psd", FALSE);
   loadTexture(7, "mortar.psd", FALSE);
   //loadTexture(9, "brick_bump_control.psd", TRUE);
   randomBricks(9,TRUE);
   randomBricks(10,FALSE);

   // parking lot
   //loadExcitedNormalTexture(11, "asphalt_bump.psd", 280);
   loadTexture(12, "asphalt_base.psd", FALSE);
   loadTexture(13, "parking_lot.psd", FALSE);
   randomParkingLot(14);
   randomNoise(15);

   // mud cracks
   loadTexture(20, "mud_base.psd", FALSE);
   loadNormalTextureHeight(21, "crack_bump.psd", 50, FALSE);
   loadNormalTextureHeight(22, "crack_bump2.psd", 50, FALSE);
   loadTexture(23, "mud_decal.psd", FALSE);

   // grass
   loadTexture(30, "grass.psd", FALSE); 
   loadTexture(31, "flowers.psd", FALSE);
   randomFlowers(32);
   loadTexture(33, "grass2.psd", FALSE);

   //perlinNoise(3);
   makeLines(4);

#ifdef PREMUL
   loadTexture(50, "jefferson.psd", FALSE);
   makeGoofy(51,52,53,54);
   makeFourColor(60);
#endif

   rawPerlinNoise(data1, 1.5);
   rawPerlinNoise(data2, 4.5);
   makeNoiseTextures();

   { uint32 dummy = 0xffffffff; makeTexture(99,1,1,&dummy); }

   buildNormalizingCubeMap(100);
   check();
}

void setTextureSet1(void)
{
   float ang = atan(1.0/9);
   // brick size is such that 9*size*cos(ang) + 1*sin(ang)*size = 1
   float size = 1 / (9*cos(ang) + 1*sin(ang));

   glActiveTextureARB(GL_TEXTURE0_ARB);
   glDisable(GL_TEXTURE_3D);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 1);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(32,32,1);
   glRotatef(ang*180/3.1415926,0,0,1);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


   glActiveTextureARB(GL_TEXTURE1_ARB);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glDisable(GL_TEXTURE_3D);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 2);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(32/size/256.0,32/size/256.0,1);
   glRotatef(0,0,0,1);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_2D);
   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_2D);

   glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ZERO);
   glEnable(GL_BLEND);
   check();
}

float x_ang = 3;//2;
float y_ang = 12;//8;

void setTextureSet2(void)
{
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_3D);
   glBindTexture(GL_TEXTURE_3D, 4);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(3,3,3);
   glRotatef(x_ang,1,0,0);
   glRotatef(y_ang,0,1,0);
   //glTranslatef(0,y_ang,x_ang*0.02);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_2D);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();

   glBlendFunc(GL_ONE, GL_ZERO);
   check();
}

static void rot(bool scale)
{
   float ang = atan(1.0 / 5);
   if (scale) {
      float size = 1.0 / (5*cos(ang) + 1*sin(ang));
      glRotatef(-ang*180/3.1415926,0,0,1);
      glScalef(size,size,1);
   } else {
      glRotatef(-ang*180/3.1415926,0,0,1);
   }
}

static void rot2(bool scale)
{
   float ang = atan(3.0 / 9);
   if (scale) {
      float size = 1.0 / (9*cos(ang) + 3*sin(ang));
      glRotatef(-ang*180/3.1415926,0,0,1);
      glScalef(size,size,1);
   } else {
      glRotatef(ang*180/3.1415926,0,0,1);
   }
}

#define SC  180 // 112

float x_tran,y_tran;

void setTextureSet3(void)
{
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_3D);
   glBindTexture(GL_TEXTURE_2D, 5);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   // adjust for finite differencing
   glTranslatef(-0.5/256,-0.5/256,0);
   glScalef(SC,SC,SC);
   rot(TRUE);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, 100);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   rot(FALSE);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 8);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   // adjust for finite differencing
   glTranslatef(-0.5/256,-0.5/256,0);
   glScalef(SC,SC,SC);
   rot2(TRUE);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 9);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glTranslatef(x_tran/256.0,y_tran/256.0,0);
   glScalef(SC/256.0,SC/256.0,1);
   // translate by half of the mortar's dimension
   glTranslatef(0.0005,0.0005,0);
}

void setTextureSet4(void)
{
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_3D);
   glBindTexture(GL_TEXTURE_2D, 6); // 6
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(20,20,20);
   glRotatef(30,0,0,1);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_3D);
   glBindTexture(GL_TEXTURE_2D, 7);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(SC,SC,SC);
   rot(TRUE);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 10);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(SC/256.0,SC/256.0,1);
   // translate by half of the mortar's dimension
   glTranslatef(0.0005,0.0005,0);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_CUBE_MAP_EXT);
}

void setTextureSet5(void)
{
   float line_scale = 60;
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 12);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(45,45,1);
   glRotatef(21,0,0,1);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 13);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(line_scale, line_scale,1);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 14);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(line_scale*8/256,2*line_scale*8/256,1);
   
   glActiveTextureARB(GL_TEXTURE3_ARB);
   glEnable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_CUBE_MAP_EXT);
   glBindTexture(GL_TEXTURE_2D, 15);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(2,2,1);
   glRotatef(70,0,0,1);

   glDisable(GL_BLEND);
   check();
}

void setTextureSet6(void)
{
   float sc = 145;
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 21);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   rot(TRUE);
   glScalef(sc,sc,1);
   glRotatef(0,0,0,1);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, 100);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 22);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(sc/4,sc/4,1);
   glRotatef(0,0,0,1);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 0);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
}

void setTextureSet7(void)
{
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 20);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(20,20,1);
   glRotatef(33,0,0,1);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 23);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(25,25,1);
   glRotatef(18,0,0,1);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 20);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(20,20,1);
   glRotatef(33,0,0,1);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_CUBE_MAP_EXT);
}

void setTextureSet8(void)
{
   float fscale=22;
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 30);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(30,0,0,1);
   glScalef(32,32,1);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 31);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(fscale,fscale,1);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 32);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(fscale/128*8,fscale/128*8,1);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_CUBE_MAP_EXT);
}

void setTextureSet9(void)
{
   float fscale=36;
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 30);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(20,0,0,1);
   glScalef(32,32,1);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 33);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(-15,0,0,1);
   glScalef(fscale,fscale,1);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 18);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(20,0,0,1);
   glScalef(32,32,1);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 19);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(-15,0,0,1);
   glScalef(fscale,fscale,1);
}

void setTextureSet10(void)
{
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 4);
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glScalef(230,230,1);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_2D);
   glActiveTextureARB(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_2D);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_2D);
}

void noTexture(void)
{
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glDisable(GL_TEXTURE_2D);

   glActiveTextureARB(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_2D);

   glActiveTextureARB(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_2D);

   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_CUBE_MAP_EXT);
}

#define multiTexCoord2f(a,b) glMultiTexCoord2fARB(GL_TEXTURE0_ARB,a,b);glMultiTexCoord2fARB(GL_TEXTURE1_ARB, a,b);glMultiTexCoord2fARB(GL_TEXTURE2_ARB, a,b);glMultiTexCoord2fARB(GL_TEXTURE3_ARB,a,b)
#define MTC2(a,b) multiTexCoord2f(a,b)

#define MTC(n,a,b) glMultiTexCoord2fARB(GL_TEXTURE0_ARB+n,a,b)
#define MT3(n,a,b,c) glMultiTexCoord3fARB(GL_TEXTURE0_ARB+n,a,b,c)

#define TS  10

static vec3f norm;
static vec3f light1 = { -250,-300,200 };
static vec3f light2 = { -3000,3000,3000 };
static vec3f light;
void vert(float x, float y, float z)
{
   if (lm == LM_diffuse) MT3(3, light.x-x, light.z - z, light.y - y);
   else if (lm == LM_specular) {
      vec3f L = { light.x-x,light.y-y,light.z-z };
      vec3f E = { camera_loc.x-x, camera_loc.y-y, camera_loc.z-z};
      vec3f_normeq(&L);
      vec3f_normeq(&E);
      vec3f_interpolate(&L, &L,&E,0.5);
      MT3(3, L.x, L.z, L.y);
      // half angle between L and eye
   }
   glVertex3f(x,y,z);
}

// tesselate to improve lighting
void draw_backwall(void)
{
   glBegin(GL_QUADS);
   {
#if 0
      MTC(0, 0,0.5); vert( 500,-500,0);
      MTC(0, 0,0.0); vert( 500,-500,500);
      MTC(0, 1,0.0); vert(-500,-500,500);
      MTC(0, 1,0.5); vert(-500,-500,0);
#else
      float s0,t0;
      for (t0=0; t0 < 1; t0 += 0.125)
      for (s0=0; s0 < 1; s0 += 0.125) {
         float s1 = s0 + 0.125;
         float t1 = t0 + 0.125;

         MTC2(s0,0.5-t0*0.5); vert(500-1000*s0,-500,500*t0);
         MTC2(s0,0.5-t1*0.5); vert(500-1000*s0,-500,500*t1);
         MTC2(s1,0.5-t1*0.5); vert(500-1000*s1,-500,500*t1);
         MTC2(s1,0.5-t0*0.5); vert(500-1000*s1,-500,500*t0);
      }
#endif
   }
   glEnd();
}

void render_world(void)
{
   //int sz=400, sz2=80, sz3=20;
   glColor3f(1,1,1);
   setTextureSet1();
   glBegin(GL_QUADS);
      MTC2(0,0.5); glVertex3f(-500,500,0);
      MTC2(0,0.0); glVertex3f(-500,500,500);
      MTC2(1,0.0); glVertex3f( 500,500,500);
      MTC2(1,0.5); glVertex3f( 500,500,0);
   glEnd();
   glDisable(GL_BLEND);

   glColor3f(1,1,1);
#if 1
   setTextureSet5();
   parking_lot();
   glBegin(GL_QUADS);
      MTC2(0,0); glVertex3f(-500,-500,0);
      MTC2(1,0); glVertex3f(-500, 500,0);
      MTC2(1,1); glVertex3f( 500, 500,0);
      MTC2(0,1); glVertex3f( 500,-500,0);
   glEnd();
#else
   setTextureSet10();
   square_tex0();
   glBegin(GL_QUADS);
      MTC2(0,0); glVertex3f(-500,-500,0);
      MTC2(1,0); glVertex3f(-500, 500,0);
      MTC2(1,1); glVertex3f( 500, 500,0);
      MTC2(0,1); glVertex3f( 500,-500,0);
   glEnd();
#endif

   setTextureSet6();
   bumpmap_diffuse2();
   light = light2;
   glColor3f(0.8,0.8,0.7);
   glBegin(GL_QUADS);
      MTC2(0,0); vert(1000 + -500,-500,0);
      MTC2(1,0); vert(1000 + -500, 500,0);
      MTC2(1,1); vert(1000 +  500, 500,0);
      MTC2(0,1); vert(1000 +  500,-500,0);
   glEnd();

   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);

#if 1
   disable_advanced();
   noTexture();
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);
   glColor3f(0.25,0.25,0.4);
   //glColor3f(1,1,1);
   glBegin(GL_QUADS);
      glVertex3f(1000 + -500,-500,0);
      glVertex3f(1000 + -500, 500,0);
      glVertex3f(1000 +  500, 500,0);
      glVertex3f(1000 +  500,-500,0);
   glEnd();

#endif

#if 1
   glEnable(GL_BLEND);
   glBlendFunc(GL_DST_COLOR, GL_ZERO);
   setTextureSet7();
   decal_overlay();
   glColor3f(1,1,1);
   glBegin(GL_QUADS);
      MTC2(0,0); vert(1000 + -500,-500,0);
      MTC2(1,0); vert(1000 + -500, 500,0);
      MTC2(1,1); vert(1000 +  500, 500,0);
      MTC2(0,1); vert(1000 +  500,-500,0);
   glEnd();
#endif

   glDisable(GL_BLEND);

   setTextureSet8();
   decal_overlay();
   glColor3f(0.85,0.85,0.85);
   glColor3f(1,1,1);
   glBegin(GL_QUADS);
      MTC2(0,0); vert(-1000 + -500,-500,0);
      MTC2(1,0); vert(-1000 + -500, 500,0);
      MTC2(1,1); vert(-1000 +  500, 500,0);
      MTC2(0,1); vert(-1000 +  500,-500,0);
   glEnd();

   glDisable(GL_BLEND);

   setTextureSet9();
   special_decal_overlay();
   glColor3f(0.9,1.0,0.9);
   glColor3f(1,1,1);
   glBegin(GL_QUADS);
      MTC2(0,0); vert(-2000 + -500,-500,0);
      MTC2(1,0); vert(-2000 + -500, 500,0);
      MTC2(1,1); vert(-2000 +  500, 500,0);
      MTC2(0,1); vert(-2000 +  500,-500,0);
   glEnd();

   setTextureSet3();
   bumpmap_diffuse();

   glColor3f(0.35,0.30,0.25);
   light = light2;
   draw_backwall();

   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);

   glColor3f(0.65,0.65,0.75);
   light = light1;
   draw_backwall();

   //disable_advanced();
   colorize_brick();

   glColor3f(1,1,1);
   setTextureSet4();
   glBlendFunc(GL_DST_COLOR, GL_ZERO);
   draw_backwall();

#if 0
   glColor3f(0.1,0.1,0.12);
   glBlendFunc(GL_ONE, GL_ONE);
   setTextureSet3();
   norm.x = norm.z = 0;
   norm.y = 1;
   bumpmap_specular();
   draw_backwall();
#endif

   disable_advanced();
   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

   check();
}

void demoDraw(void)
{

}

int show_tex=0;
void draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);

   setupCamera(&camera_loc, &camera_ang);

   render_world();

   glDisable(GL_BLEND);
   glActiveTextureARB(GL_TEXTURE3_ARB);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_3D);
   glActiveTextureARB(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_3D);
   glActiveTextureARB(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_TEXTURE_3D);
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glDisable(GL_TEXTURE_3D);

   if (show_tex) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0,scr_w,0,scr_h);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, show_tex);
      glDisable(GL_BLEND);
      #ifdef PREMUL
      glBindTexture(GL_TEXTURE_2D, 60);
      #endif
      glColor3f(1,1,1);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      #ifdef PREMUL
      glDisable(GL_TEXTURE_2D);
      glColor3f(0.5,0.5,0.5);
      glBegin(GL_POLYGON);
         glTexCoord2f(0,1); glVertex2f(0,0);
         glTexCoord2f(1,1); glVertex2f(512,0);
         glTexCoord2f(1,0); glVertex2f(512,512);
         glTexCoord2f(0,0); glVertex2f(0,512);
      glEnd();
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      glBegin(GL_POLYGON);
         glTexCoord2f(0.25,0.75); glVertex2f(0,0);
         glTexCoord2f(0.75,0.75); glVertex2f(512,0);
         glTexCoord2f(0.75,0.25); glVertex2f(512,512);
         glTexCoord2f(0.25,0.25); glVertex2f(0,512);
      glEnd();
      #else
      glBegin(GL_POLYGON);
         glTexCoord2f(0,1); glVertex2f(0,0);
         glTexCoord2f(1,1); glVertex2f(512,0);
         glTexCoord2f(1,0); glVertex2f(512,512);
         glTexCoord2f(0,0); glVertex2f(0,512);
      glEnd();
      #endif
      #ifdef PREMULx
      glEnable(GL_BLEND);
      if (show_tex == 4)
         glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      else
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBindTexture(GL_TEXTURE_2D, 50+show_tex);
      glBegin(GL_POLYGON);
         glTexCoord2f(0,1); glVertex2f(128-60,128-60);
         glTexCoord2f(1,1); glVertex2f(384+60,128-60);
         glTexCoord2f(1,0); glVertex2f(384+60,384+60);
         glTexCoord2f(0,0); glVertex2f(128-60,384+60);
      glEnd();
      #endif

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
   }

   glDisable(GL_TEXTURE_2D);
   check();
}

bool floored;
vec3f vel, ang_vel;

void computeRelativeTranslation(vec3f *out, vec3f *src, vec3f *angle)
{
  GLdouble matrix[16];
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glRotatef(-angle->y,  0.0f, 1.0f, 0.0f);
  glRotatef(-angle->x,  1.0f, 0.0f, 0.0f);
  glRotatef(-angle->z,  0.0f, 0.0f, 1.0f);
  glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
  glPopMatrix();

  out->x = matrix[0]*src->x + matrix[1]*src->y + matrix[2]*src->z;
  out->y = matrix[4]*src->x + matrix[5]*src->y + matrix[6]*src->z;
  out->z = matrix[8]*src->x + matrix[9]*src->y + matrix[10]*src->z;
}

#define HT 6
#define RAD  1.5
#define EYE_HT 5.25

bool fly;

#define GRAVITY 32

void playerTeleport(void)
{
}

static bool slew;
extern void physics_compute_move(vec3f *new_loc, vec3f *old_loc, float rad, float ht);
void camera_sim(float simtime, double total)
{
   vec3f rot_vel;
   vec3f c_ang;
   c_ang = camera_ang;
   computeRelativeTranslation(&rot_vel, &vel, &c_ang);
   vec3f_addeq_scale(&camera_ang, &ang_vel, simtime);
   vec3f_addeq_scale(&camera_loc, &rot_vel, simtime);
}

void teleportCamera(float x, float y)
{
   camera_loc.x = x;
   camera_loc.y = y;
}

bool simulation_active = TRUE;
bool editor_mode = TRUE;
bool freeze_lod = FALSE;

void simulate(float dt)
{
}

void doDraw(void)
{
}

bool outta_here = FALSE;
int demoRunLoopmode(float elapsedTime)
{
   static int hack_ani;
   static double totalTime;
   float simTime = elapsedTime;

   if (simTime > 0.125) simTime = 0.125;
   assert(simTime > 0);
   totalTime += simTime;
   camera_sim(simTime, totalTime);

   draw();
   glFinish();
   SwapBuffers(wglGetCurrentDC());

   return outta_here;
}

void gameMouse(int event, int x, int y, int shift, int control)
{
}

#define ANG_STEP 8
#define LOC_STEP 0.5

extern void turn();
extern bool quad_light;
extern void toggle_player_light(void);
extern bool ambient;
extern bool tex_white;
extern bool show_vis;
extern bool big_portal;
extern bool do_vis;

int demoProcessCharacter(int ch)
{
   switch(ch) {
      case 27: 
         return TRUE;

      case 'F': slew = !slew; break;

      case '\t': demoInit(); break;

      case '=': show_tex += 1; break;
      case '-': if (show_tex > 0) --show_tex; break;

      case ']': a_trans += 1; makeNoiseTextures(); break;
      case '[': a_trans -= 1; makeNoiseTextures(); break;
      case '}': b_trans += 1; makeNoiseTextures(); break;
      case '{': b_trans -= 1; makeNoiseTextures(); break;

      case 14: normalize = !normalize; break;
#ifdef SCREENSHOT
      case 26: {
         static int which=1;
         char filename[64];
         PSDbitmap *bm = malloc(sizeof(PSDbitmap) + sizeof(uint32) * scr_w * scr_h);
         extern void write_png(char *file_name, PSDbitmap *bm);
         bm->w = scr_w;
         bm->h = scr_h;
         glReadPixels(0,0,scr_w,scr_h, GL_RGBA, GL_UNSIGNED_BYTE, &bm->data[0]);
         sprintf(filename, "article/screen%03d.png", which++);
         write_png(filename, bm);
         free(bm);
         break;
      }
#endif

      case 'a': ang_vel.z += ANG_STEP; break;
      case 'd': ang_vel.z -= ANG_STEP; break;
      case 'r': ang_vel.x += ANG_STEP; break;
      case 'v': ang_vel.x -= ANG_STEP; break;
      case 'E': ang_vel.y += ANG_STEP; break;
      case 'Q': ang_vel.y -= ANG_STEP; break;

      case 'w': vel.y += LOC_STEP; break;
      case 'x': vel.y -= LOC_STEP; break;
      case 'z': vel.x -= LOC_STEP; break;
      case 'c': vel.x += LOC_STEP; break;
      case 'q': vel.z += LOC_STEP; break;
      case 'e': vel.z -= LOC_STEP; break;

      case '\r': camera_loc.x = camera_loc.y = 0; camera_loc.z = 8;
                camera_ang.x = camera_ang.y = camera_ang.z = 0;
                playerTeleport();
      case ' ': vel.x = vel.y = vel.z = 0;
                ang_vel.x = ang_vel.y = ang_vel.z = 0;
                playerTeleport();
                break;
      default:
         break;
   }
   return FALSE;
}


#define NORMALIZE_SIZE  32

// guarantee that we compute the exact same data along all edges
#define EXPAND(x)   ((float) (x) / (NORMALIZE_SIZE-1) * 2 -1)
#define CONTRACT(x) ((int) ((x) * 127) + 128)

static void buildNormalizingCubeMap(GLuint x)
{
   uint32 cube[NORMALIZE_SIZE][NORMALIZE_SIZE];
   vec3f n;
   int s,t;
   glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, x);

   // px
   for (t=0; t < NORMALIZE_SIZE; ++t) {
      for (s=0; s < NORMALIZE_SIZE; ++s) {
         n.x = 1;
         n.y = -EXPAND(t);
         n.z = -EXPAND(s);
         vec3f_normeq(&n);
         cube[t][s] = (CONTRACT(n.z)<<16) + (CONTRACT(n.y)<<8) + (CONTRACT(n.x));
      }
   }
   glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, 0, GL_RGB16, NORMALIZE_SIZE, NORMALIZE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, cube[0]);

   // -px
   for (t=0; t < NORMALIZE_SIZE; ++t) {
      for (s=0; s < NORMALIZE_SIZE; ++s) {
         n.x = -1;
         n.y = -EXPAND(t);
         n.z = EXPAND(s);
         vec3f_normeq(&n);
         cube[t][s] = (CONTRACT(n.z)<<16) + (CONTRACT(n.y)<<8) + (CONTRACT(n.x));
      }
   }
   glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, 0, GL_RGB16, NORMALIZE_SIZE, NORMALIZE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, cube[0]);

   // py
   for (t=0; t < NORMALIZE_SIZE; ++t) {
      for (s=0; s < NORMALIZE_SIZE; ++s) {
         n.x = EXPAND(s);
         n.y = 1;
         n.z = EXPAND(t);
         vec3f_normeq(&n);
         cube[t][s] = (CONTRACT(n.z)<<16) + (CONTRACT(n.y)<<8) + (CONTRACT(n.x));
      }
   }
   glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, 0, GL_RGB16, NORMALIZE_SIZE, NORMALIZE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, cube[0]);

   // -py
   for (t=0; t < NORMALIZE_SIZE; ++t) {
      for (s=0; s < NORMALIZE_SIZE; ++s) {
         n.x = EXPAND(s);
         n.y = -1;
         n.z = -EXPAND(t);
         vec3f_normeq(&n);
         cube[t][s] = (CONTRACT(n.z)<<16) + (CONTRACT(n.y)<<8) + (CONTRACT(n.x));
      }
   }
   glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, 0, GL_RGB16, NORMALIZE_SIZE, NORMALIZE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, cube[0]);

   // pz
   for (t=0; t < NORMALIZE_SIZE; ++t) {
      for (s=0; s < NORMALIZE_SIZE; ++s) {
         n.x = EXPAND(s);
         n.y = -EXPAND(t);
         n.z = 1;
         vec3f_normeq(&n);
         cube[t][s] = (CONTRACT(n.z)<<16) + (CONTRACT(n.y)<<8) + (CONTRACT(n.x));
      }
   }
   glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, 0, GL_RGB16, NORMALIZE_SIZE, NORMALIZE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, cube[0]);

   // px
   for (t=0; t < NORMALIZE_SIZE; ++t) {
      for (s=0; s < NORMALIZE_SIZE; ++s) {
         n.x = -EXPAND(s);
         n.y = -EXPAND(t);
         n.z = -1;
         vec3f_normeq(&n);
         cube[t][s] = (CONTRACT(n.z)<<16) + (CONTRACT(n.y)<<8) + (CONTRACT(n.x));
      }
   }
   glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, 0, GL_RGB16, NORMALIZE_SIZE, NORMALIZE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, cube[0]);

   glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
