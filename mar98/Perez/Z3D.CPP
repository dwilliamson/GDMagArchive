/*
3d stuff

Z3d.cpp
*/
#include "z3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#define PI 3.14159

//typical brainless 3D code that we all know and love.
//note!  the abrash poly filler is in here!

float sinlook[32767],coslook[32767];


inline float DotProduct3D(vert3d_t *a, vert3d_t *b)
{
	return(a->v[0]*b->v[0]+a->v[1]*b->v[1]+a->v[2]*b->v[2]);
}

void CrossProduct3D(vert3d_t *u,vert3d_t *v, vert3d_t *normal)
{
  normal->v[0]=  (u->v[1]*v->v[2]-u->v[2]*v->v[1]);
  normal->v[1]= -(u->v[0]*v->v[2]-u->v[2]*v->v[0]);
  normal->v[2]=  (u->v[0]*v->v[1]-u->v[1]*v->v[0]);
}

void Normalize(vert3d_t *v)
{
	float mag;
	mag=(float)sqrt(DotProduct3D(v,v));
	v->v[0]/=mag;
	v->v[1]/=mag;
	v->v[2]/=mag;
}

void BackRotateVector(vert3d_t *in, vert3d_t *out, vert3d_t *axis)
{
  out->v[0]=in->v[0] *axis[0].v[0] + in->v[1] * axis[1].v[0] + in->v[2] * axis[2].v[0];
  out->v[1]=in->v[0] *axis[0].v[1] + in->v[1] * axis[1].v[1] + in->v[2] * axis[2].v[1];
  out->v[2]=in->v[0] *axis[0].v[2] + in->v[1] * axis[1].v[2] + in->v[2] * axis[2].v[2];
}

void AssignVert(vert3d_t *vert, float x, float y, float z)
{
	vert->v[0]=x;
	vert->v[1]=y;
	vert->v[2]=z;
}

void BuildLookupTables(void)
{
    int angle;
    float rad;
    for(angle=0;angle<32768;angle++)
    {
        rad=(float)(PI*(float)angle/(float)16384);
        coslook[angle]=(float)cos(rad);
        sinlook[angle]=(float)sin(rad);
    }
}

inline int TestVert(plane_t *plane,vert3d_t *vert)
{
  float val;
  val=plane->d + DotProduct3D(vert,&plane->norm);
  if(val > EPSILON)
    return(FRONT);
  if(val < -EPSILON)
    return(BACK);
  return(COPLANAR);
}


inline void RotateVerts(vert3d_t *in, vert3d_t *out, vert3d_t *axis, int nverts)
{
	vert3d_t temp;
	for(int i=0;i<nverts;i++)
	{
        temp.v[0]=DotProduct3D(&axis[0],in);
        temp.v[1]=DotProduct3D(&axis[1],in);
        temp.v[2]=DotProduct3D(&axis[2],in);
		out->v[0]=temp.v[0];
		out->v[1]=temp.v[1];
		out->v[2]=temp.v[2];
		in++;
		out++;
	}

}
inline void TranslateVerts(vert3d_t *vert, vert3d_t *disp,int nverts)
{
	for(int i=0;i<nverts;i++){
		vert->v[0]+=disp->v[0];
		vert->v[1]+=disp->v[1];
		vert->v[2]+=disp->v[2];
		vert++;
	}
}
inline void RotateUnitVectors(vert3d_t *axis, int *vals)
{
    int index;
    vert3d_t *pvert;
    float sinangx,cosangx,sinangy,cosangy,sinangz,cosangz;
    float xold,yold,zold,xnew,ynew,znew;//temporary rotation results to prevent round off error
    sinangx=sinlook[vals[0]];
    cosangx=coslook[vals[0]];
    sinangy=sinlook[vals[1]];
    cosangy=coslook[vals[1]];
    sinangz=sinlook[vals[2]];
    cosangz=coslook[vals[2]];
    pvert=axis;
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

int ClipPoly(poly_t *in, plane_t *plane, poly_t *out)
{
	int thisvert,nextvert,thisval,nextval,mytemp;
	thisvert=0;
	nextvert=1;
	out->nverts=0;
	while(thisvert<in->nverts){
		mytemp=TestVert(plane,&in->v[thisvert]);
		thisval=1;
		if(mytemp==BACK)
			thisval=0;
		mytemp=TestVert(plane,&in->v[nextvert]);
		nextval=1;
		if(mytemp==BACK)
			nextval=0;
		//first, add the vert if it's on the right side.
		if(thisval){
			out->v[out->nverts]=in->v[thisvert];
			out->c[out->nverts]=in->c[thisvert];
			out->f[out->nverts]=in->f[thisvert];
			out->nverts++;
		}
		//then just check for a split vert.
		if((thisval&&!nextval)||(!thisval&&nextval)){
			//add a split vertex.
		    vert3d_t temp,temp2;
			float dist1,dist2,t;
			dist1=-DotProduct3D(&in->v[thisvert],&plane->norm);
			dist2=-DotProduct3D(&in->v[nextvert],&plane->norm);
			temp.v[0]=in->v[nextvert].v[0]-in->v[thisvert].v[0];
			temp.v[1]=in->v[nextvert].v[1]-in->v[thisvert].v[1];
			temp.v[2]=in->v[nextvert].v[2]-in->v[thisvert].v[2];
			temp2.v[0]=in->c[nextvert].v[0]-in->c[thisvert].v[0];
			temp2.v[1]=in->c[nextvert].v[1]-in->c[thisvert].v[1];
			temp2.v[2]=in->c[nextvert].v[2]-in->c[thisvert].v[2];
			t=(plane->d - dist1)/(dist2-dist1);
			out->v[out->nverts].v[0]=in->v[thisvert].v[0]+t*temp.v[0];
			out->v[out->nverts].v[1]=in->v[thisvert].v[1]+t*temp.v[1];
			out->v[out->nverts].v[2]=in->v[thisvert].v[2]+t*temp.v[2];
			out->c[out->nverts].v[0]=in->c[thisvert].v[0]+t*temp2.v[0];
			out->c[out->nverts].v[1]=in->c[thisvert].v[1]+t*temp2.v[1];
			out->c[out->nverts].v[2]=in->c[thisvert].v[2]+t*temp2.v[2];
			out->f[out->nverts]=1;//make it visible.
			out->nverts++;
		}
		thisvert++;
		nextvert++;
		if(nextvert==in->nverts)
			nextvert=0;
	}
	if(out->nverts>2)
		return(VALID);//good poly
	return(INVALID);//bad poly
}

extern char *pDIB;		// pointers to DIB section we'll draw into
extern int DIBWidth, DIBHeight;
extern int DIBPitch;
extern double  fieldofview, xcenter, ycenter;
extern double  xscreenscale, yscreenscale, maxscale;

void FillPolygon2D(polygon2D_t *ppoly)
{
    int     i, j, topvert, bottomvert, leftvert, rightvert, nextvert;
    int     itopy, ibottomy, islope, spantopy, spanbottomy, x, count;
    double  topy, bottomy, slope, height, width, prestep;
    span_t  spans[2048], *pspan;

    topy = 999999.0;
    bottomy = -999999.0;
    for (i=0 ; i<ppoly->numverts ; i++)
    {
        if (ppoly->verts[i].y < topy)
        {
            topy = ppoly->verts[i].y;
            topvert = i;
        }
        if (ppoly->verts[i].y > bottomy)
        {
            bottomy = ppoly->verts[i].y;
            bottomvert = i;
        }
    }

    itopy = (int)ceil(topy);
    ibottomy = (int)ceil(bottomy);

    if (ibottomy == itopy)
        return;     // reject polygons that don't cross a scan

    // Scan out the left edge
    pspan = spans;
    leftvert = topvert;

    do
    {
        nextvert = leftvert - 1;
        if (nextvert < 0)
            nextvert = ppoly->numverts - 1;
        spantopy = (int)ceil(ppoly->verts[leftvert].y);
        spanbottomy = (int)ceil(ppoly->verts[nextvert].y);
        if (spantopy < spanbottomy)
        {
            height = ppoly->verts[nextvert].y -
                    ppoly->verts[leftvert].y;
            width = ppoly->verts[nextvert].x -
                    ppoly->verts[leftvert].x;
            slope = width / height;
            prestep = spantopy - ppoly->verts[leftvert].y;
            x = (int)((ppoly->verts[leftvert].x +
                    (slope * prestep)) * 65536.0) + ((1 << 16) - 1);
            islope = (int)(slope * 65536.0);

            for (j=spantopy ; j<spanbottomy ; j++)
            {
                pspan->xleft = x >> 16;
                x += islope;
                pspan++;
            }
        }

        leftvert--;
        if (leftvert < 0)
            leftvert = ppoly->numverts - 1;
    } while (leftvert != bottomvert);

    // Scan out the right edge
    pspan = spans;
    rightvert = topvert;

    do
    {
        nextvert = (rightvert + 1) % ppoly->numverts;
        spantopy = (int)ceil(ppoly->verts[rightvert].y);
        spanbottomy = (int)ceil(ppoly->verts[nextvert].y);
        if (spantopy < spanbottomy)
        {
            height = ppoly->verts[nextvert].y -
                    ppoly->verts[rightvert].y;
            width = ppoly->verts[nextvert].x -
                    ppoly->verts[rightvert].x;
            slope = width / height;
            prestep = spantopy - ppoly->verts[rightvert].y;
            x = (int)((ppoly->verts[rightvert].x +
                    (slope * prestep)) * 65536.0) + ((1 << 16) - 1);
            islope = (int)(slope * 65536.0);

            for (j=spantopy ; j<spanbottomy ; j++)
            {
                pspan->xright = x >> 16;
                x += islope;
                pspan++;
            }
        }

        rightvert = (rightvert + 1) % ppoly->numverts;
    } while (rightvert != bottomvert);

    // Draw the spans
    pspan = spans;

    for (i=itopy ; i<ibottomy ; i++)
    {
        count = pspan->xright - pspan->xleft;
        if (count > 0)
        {
            memset (pDIB + (DIBPitch * i) + pspan->xleft,
                    ppoly->color,
                    count);
        }
        pspan++;
    }
}

void ProjectPolygon (poly_t *ppoly, polygon2D_t *ppoly2D)
{
    int     i;
    double  zrecip;

    for (i=0 ; i<ppoly->nverts ; i++)
    {
        zrecip = 1.0 / ppoly->v[i].v[2];
        ppoly2D->verts[ppoly->nverts-1-i].x =
               ppoly->v[i].v[0] * zrecip * maxscale + xcenter;
        ppoly2D->verts[ppoly->nverts-1-i].y = DIBHeight -
             (ppoly->v[i].v[1] * zrecip * maxscale + ycenter);
    }

    ppoly2D->color = ppoly->color;
    ppoly2D->numverts = ppoly->nverts;
}

