///////////////////////////////////////////////////////////////////////////////
//
// VShader.cpp : implementation file
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
#include "VShader.h"
#include "LoadOBJ.h"
/*
 * helper error macros
 */
#ifdef _DEBUG
inline void SpitError(HRESULT hr, char *szText)
{
    static char szErr[512];
    int cch = _snprintf(szErr, sizeof(szErr), "%s failed 0x%08lx: ", szText, hr);

    D3DXGetErrorString(hr, &szErr[cch], sizeof(szErr));
    lstrcat(szErr, "\n");
    OutputDebugString(szErr);
}

#define DO(x)                                                   \
{                                                               \
    if(HRESULT res = (x))                                       \
        { SpitError(x, #x); return res; }                       \
}

#define DOERR(x)                                                \
{                                                               \
    if(res = (x))                                               \
        { SpitError(x, #x); goto err; }                         \
}

#define DOVERIFY(x)                                             \
{                                                               \
    if(HRESULT res = (x))                                       \
        { SpitError(x, #x); }                                   \
}

#else

#define DO(x)                                                   \
{                                                               \
    if(HRESULT res = (x))                                       \
        { return res; }                                         \
}

#define DOERR(x)                                                \
{                                                               \
    if(res = (x))                                               \
        { goto err; }                                           \
}

#define DOVERIFY(x)  (x)

#endif

const D3DXVECTOR3 vAxisX(1.0f, 0.0f, 0.0f);
const D3DXVECTOR3 vAxisY(0.0f, 1.0f, 0.0f);
const D3DXVECTOR3 vAxisZ(0.0f, 0.0f, 1.0f);

D3DXMATRIX g_WorldMatrix;
D3DXMATRIX g_ViewMatrix;
D3DXMATRIX g_ProjectionMatrix;

///////////////////////////////////////////////////////////////////////////////
//	Function:	CVShader()
//	Purpose:	Class creator
///////////////////////////////////////////////////////////////////////////////
CVShader::CVShader()
{
	m_fillMode = D3DFILL_WIREFRAME;

	m_pVB = NULL;
	m_pIB = NULL;

	m_pDev         = NULL;

	m_dwBackBufferWidth  = 800;
	m_dwBackBufferHeight = 600;

	// Set up the FVF
	m_dwFVF = D3DFVF_XYZ |D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(0);

	// Initialize position of the camera
	m_Camera.pos.x = 0.0f;
	m_Camera.pos.y = 0.0f;
	m_Camera.pos.z = 10.0f;
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.fov = 60.0f;

	// Initialize the light direction
	m_vLightDir.x = 0.3f;
	m_vLightDir.y = 0.2f;
	m_vLightDir.z = -0.8f;
	m_vLightDir.w = 0.0f;

	D3DXVec4Normalize(&m_vLightDir, &m_vLightDir);

	m_Visual.triCnt = 0;
	m_dwNumVertices = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	Release()
//	Purpose:	Release memory for quitting program
///////////////////////////////////////////////////////////////////////////////
void CVShader::Release()
{
	if(m_pVB != NULL)
	{
		m_pVB->Release();
		m_pVB = NULL;
	}

	if(m_pIB != NULL)
	{
		m_pIB->Release();
		m_pIB = NULL;
	}

	if(m_pDev != NULL)
	{
		m_pDev->Release();
		m_pDev = NULL;
	}

}

///////////////////////////////////////////////////////////////////////////////
//	Function:	SetupVSMatrices()
//	Purpose:	Sets up the Vertex Shader Matrices for use within the shader
///////////////////////////////////////////////////////////////////////////////
HRESULT CVShader::SetupVSMatrices()
{
	D3DXMATRIX mvpMat, worldITMat, worldMat;

	D3DXMatrixTranspose(&worldMat, &g_WorldMatrix);

	D3DXMatrixIdentity(&mvpMat);

	D3DXMatrixMultiply(&mvpMat, &g_WorldMatrix, &g_ViewMatrix);

	D3DXMatrixInverse(&worldITMat, NULL, &g_WorldMatrix);

	D3DXMatrixMultiply(&mvpMat, &mvpMat, &g_ProjectionMatrix);
	D3DXMatrixTranspose(&mvpMat, &mvpMat);

	// Set the transformation matrix in Constants 0-3
	DO(m_pDev->SetVertexShaderConstant(0, &mvpMat, 4));

	// Set the inverse-transpose matrix in Constants 4-6
	// Used for normals so only the 3x3 is needed
	DO(m_pDev->SetVertexShaderConstant(4, &worldITMat, 3));

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	InitVSStuff()
//	Purpose:	Sets up initial Vertex Shader constants to use within the shader
///////////////////////////////////////////////////////////////////////////////
HRESULT CVShader::InitVSStuff()
{
	DO(SetupVSMatrices());

	// Set up the Light vector constant in #8
	DO(m_pDev->SetVertexShaderConstant(8, &m_vLightDir, 1));

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	Init()
//	Arguments:	Handle to window and a file name to load
//	Purpose:	Sets up program and builds the D3D objects
///////////////////////////////////////////////////////////////////////////////
HRESULT CVShader::Init(HWND hwnd, const char* fileName)
{
	DO(InitD3D(hwnd));

	DO(InitVB(fileName));

	D3DXMatrixPerspectiveFovLH(&g_ProjectionMatrix,
							D3DXToRadian(m_Camera.fov),
							(float)m_dwBackBufferHeight  / m_dwBackBufferWidth,
							0.1f,
							2000.0f);
	DO(m_pDev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&g_ProjectionMatrix));

	char* vsFileName = "VShader.vsh";
	ID3DXBuffer* vShaderOC;
	ID3DXBuffer* errors;

	// Compiles the text shader
	DO(D3DXAssembleShaderFromFile(vsFileName, NULL, NULL, &vShaderOC, &errors));

	// Declare the vertex format FVF for the stream into the shader
	DWORD dwDecl[] =
	{
		D3DVSD_STREAM( 0 ),
		D3DVSD_REG( 0,  D3DVSDT_FLOAT3 ),
		D3DVSD_REG( 1,  D3DVSDT_FLOAT3 ),
		D3DVSD_REG( 2,  D3DVSDT_FLOAT1 ),
		D3DVSD_END()
	};

	// Create a VS from the compiled shader
	DO(m_pDev->CreateVertexShader(dwDecl, (const DWORD*)vShaderOC->GetBufferPointer(), &m_vsHandle, D3DUSAGE_SOFTWAREPROCESSING));

	D3DCAPS8 d3dCaps;

	m_pDev->GetDeviceCaps( &d3dCaps );
	int IndexedMatrixMaxSize = d3dCaps.MaxVertexBlendMatrixIndex;
	int MatrixMaxSize = d3dCaps.MaxVertexBlendMatrices;
	int temp = d3dCaps.MaxVertexShaderConst;


	//initialize vertex shader constants
	DO(InitVSStuff());

	DO(m_pDev->SetRenderState(D3DRS_ZENABLE, D3DZB_USEW));
	DO(m_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

	DO(m_pDev->SetRenderState(D3DRS_LIGHTING, FALSE));

	DO(m_pDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW));
	DO(m_pDev->SetRenderState(D3DRS_FILLMODE, m_fillMode));

	DO(m_pDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
	DO(m_pDev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0));

	DO(m_pDev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP));
	DO(m_pDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT));
	DO(m_pDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT));
	DO(m_pDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE));

	DO(m_pDev->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP));
	DO(m_pDev->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT));
	DO(m_pDev->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT));
	DO(m_pDev->SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR));

	// Only using one texture stage for this demo
	// Diffuse material color is modulated through the TFactor
	DO(m_pDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
	DO(m_pDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE ));
	DO(m_pDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR));
	DO(m_pDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE ));

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	InitD3D()
//	Arguments:	Handle to window
//	Purpose:	Sets up D3D 
///////////////////////////////////////////////////////////////////////////////
HRESULT CVShader::InitD3D(HWND hwnd)
{
	// Create D3D 8
	IDirect3D8 *pD3D = Direct3DCreate8(D3D_SDK_VERSION);
	if(!pD3D)
		return E_FAIL;

	// Set the screen mode
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth            = m_dwBackBufferWidth;
	d3dpp.BackBufferHeight           = m_dwBackBufferHeight;
	d3dpp.BackBufferFormat           = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount            = 1;
	d3dpp.Windowed                   = m_bWindowed;
	d3dpp.EnableAutoDepthStencil     = TRUE;
	d3dpp.AutoDepthStencilFormat     = D3DFMT_D16;
	d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	d3dpp.Flags                      = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.hDeviceWindow              = hwnd;

	D3DDEVTYPE devType = D3DDEVTYPE_HAL;
	DWORD behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	if(m_bRefRast)
	{
		devType = D3DDEVTYPE_REF;
	}

	// Create the device
	DO(pD3D->CreateDevice(D3DADAPTER_DEFAULT, devType, hwnd, behaviorFlags, &d3dpp, &m_pDev));

	// Now we no longer need the D3D interface so lets free it
	pD3D->Release();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	InitVB()
//	Arguments:	Object filename
//	Purpose:	Sets up the object vertex buffers
///////////////////////////////////////////////////////////////////////////////
HRESULT CVShader::InitVB(const char* fileName)
{
	Extents e;
	UINT uLength;
	D3DPOOL pool;
	int i;

	if (LoadOBJ(fileName,&m_Visual))
	{
		// Create the Vertex buffer
		if (m_Visual.normalCnt > m_Visual.vertexCnt)
		{
			// Set the total number
			m_dwNumVertices = m_Visual.normalCnt;

			uLength = m_dwNumVertices * sizeof(MyVertex);
    
			pool = m_bRefRast ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;

			DO(m_pDev->CreateVertexBuffer(uLength, D3DUSAGE_WRITEONLY, m_dwFVF, pool, &m_pVB));
  
			e.maxX = e.maxY = e.maxZ = -10000;
			e.minX = e.minY = e.minZ = 10000;

			MyVertex* pVertsDst;

			DO(m_pVB->Lock(0, uLength, (BYTE**)&pVertsDst, 0));


			// First set up the normals
			for(i = 0; i < m_dwNumVertices; i++)
			{
				pVertsDst[i].nx = m_Visual.normal[i].x;
				pVertsDst[i].ny = m_Visual.normal[i].y;
				pVertsDst[i].nz = m_Visual.normal[i].z;
  
				pVertsDst[i].u0 = 0.0f;
		//      pVertsDst[i].u1 = 0.0f;

			}

			// Go through triangles to set the vertex positions
			for(i = 0; i < m_dwNumVertices; i++)
			{
				int which = -1;
				for(int t = 0; t < m_Visual.triCnt; t++)
				{
					if (m_Visual.index[t].n[0] == i)
					{
						which = m_Visual.index[t].v[0];
						break;
					}
					if (m_Visual.index[t].n[1] == i)
					{
						which = m_Visual.index[t].v[1];
						break;
					}
					if (m_Visual.index[t].n[2] == i)
					{
						which = m_Visual.index[t].v[2];
						break;
					}
				}
				if (which > -1)
				{
					pVertsDst[i].x = m_Visual.vertex[which].x;
					pVertsDst[i].y = m_Visual.vertex[which].y;
					pVertsDst[i].z = m_Visual.vertex[which].z;
  
					//setup extents
					if(pVertsDst[i].x > e.maxX)
						e.maxX = pVertsDst[i].x;
					if(pVertsDst[i].y > e.maxY)
						e.maxY = pVertsDst[i].y;
					if(pVertsDst[i].z > e.maxZ)
						e.maxZ = pVertsDst[i].z;
					if(pVertsDst[i].x < e.minX)
						e.minX = pVertsDst[i].x;
					if(pVertsDst[i].y < e.minY)
						e.minY = pVertsDst[i].y;
					if(pVertsDst[i].z < e.minZ)
						e.minZ = pVertsDst[i].z;
				}
			}

			DO(m_pVB->Unlock());

			//setup matrices to view this mesh
			SetupMatrices(e);

			D3DFORMAT format = D3DFMT_INDEX16;

			WORD* pIndicesDest;

			uLength = m_Visual.triCnt * 3 * sizeof(*pIndicesDest);

			DO(m_pDev->CreateIndexBuffer(uLength, D3DUSAGE_WRITEONLY, format, pool, &m_pIB));  
			DO(m_pIB->Lock(0, uLength, (BYTE**)&pIndicesDest, 0));

			int indexCtr = 0;
			for(i = 0; i < m_Visual.triCnt; i++)
			{
				pIndicesDest[indexCtr++] = (int)m_Visual.index[i].n[0];
				pIndicesDest[indexCtr++] = (int)m_Visual.index[i].n[1];
				pIndicesDest[indexCtr++] = (int)m_Visual.index[i].n[2];
			}
  
			DO(m_pIB->Unlock());
		}
		else	// More vertices then normals
		{
			// Set the total number
			m_dwNumVertices = m_Visual.vertexCnt;

  			uLength = m_dwNumVertices * sizeof(MyVertex);
    
			pool = m_bRefRast ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;

			DO(m_pDev->CreateVertexBuffer(uLength, D3DUSAGE_WRITEONLY, m_dwFVF, pool, &m_pVB));
  
			e.maxX = e.maxY = e.maxZ = -10000;
			e.minX = e.minY = e.minZ = 10000;

			MyVertex* pVertsDst;

			DO(m_pVB->Lock(0, uLength, (BYTE**)&pVertsDst, 0));

			// First set up the vertices
			for(i = 0; i < m_dwNumVertices; i++)
			{
				pVertsDst[i].x = m_Visual.vertex[i].x;
				pVertsDst[i].y = m_Visual.vertex[i].y;
				pVertsDst[i].z = m_Visual.vertex[i].z;    

				//setup extents
				if(pVertsDst[i].x > e.maxX)
					e.maxX = pVertsDst[i].x;
				if(pVertsDst[i].y > e.maxY)
					e.maxY = pVertsDst[i].y;
				if(pVertsDst[i].z > e.maxZ)
					e.maxZ = pVertsDst[i].z;
				if(pVertsDst[i].x < e.minX)
					e.minX = pVertsDst[i].x;
				if(pVertsDst[i].y < e.minY)
					e.minY = pVertsDst[i].y;
				if(pVertsDst[i].z < e.minZ)
					e.minZ = pVertsDst[i].z;

				pVertsDst[i].u0 = 0.0f;
		//      pVertsDst[i].u1 = 0.0f;

			}

			// Go through triangles to set the normal positions
			for(i = 0; i < m_dwNumVertices; i++)
			{
				int which = -1;
				for(int t = 0; t < m_Visual.triCnt; t++)
				{
					if (m_Visual.index[t].v[0] == i)
					{
						which = m_Visual.index[t].n[0];
						break;
					}
					if (m_Visual.index[t].v[1] == i)
					{
						which = m_Visual.index[t].n[1];
						break;
					}
					if (m_Visual.index[t].v[2] == i)
					{
						which = m_Visual.index[t].n[2];
						break;
					}
				}
				if (which > -1)
				{
					pVertsDst[i].nx = m_Visual.normal[which].x;
					pVertsDst[i].ny = m_Visual.normal[which].y;
					pVertsDst[i].nz = m_Visual.normal[which].z;    
				}
			}

			DO(m_pVB->Unlock());

			//setup matrices to view this mesh
			SetupMatrices(e);

			D3DFORMAT format = D3DFMT_INDEX16;

			WORD* pIndicesDest;

			uLength = m_Visual.triCnt * 3 * sizeof(*pIndicesDest);

			DO(m_pDev->CreateIndexBuffer(uLength, D3DUSAGE_WRITEONLY, format, pool, &m_pIB));  
			DO(m_pIB->Lock(0, uLength, (BYTE**)&pIndicesDest, 0));

			int indexCtr = 0;
			for(i = 0; i < m_Visual.triCnt; i++)
			{
				pIndicesDest[indexCtr++] = (int)m_Visual.index[i].v[0];
				pIndicesDest[indexCtr++] = (int)m_Visual.index[i].v[1];
				pIndicesDest[indexCtr++] = (int)m_Visual.index[i].v[2];
			}

			DO(m_pIB->Unlock());
		}


		// Load the cartoon shade table
		D3DXCreateTextureFromFile(m_pDev, "shades.bmp", &m_pShadeTexture);
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	SetupMatrices()
//	Arguments:	object extents
//	Purpose:	Sets up the world matrices
///////////////////////////////////////////////////////////////////////////////
void CVShader::SetupMatrices(const Extents& e)
{
	float dx = e.maxX - e.minX;
	float dy = e.maxY - e.minY;
	float dz = e.maxZ - e.minZ;

	float midX = (dx / 2) + e.minX;
	float midY = (dy / 2) + e.minY;
	float midZ = (dz / 2) + e.minZ;

	float maxExtent = (dx > dy) ? dx : dy;
	maxExtent = (maxExtent > dz) ? maxExtent : dz;

	//place object at origin
	D3DXMatrixTranslation(&g_WorldMatrix, -midX, -midY, -midZ);

	D3DXVECTOR3 vLookAt;
	D3DXVECTOR3 vUp;
	D3DXVECTOR3	vEyeVec;
	D3DXVECTOR3	vEye;

	vEye.x    = 0.0f; vEye.y    = 0.0f; vEye.z    = e.minZ - (0.4f * maxExtent);
	vLookAt.x = 0.0f; vLookAt.y = 0.0f; vLookAt.z = 0.0f;
	vUp.x     = 0.0f; vUp.y     = 1.0f; vUp.z     = 0.0f;

	vEyeVec = vLookAt - vEye;
	D3DXVec3Normalize(&vEyeVec, &vEyeVec);
	//set view
	D3DXMatrixLookAtLH(&g_ViewMatrix, &vEye, &vLookAt, &vUp);

	//save original world matrix in origWorld
	m_OrigWorldMatrix = g_WorldMatrix;

	D3DXMATRIX currRot,currTrans;
	D3DXMatrixIdentity(&m_Camera.matrix);

	D3DXMatrixRotationAxis(&currRot, &vAxisX, D3DXToRadian(m_Camera.rot.x));
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currRot);
	D3DXMatrixRotationAxis(&currRot, &vAxisY, D3DXToRadian(m_Camera.rot.y));
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currRot);
	D3DXMatrixRotationAxis(&currRot, &vAxisZ, D3DXToRadian(m_Camera.rot.z));
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currRot);

	D3DXMatrixTranslation(&currTrans, m_Camera.pos.x, m_Camera.pos.y, m_Camera.pos.z);
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currTrans);

	D3DXMatrixMultiply(&g_WorldMatrix, &g_WorldMatrix, &m_Camera.matrix);

}

///////////////////////////////////////////////////////////////////////////////
//	Function:	UpdateCamera
//	Purpose:	Called anytime camera parameters change and the matrices need
//				to be rebuilt
///////////////////////////////////////////////////////////////////////////////
void CVShader::UpdateCamera()
{
	D3DXMATRIX currRot,currTrans;
	D3DXMatrixIdentity(&m_Camera.matrix);

	D3DXMatrixRotationAxis(&currRot, &vAxisX, D3DXToRadian(m_Camera.rot.x));
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currRot);
	D3DXMatrixRotationAxis(&currRot, &vAxisY, D3DXToRadian(m_Camera.rot.y));
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currRot);
	D3DXMatrixRotationAxis(&currRot, &vAxisZ, D3DXToRadian(m_Camera.rot.z));
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currRot);

	D3DXMatrixTranslation(&currTrans, m_Camera.pos.x, m_Camera.pos.y, m_Camera.pos.z);
	D3DXMatrixMultiply(&m_Camera.matrix, &m_Camera.matrix, &currTrans);

	D3DXMatrixMultiply(&g_WorldMatrix, &m_OrigWorldMatrix, &m_Camera.matrix);
}

///////////////////////////////////////////////////////////////////////////////
//	Function:	DXRun
//	Purpose:	Main Render Loop
///////////////////////////////////////////////////////////////////////////////
HRESULT CVShader::DXRun()
{
	SetupVSMatrices();

	DO(m_pDev->SetRenderState(D3DRS_FILLMODE, m_fillMode));

	DO(m_pDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00BFBFBF, 1.0f, 0L));

	DO(m_pDev->BeginScene());

	DO(m_pDev->SetVertexShader(m_vsHandle));
	DO(m_pDev->SetStreamSource(0, m_pVB, D3DXGetFVFVertexSize(m_dwFVF)));
	DO(m_pDev->SetIndices(m_pIB, 0));

	DO(m_pDev->SetTexture(0, m_pShadeTexture));
	// Set the object diffuse color as the Texture Factor
	m_pDev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA((int)(255.0f * m_Visual.Kd.x), (int)(255.0f * m_Visual.Kd.y), (int)(255.0f * m_Visual.Kd.z), 0));

	// Draw the triangles
	if (m_Visual.triCnt > 0)
		DO(m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 
			m_dwNumVertices, 0, m_Visual.triCnt));


	DO(m_pDev->EndScene());

	DO(m_pDev->Present(NULL, NULL, NULL, NULL));

	return S_OK;
}
