/*
header to camera code

zcamera.h
*/
#pragma once

#include "Ztypes.h"
#include "newengg.h"
#include "Z3d.h"
typedef class ZCamera {

public:

	// Constructors / Destructors

	ZCamera(float a,float b);
	~ZCamera();
	// Creation functions
	vert3d_t loc, axis[3];
	plane_t frus[4],frusw[4];
	int xang,yang,zang;
	float fov, scale;
	void Place(float x,float y,float z){
		loc.v[0]=x;
		loc.v[1]=y;
		loc.v[2]=z;
	}
	void Nudge(float x,float y,float z){
		loc.v[0]+=x;
		loc.v[1]+=y;
		loc.v[2]+=z;
	}
	void InitRot(int xa, int ya, int za){
		axis[0].v[1]=axis[0].v[2]=0.0;
		axis[1].v[0]=axis[1].v[2]=0.0;
		axis[2].v[0]=axis[2].v[1]=0.0;
		axis[0].v[0]=axis[1].v[1]=axis[2].v[2]=1.0;
		xang=xa;
		yang=ya;
		zang=za;
	}
	void GenFrus();
	int TestVert3D(int frusnum,vert3d_t *vert);
	void TestVFrus(vert3d_t *vert,unsigned char *out);
	void GenNewFrus();//finds the new frustum (duh)
	void ZCamera::RotateAxis();
} ZCamera, *LPZCamera;