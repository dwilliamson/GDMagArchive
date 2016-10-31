/*
camera code

zcamera.cpp
*/

//#include "Zfile.h"
#include "Zcamera.h"
#include "Ztypes.h"
#include "newengg.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "glide.h"
#define PI 3.14159
extern float sinlook[32767],coslook[32767];

//semi-unnecesary camera code

/*/////////////////////////////////////////////////////////////////////
constructor

blah, blah
/////////////////////////////////////////////////////////////////////*/

ZCamera::ZCamera(float a, float b) {
	BuildLookupTables();
	fov=a;
	scale=b/a;
}

/*/////////////////////////////////////////////////////////////////////
destructor

blah, blah
/////////////////////////////////////////////////////////////////////*/

ZCamera::~ZCamera() {
}


//sigh, the problems with not having one 50,000 line c++ file...
extern char *pDIB;		// pointers to DIB section we'll draw into
extern int DIBWidth, DIBHeight;
extern int DIBPitch;
extern double  fieldofview, xcenter, ycenter;
extern double  xscreenscale, yscreenscale, maxscale;

/*/////////////////////////////////////////////////////////////////////
genfrus

generate the frustum, uses ugly trig.
/////////////////////////////////////////////////////////////////////*/

void ZCamera::GenFrus(){
	float theta;
	//we're pushing the planes in EVER so slightly so that
	//there's no div by 0
	//okay, here's the messy trig.
	theta=(float)atan((xcenter)/(maxscale));
	frus[0].norm.v[0]=-1.f*(float)(cos(theta));
	frus[0].norm.v[1]=0.0;
	frus[0].norm.v[2]=(float)sin(theta);
	frus[0].d=-EPSILON;//right plane
	theta=(float)atan((ycenter-0.5)/(maxscale));
	frus[1].norm.v[0]=0.0;
	frus[1].norm.v[1]=-1.f*(float)(cos(theta));
	frus[1].norm.v[2]=(float)sin(theta);
	frus[1].d=-EPSILON;//top plane
	theta=(float)atan((xcenter-0.5)/(maxscale));
	frus[2].norm.v[0]=(float)(cos(theta));
	frus[2].norm.v[1]=0.0;
	frus[2].norm.v[2]=(float)sin(theta);
	frus[2].d=-EPSILON;//left plane
	theta=(float)atan((ycenter-0.5)/(maxscale));
	frus[3].norm.v[0]=0.0;
	frus[3].norm.v[1]=(float)(cos(theta));
	frus[3].norm.v[2]=(float)sin(theta);
	frus[3].d=-EPSILON;//bottom plane
}

/*/////////////////////////////////////////////////////////////////////
testvert3d

good stuff for clipping.
/////////////////////////////////////////////////////////////////////*/
int ZCamera::TestVert3D(int frusnum,vert3d_t *vert)
{
  float dist;
  dist=-DotProduct3D(vert,&frus[frusnum].norm);
  //printf("dist is %f, plane is %f\n",dist,plane->r);
  if((frus[frusnum].d - dist) > EPSILON)
    return(FRONT);
  if((frus[frusnum].d - dist) < -EPSILON)
    return(BACK);
  return(COPLANAR);
}


/*/////////////////////////////////////////////////////////////////////
rotateaxis

standard rotation fare
/////////////////////////////////////////////////////////////////////*/

void ZCamera::RotateAxis()
{
    //this finds the new unit vectors
   
    int index;
    vert3d_t *pvert;
    float sinangx,cosangx,sinangy,cosangy,sinangz,cosangz;
    float xold,yold,zold,xnew,ynew,znew;//temporary rotation results to prevent round off error
    sinangx=sinlook[xang];
    cosangx=coslook[xang];
    sinangy=sinlook[yang];
    cosangy=coslook[yang];
    sinangz=sinlook[zang];
    cosangz=coslook[zang];
    pvert=&axis[0];
    for(index=0; index<3; index++)
    {
////////init points/////////////////////
        xold=pvert->v[0];
        yold=pvert->v[1];
        zold=pvert->v[2];
////////rotate point locally////////////        
        //xrot
        xnew=xold;
        ynew=(yold*cosangx)-(zold*sinangx);
        znew=(yold*sinangx)+(zold*cosangx);
        xold=xnew;
        yold=ynew;
        zold=znew;
        //yrot            
        xnew=((xold*cosangy)+(zold*sinangy));
        ynew=yold;
        znew=((-1*(xold*sinangy))+(zold*cosangy));
        xold=xnew;
        yold=ynew;
        zold=znew;
        //zrot
        xnew=((xold*cosangz)-(yold*sinangz));
        ynew=((yold*cosangz)+(xold*sinangz));
        znew=zold;
        //change object location
        pvert->v[0]=xnew;
        pvert->v[1]=ynew;
        pvert->v[2]=znew;
        //printf("%f,%f,%f\n",pvert->v[0],pvert->v[1],pvert->v[2]);
        pvert++;
    }
    //getchar();
}

/*/////////////////////////////////////////////////////////////////////
gennewfrus

generate a frustum for the new camera axis, tricky, but if you understand
backrotatevector then it's pretty straightforward
/////////////////////////////////////////////////////////////////////*/

void ZCamera::GenNewFrus()
{
	int i;
	//hmmm... okay, we're just backrotating.
	for(i=0;i<4;i++)
		BackRotateVector(&frus[i].norm, &frusw[i].norm, &axis[0]);
	//now gen the d's
	for(i=0;i<4;i++)
		frusw[i].d=-DotProduct3D(&frusw[i].norm,&loc);
}
