// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : meshes.cpp
// Description : Basic functions tu use simple meshes
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------
#include <stdio.h>

#include "meshes.hpp"
#include "tools.hpp"




Mesh::Mesh()
{
	nVertices		= 0;
	nTriangles		= 0;
	VertexArray		= NULL;
	TriangleArray	= NULL;
	FlagArray		= NULL;
}



Mesh::~Mesh()
{
	if (VertexArray)			delete [] VertexArray;
	if (TriangleArray)			delete [] TriangleArray;
	if (TriangleNormalArray)	delete [] TriangleNormalArray;
	if (FlagArray)				delete [] FlagArray;
}


void Mesh::Create(int nv, int nt, float *va, int *ta)
{
	nVertices = nv;
	nTriangles = nt;
	
	VertexArray			= new Vertex [nVertices];
	TriangleArray		= new int [nTriangles*3];
	TriangleNormalArray = new Hvector [nTriangles];
	FlagArray			= new char [nTriangles];

	for (int i=0; i<nv; i++)
		VertexArray[i] = Vertex(va[i*3], va[i*3+1], va[i*3+2]);

	for (int j=0; j<nt*3; j++)
		TriangleArray[j] = ta[j];

	for (int n=0; n<nt; n++)
	{
		Hvector v1, v2, v3;
		v1.x = VertexArray[ TriangleArray[n*3]+0 ].x;
		v1.y = VertexArray[ TriangleArray[n*3]+0 ].y;
		v1.z = VertexArray[ TriangleArray[n*3]+0 ].z;

		v2.x = VertexArray[ TriangleArray[n*3+1] ].x;
		v2.y = VertexArray[ TriangleArray[n*3+1] ].y;
		v2.z = VertexArray[ TriangleArray[n*3+1] ].z;

		v3.x = VertexArray[ TriangleArray[n*3+2] ].x;
		v3.y = VertexArray[ TriangleArray[n*3+2] ].y;
		v3.z = VertexArray[ TriangleArray[n*3+2] ].z;

		TriangleNormalArray[n] = NormalVector(v1, v2, v3);
	}
}




void Mesh::Rotate(Camera &camera)
{
	Hvector Position;
	float	ooz;
	float width = camera.Width;
	float height = camera.Height;
	float transx = width / 2;
	float transy = height / 2;

	// cas special pour la camera
	Hmatrix4x4 invcam = camera.Orient;
	invcam.Inverse();
	Global = Hproduct(invcam, Local); // global = rotation finale
	Hvector PosMesh = Local.GetPosition();
	Hvector PosCam = camera.Orient.GetPosition();
	Hvector PosMeshInCam = PosMesh-PosCam;
	PosMeshInCam = Product(invcam, PosMeshInCam);
	Global.SetPosition(PosMeshInCam);
	// cas special pour la camera


	Hmatrix4x4 &m = Global;
	m.GetPosition(Position);
	for (int i=0; i<nVertices; i++)
	{
		Vertex &v = VertexArray[i];

		v.flags = 0; // RAZ flags
		v.pz = (v.x*m.xz + v.y*m.yz + v.z*m.zz) + Position.z;
		if (v.pz<16.0)
		{
			v.flags |= 16;
			continue;						// Z-clipping is not handled here.
		}
		v.px = (v.x*m.xx + v.y*m.yx + v.z*m.zx) + Position.x;
		v.py = (v.x*m.xy + v.y*m.yy + v.z*m.zy) + Position.y;

		ooz = camera.Focus / v.pz;
		v.px = (v.px * ooz) + transx;
		v.py = (v.py * ooz) + transy;
		v.pz = 1.0f / v.pz;
		
		if (v.px <0)		v.flags |= 1;	// left		0001 - 1
		if (v.px >width-1)	v.flags |= 2;	// right	0010 - 2
		if (v.py <0)		v.flags |= 4;	// top		0100 - 4
		if (v.py >height-1)	v.flags |= 8;	// bottom	1000 - 8
	}
}



void Mesh::SetPosition(float px, float py, float pz)
{
	Local.SetPosition( Hvector(px, py, pz) );
}



void Mesh::SetPosition(Hvector &v)
{
	Local.SetPosition( v );
}



void Mesh::Euler(long a, long b, long c)
{
	Local.Euler(a, b, c);
}



Hvector Mesh::GetPosition()
{
	return Local.GetPosition();
}


void Mesh::Move(int Movement, float Value)
{
	switch (Movement)
	{
	case XPLUS:
		Local.tx += Value;
		break;
	case YPLUS:
		Local.ty -= Value;
		break;
	case ZPLUS:
		Local.tz += Value;
		break;
	case XLESS:
		Local.tx -= Value;
		break;
	case YLESS:
		Local.ty += Value;
		break;
	case ZLESS:
		Local.tz -= Value;
		break;
	}
}