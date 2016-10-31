///////////////////////////////////////////////////////////////////////////////
//
// VShader.h : header file
//
// Purpose:	Implementation of Vertex Shaders in Direct3D
//
// Created:
//		JL 12/15/00
//
// Notes:	Modified from the Original Nividia (Microsoft?) Vertex Shader Demo
//			Original code doesn't appear to be available anymore
//			Portions Copyright (C) 1999, 2000 NVIDIA Corporation
//			
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2001 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#ifndef VShader_H
#define VShader_H

#include "Windows.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

#include "D3D8.h"
#include "d3dx8.h"

// Structure for my custom vertex type
struct MyVertex
{
	float x, y, z;			// v0.xyz Vertex Coordinate
	float nx, ny, nz;		// v1.xyz Normals
	float u0;				// v2.x	texcoord for stage 0
};

typedef MyVertex MyVector;

struct MyFace {
	int v1, v2, v3;
	float nx, ny, nz;
};

typedef struct
{
	long	v[3],n[3],t[3];
	int		mat;
} t_triIndex;


// 3D Object rendering structure
typedef struct s_Visual
{
	D3DXVECTOR3	*vertex;		// Vertex
	long		vertexCnt;		// number of vertices
	D3DXVECTOR3	*normal;		// Normal
	long		normalCnt;		// number of normals
	D3DXVECTOR3	*uv;			// Texture Coordinates
	long		uvCnt;			// number of texture coordinates
	t_triIndex	*index;			// face pointer
	long		triCnt;			// number of triangles
	D3DXVECTOR3 Ka,Kd,Ks;		// global color for object
	float		Ns;				// specular coefficient
	char		map[255];		// Texture map name
	unsigned int	texID;		// texture id number
	int			texWidth, texHeight, texDepth;	// texture size
	D3DXVECTOR3	bbox[2];		// BBOX upper left and lower right
} t_Visual;


// Camera used for visualization
typedef struct s_Camera
{
	D3DXVECTOR3		rot;					// CURRENT ROTATION FACTORS
	D3DXVECTOR3		pos;					// CURRENT TRANSLATION FACTORS
	float			fov;					// FIELD OF VIEW
	D3DXMATRIX		matrix;
} t_Camera;


struct Extents
{
	float maxX, minX;
	float maxY, minY;
	float maxZ, minZ;
};

typedef float Vec3[3];

class CVShader
{
public:
	t_Camera	m_Camera;
	DWORD       m_dwBackBufferWidth;
	DWORD       m_dwBackBufferHeight;
	D3DFILLMODE m_fillMode;
	D3DXVECTOR4	m_vLightDir;	// Used for lighting
	bool        m_bWindowed;
	bool        m_bRefRast;
	t_Visual	m_Visual;		// Actual model to render
	char*       name;

	HRESULT Init(HWND hwnd, const char* fileName);
	HRESULT DXRun();
	void Release();
	void UpdateCamera();

	// constructor
	CVShader();

	// destructor
	~CVShader()
		{ Release(); }

private:

	IDirect3DVertexBuffer8* m_pVB;
	IDirect3DIndexBuffer8*	m_pIB;

	IDirect3DDevice8		*m_pDev;         // d3d device

	D3DCAPS8				m_d3dCaps;
	LPDIRECT3DTEXTURE8		m_pShadeTexture;

	DWORD					m_dwFVF;
	DWORD					m_dwNumVertices;

	DWORD					m_vsHandle;
	D3DXMATRIX				m_OrigWorldMatrix;

	HRESULT LoadXFile(const char* fileName, bool& bVBCreated);
	void SetupMatrices(const Extents& e);

	HRESULT InitD3D(HWND hwnd);
	HRESULT InitVB(const char* fileName);
	HRESULT InitVSStuff();
	HRESULT SetupVSMatrices();
};

#endif