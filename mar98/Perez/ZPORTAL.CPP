/*
portal rendering run one.

ZPortal.cpp
*/

#include "Zportal.h"
#include "Ztypes.h"
#include "newengg.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "Zfile.h"
#include "glide.h"
#include "zcamera.h"
#include "zrender.h"
#define PI 3.14159
extern float sinlook[32767],coslook[32767];

//************************************
// Constructor
//------------------------------------
// Nothing to do!
//************************************

ZPortal::ZPortal() {
	oh.v[0]=10.0;
	oh.v[1]=15.0;
	oh.v[2]=10.0;
	oaxis[0].v[0]=0.0;
	oaxis[0].v[1]=0.0;
	oaxis[0].v[2]=1.0;
	oaxis[1].v[0]=-1.0;
	oaxis[1].v[1]=0.0;
	oaxis[1].v[2]=0.0;
	oaxis[2].v[0]=0.0;
	oaxis[2].v[1]=-1.0;
	oaxis[2].v[2]=0.0;

}

//************************************
// Destructor
//------------------------------------
// Close the file if needed
//************************************

ZPortal::~ZPortal() {
}

void ZPortal::GenPlane(ZPpoly_t *node)
{
	vert3d_t u,v,n;
	float mag;
    u.v[0]=node->poly.v[0].v[0]-node->poly.v[1].v[0];
    u.v[1]=node->poly.v[0].v[1]-node->poly.v[1].v[1];
    u.v[2]=node->poly.v[0].v[2]-node->poly.v[1].v[2];
    v.v[0]=node->poly.v[2].v[0]-node->poly.v[1].v[0];
    v.v[1]=node->poly.v[2].v[1]-node->poly.v[1].v[1];
    v.v[2]=node->poly.v[2].v[2]-node->poly.v[1].v[2];
    CrossProduct3D(&u,&v,&n);
	mag=(float)sqrt(DotProduct3D(&n,&n));
    node->plane.norm.v[0]=(float)(n.v[0]/mag);
    node->plane.norm.v[1]=(float)(n.v[1]/mag);
    node->plane.norm.v[2]=(float)(n.v[2]/mag);
	node->plane.d=-DotProduct3D(&node->plane.norm,&node->poly.v[0]);
}

void ZPortal::LoadB2(char *filename)
{
	//LPGXNode node;
	int i,i2,i3,nverts,npolys,nportals;
	vert3d_t *tempbuff,color;
	GU32 temp,tmp;
	LPZFile _file;
	_file = new ZFile;
	_file->Open(filename);
	_file->Read(&temp, sizeof(int), &tmp);
	nverts=(GU32)temp;
	_file->Read(&temp, sizeof(int), &tmp);
	npolys=(GU32)temp;
	_file->Read(&temp, sizeof(int), &tmp);
	ncells=(GU32)temp;
	tempbuff=(vert3d_t *)malloc(nverts*sizeof(vert3d_t));
	//read verts
	for(i=0;i<nverts;i++) {
		//the vertex numbers are screwy because the data was recorded
		//with the up-down coord being Z and the in-out coord being x, so
		//it's changed to this to make viewing, etc, less of a headache.
		_file->Read(&tempbuff[i].v[2], sizeof(float), &tmp);
		_file->Read(&tempbuff[i].v[0], sizeof(float), &tmp);
		_file->Read(&tempbuff[i].v[1], sizeof(float), &tmp);
		tempbuff[i].v[0]*=10.0;
		tempbuff[i].v[1]*=10.0;
		tempbuff[i].v[2]*=10.0;
	}
	//read faces
	for(i=0;i<ncells;i++)
	{
		_file->Read(&temp, sizeof(int), &tmp);
		npolys=(GU32)temp;
		_file->Read(&temp, sizeof(int), &tmp);
		nportals=(GU32)temp;
		c[i].npolys=npolys+nportals;
		//first read the normal polys
		for(i2=0;i2<npolys;i2++) {
			_file->Read(&temp, sizeof(int), &tmp);
			color.v[0]=256.f*(float)rand()/32767.f;
			color.v[1]=256.f*(float)rand()/32767.f;
			color.v[2]=256.f*(float)rand()/32767.f;
			c[i].p[i2].poly.nverts = (int)temp;
			c[i].p[i2].type=-1;//not a portal
			for(i3=0;i3<c[i].p[i2].poly.nverts;i3++)
			{
				_file->Read(&temp, sizeof(int), &tmp);
				c[i].p[i2].poly.v[i3] = tempbuff[(int)temp];
				c[i].p[i2].poly.c[i3] = color;
			}
			GenPlane(&c[i].p[i2]);
			c[i].p[i2].poly.color=1+rand()%15;
		}
		//now the portals
		for(i2=0;i2<nportals;i2++) {
			_file->Read(&temp, sizeof(int), &tmp);
			c[i].p[i2+npolys].type= (int)temp;
			_file->Read(&temp, sizeof(int), &tmp);
			c[i].p[i2+npolys].poly.nverts = (int)temp;
			for(i3=0;i3<c[i].p[i2+npolys].poly.nverts;i3++)
			{
				_file->Read(&temp, sizeof(int), &tmp);
				c[i].p[i2+npolys].poly.v[i3] = tempbuff[(int)temp];
				c[i].p[i2+npolys].poly.c[i3] = color;
			}
			GenPlane(&c[i].p[i2+npolys]);
		}
	}
	delete(tempbuff);
	//now just fix the last one.
	_file->Close();
}

void ZPortal::BuildFrus(poly_t *poly,ZPfrus_t *frus)
{
	//ARGH, THIS IS A SLOW HACK AND SHOULD BE FIXED IMMIDIATELY
	//NOTE:we're assuming no near/far plane.
	vert3d_t a,b;
	int i,j;
	frus->nfrus=0;//just to make sure.
	//well... each plane... uhhh... well, the normals point out.
	i=0;
	j=1;
	while(i<poly->nverts){
		//stuff
		a.v[0]=poly->v[i].v[0]-_cam->loc.v[0];
		a.v[1]=poly->v[i].v[1]-_cam->loc.v[1];
		a.v[2]=poly->v[i].v[2]-_cam->loc.v[2];
		b.v[0]=poly->v[j].v[0]-_cam->loc.v[0];
		b.v[1]=poly->v[j].v[1]-_cam->loc.v[1];
		b.v[2]=poly->v[j].v[2]-_cam->loc.v[2];
		CrossProduct3D(&a,&b,&frus->p[frus->nfrus].norm);
		Normalize(&frus->p[frus->nfrus].norm);
		//find the d
		frus->p[frus->nfrus].d=-DotProduct3D(&frus->p[frus->nfrus].norm,&_cam->loc);
		i++;
		j++;
		if(j==poly->nverts)j=0;
		frus->nfrus++;
	}
}

void ZPortal::DrawCell(ZPcell_t *cell,ZPfrus_t *frus)
{
	int i,frusnum,flag,output;
	vert3d_t temp;
	poly_t *poly;
	ZPfrus_t frus2;
	//rotate it, move it, project it, and draw it.
	poly_t spoly1,spoly2,*pa,*pb,*ptemp;
	frus2.nfrus=0;
	for(i=0;i<cell->npolys;i++){
		//first do a dp test.
		if(TestVert(&cell->p[i].plane,&_cam->loc)==FRONT){
			poly=&cell->p[i].poly;
			spoly1.nverts=poly->nverts;
			spoly1=*poly;
			spoly2=*poly;
			temp.v[0]=-_cam->loc.v[0];
			temp.v[1]=-_cam->loc.v[1];
			temp.v[2]=-_cam->loc.v[2];
			//now we clip.
			pa=&spoly1;
			pb=&spoly2;
			output=VALID;
			for(frusnum=0;frusnum<frus->nfrus;frusnum++)
			{
				if(output==VALID){
					output=ClipPoly(pa, &frus->p[frusnum], pb);
					//put the output poly, good or bad, into pa.
					ptemp=pa;
					pa=pb;
					pb=ptemp;
				}
			}
			if(output==VALID){
				if(cell->p[i].type==-1)//this is a regular polygon
					DrawPoly(pa);
				else{
					BuildFrus(pa,&frus2);
					DrawCell(&c[cell->p[i].type],&frus2);
				}
			}
		}
	}
}


void ZPortal::DrawPoly(poly_t *poly){
	vert3d_t temp;
	polygon2D_t poly2d;
	int i;
	if(poly->nverts>2 && !flag){
		temp.v[0]=-_cam->loc.v[0];
		temp.v[1]=-_cam->loc.v[1];
		temp.v[2]=-_cam->loc.v[2];
		TranslateVerts(&poly->v[0], &temp,poly->nverts);
		RotateVerts(&poly->v[0], &poly->v[0],&_cam->axis[0],poly->nverts);
		ProjectPolygon (poly, &poly2d);
		FillPolygon2D(&poly2d);
/*		ProjectVerts(&poly->v[0], poly->nverts);
		//we should be ready to draw now.
		i=poly->nverts-1;
		while(i>1){
			_rend->DrawTriangle(&poly->v[0],&poly->c[0],&poly->v[i-1],&poly->c[i-1],&poly->v[i],&poly->c[i]);
			i--;
		}*/
	}
	//this is the hack for drawing overhead.
	if(poly->nverts>2 && flag){
		temp.v[0]=-oh.v[0];
		temp.v[1]=-oh.v[1];
		temp.v[2]=-oh.v[2];
		TranslateVerts(&poly->v[0], &temp,poly->nverts);
		RotateVerts(&poly->v[0], &poly->v[0],&oaxis[0],poly->nverts);
		ProjectPolygon (poly, &poly2d);
		FillPolygon2D(&poly2d);

		//this is the old render code.
		/*		ProjectVerts(&poly->v[0], poly->nverts);
		//we should be ready to draw now.
		i=poly->nverts-1;
		while(i>1){
			_rend->DrawTriangle(&poly->v[0],&poly->c[0],&poly->v[i-1],&poly->c[i-1],&poly->v[i],&poly->c[i]);
			i--;
		}*/
	}
}

void ZPortal::FindCameraCell()
{
	int i,i2,newcell=-1,inside;
	float dp,dp2;
	for(i=0;i<ncells;i++)
	{
		inside=1;
		for(i2=0;i2<c[i].npolys;i2++)
		{
			if(TestVert(&c[i].p[i2].plane,&_cam->loc)==BACK)
				inside=0;
		}
		if(inside)//we're inside this cell.
			newcell=i;
	}
	//here we can do collision detection
	if(newcell>=0)
		currcell=newcell;
	else{//do collision detection
		//basically, for every offensive plane, we just move the camera along
		//the plane normal till it's inside the cell.
		for(i=0;i<c[currcell].npolys;i++)
		{
			dp2=-DotProduct3D(&c[currcell].p[i].plane.norm,&_cam->loc);
			if((dp2-c[currcell].p[i].plane.d)>-0.1){
			//if(TestVert(&c[currcell].p[i].plane,&_cam->loc)==BACK){
				//find how far behind this polygon it is.
				dp=-DotProduct3D(&c[currcell].p[i].plane.norm,&_cam->loc);
				dp=(dp-c[currcell].p[i].plane.d);//we move it 0.1 away from the wall.
				//if(dp<0)dp*=-1.0;
				dp+=0.1;
				_cam->loc.v[0]+=dp*c[currcell].p[i].plane.norm.v[0];
				_cam->loc.v[1]+=dp*c[currcell].p[i].plane.norm.v[1];
				_cam->loc.v[2]+=dp*c[currcell].p[i].plane.norm.v[2];
			}
		}
	}
}



