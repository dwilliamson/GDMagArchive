// AGP_Performance.cpp App Wizard Version 2.0 Beta 1
// ----------------------------------------------------------------------
// 
// Copyright © 2001 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative works of this 
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  This software is provided "AS IS." 
//
// Intel specifically disclaims all warranties, express or implied, and all liability,
// including consequential and other indirect damages, for the use of this software, 
// including liability for infringement of any proprietary rights, and including the 
// warranties of merchantability and fitness for a particular purpose.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "stdafx.h"


//
// This is the function where the majority of the work get's done so I put it first.
//
// A couple things to note:  To do fast string moves (using "rep movsd") you need to
// make sure that the data pointers are 64-byte aligned.  For our case, this is easy
// just by having a temporary buffer.
//
HRESULT AGP_Performance::UpdateWorld()
{
	LPDIRECT3DVERTEXBUFFER8 pVB;
	//
	// Allocate a temporary buffer so that we can make sure the pointer, temp,
	// is 64-byte aligned.  Otherwise, the "rep movsd" inline assembly below 
	// here may perform poorly
	//
	BYTE tmpBuffer[128];
	D3DVERTEX *temp = (D3DVERTEX *)( ((DWORD)tmpBuffer+63) & ~63 );
	int Floats = mFloatsToTouch; // Need a local variable that the inline assembly can get at
	D3DVERTEX *pVerts, *pSrcVerts, *pTempSrc, *pTempDst;
	
	mpWrapper->mpDevice->SetTransform(D3DTS_WORLD, &mWorldMat.mMx);
	mpWrapper->mpDevice->SetTransform(D3DTS_VIEW, &mViewMat.mMx);

	//
	// Update the value for our sine wave
	//
	mTheta += mpWrapper->GetFrameTime();
	if( mTheta >= 1.0 )
		mTheta -= 1.0;
	
	//
	// We could have more than one sphere
	//
	for( int idx = 0; idx < NUM_SPHERES; idx++ )
	{
		//
		// Get a pointer to the Dx8 vertex buffer
		//
		pVB = mpSphere[idx]->GetVBptr();
		//
		// Get a pointer to the system memory copy of the vertices
		//
		pSrcVerts = (D3DVERTEX *)mpSphere[idx]->GetVerticesPtr();
		//
		// Lock the vertex buffer -- hopefully we get back a pointer
		// in AGP memory
		//
		pVB->Lock( 0, 0, (BYTE **)&pVerts, 0 );

		//
		// Update the vertices by changing the radius using a sine wave
		//
		float r;
		D3DVECTOR d;
		r = mRadius + mRadius * 0.5f * (float)sin( mTheta * 2.0f * PI );

		//
		// mVertexSkip is either 1 or 2.
		//
		for( int i=0; i<mpSphere[idx]->GetNumVerts(); i+=mVertexSkip )
		{
			//
			// Make a local copy of the original vertices modified by the
			// new radius, r.  Also, copy over the original normal and
			// texture coordinates so we can do a fast move from the local, stack
			// memory (temp) to the AGP/WC memory (pVerts)
			temp[0].dvX = pSrcVerts[i].dvX * r;
			temp[0].dvY = pSrcVerts[i].dvY * r;
			temp[0].dvZ = pSrcVerts[i].dvZ * r;
			temp[0].dvNX = pSrcVerts[i].dvNX;
			temp[0].dvNY = pSrcVerts[i].dvNY;
			temp[0].dvNZ = pSrcVerts[i].dvNZ;
			temp[0].dvTU = pSrcVerts[i].dvTU;
			temp[0].dvTV = pSrcVerts[i].dvTV;
			temp[1].dvX = pSrcVerts[i+1].dvX * r;
			temp[1].dvY = pSrcVerts[i+1].dvY * r;
			temp[1].dvZ = pSrcVerts[i+1].dvZ * r;
			temp[1].dvNX = pSrcVerts[i+1].dvNX;
			temp[1].dvNY = pSrcVerts[i+1].dvNY;
			temp[1].dvNZ = pSrcVerts[i+1].dvNZ;
			temp[1].dvTU = pSrcVerts[i+1].dvTU;
			temp[1].dvTV = pSrcVerts[i+1].dvTV;
			if( Floats > 0 )
			{
				pTempSrc = temp;
				pTempDst = &pVerts[i];
				//
				// I used inline assembly so I didn't have a C++ 'for' loop that
				// would degrade performance.  The best way to blast these bytes
				// over is using a 64-bit aligned string move operation
				//
				_asm
				{
					push ecx
					push esi
					push edi
					mov ecx, Floats
					mov esi, pTempSrc
					mov edi, pTempDst
					rep movsd
					pop edi
					pop esi
					pop ecx
				}
			}
		}
		
		pVB->Unlock();
	}
	
	mpRootObj->Update(mpWrapper->GetFrameTime(), true);
	
	//update frame counter
	mTimeSinceLastFpsUpdate += mpWrapper->GetFrameTime();
	mNumFrames++;
	if (mTimeSinceLastFpsUpdate >= 1.0f) //update the frame counter once in a while
	{
		DWORD dwFps = (DWORD)((float)mNumFrames/ mTimeSinceLastFpsUpdate );
		char fpsstring[10];
		sprintf(fpsstring, "%04d", dwFps);
		mpTextMgr->ChangeStringContent(mStringId[0],fpsstring);
		//
		// Reset our counters
		//
		mNumFrames = 0;
		mTimeSinceLastFpsUpdate = 0.0f;
	}
	
	
	return S_OK;
}


// Constructor
AGP_Performance::AGP_Performance()
{
	mpWrapper = NULL;

	//
	// Setup up some stuff about the camera
	//
	mWorldMat.SetIdentity();
	mViewMat.SetView(CAMERA_POS, CAMERA_TARGET, IawVector(0, 1, 0));
	mProjectionMat.SetProjection(CAMERA_FOV, 0.75f, CAMERA_NEARCLIP, CAMERA_FARCLIP);
	
	mMousePrevX = 0;
	mMousePrevY = 0;
	
	mpRootObj = NULL;
	
	
	mpFontTexture = NULL;
	mpTextMgr = NULL;
	
	mTimeSinceLastFpsUpdate = 0;
	mNumFrames = 0;
	
	mFloatsToTouch = 0;
	mTheta = 0.0f;
	mVertexSkip = 2;
	
	mpShaderManager = NULL;
	
	
}

// Destructor
AGP_Performance::~AGP_Performance()
{

}

HRESULT AGP_Performance::InitWorld(IawD3dWrapper* pWrapper)
{
	int i; 
	
	//
	// Save our own pointer to the wrapper
	//
	mpWrapper = pWrapper;
	
	//
	// Create a root object to hold the objects in the scene
	//
	mpRootObj = new IawObject();
	mpRootObj->Init(pWrapper);
	mpRootObj->SetObjectName("Root Obj");
		
	
	//
	// Display some text strings
	//
	mpTextMgr = new IawTextMgr();
	mpTextMgr->Init(mpWrapper,/*NULL,*/ 0);
	mpTextMgr->AddString("XXXX", &mStringId[0], 0.1f, 0.9f, 0.05f, 0.07f);
	mpTextMgr->AddString("FPS", &mStringId[1], 0.33f, 0.9f, 0.05f, 0.07f);
	mpTextMgr->AddString("Touch 0 Floats at", &mStringId[2], 0.1f, 0.75f, 0.04f, 0.07f);
	mpTextMgr->AddString("Every Other Vertex", &mStringId[3], 0.1f, 0.825f, 0.04f, 0.07f);
	
	mpFontTexture = new IawTexture(mpWrapper);
	mpFontTexture->CreateTexture(IawTexture::FONT_TEXTURE_SIZE,
		IawTexture::FONT_TEXTURE_SIZE,
		16,
		0,
		IawTexture::IAW_TEXTR_CREATE_FONT,
		NULL);
	
	
	//
	// Create some spheres
	//
	mRadius = 1.0f;
	int cnt = (int)sqrt(NUM_SPHERES);
	int idx = 0;
	float half_width = (cnt > 1) ? 2.0f : 0.0f;
	float spacing = (cnt > 1) ? 4.0f / (float)(cnt-1): 0.0f;
	float x, y;
	for( i=0; i<cnt; i++ )
	{
		y = -half_width + i * spacing;
		for( int j=0; j<cnt; j++ )
		{
			x = -half_width + j * spacing;
			mpSphere[idx] = new IawSphere();
			mpSphere[idx]->Init( mpWrapper, mRadius, 105 );
			mpRootObj->AddChild( mpSphere[idx] );
			mpSphere[idx]->SetPos( IawVector( x, y, 0.0f ) );
			idx++;
		}
	}
	

	// Add the text manager to the root
	mpRootObj->AddChild(mpTextMgr);
	
	//
	// Create a shader manager, even though we aren't doing any fancy shading
	//
	mpShaderManager = new IawShaderMgr("Shdr Mgr Test");
	
	int shader, imp, comp, element;
	i = 0;
	mpShaderManager->CreateShader(&shader, "Default Shader");  
    mpShaderManager->CreateShaderImplementation(shader, &imp, "DS Ver 1");
    mpShaderManager->CreateShaderComponent(shader, imp, &comp, "DS V1, Comp 1");
	mpShaderManager->CreateElement(&element, "Default renderstate");
	mpShaderManager->SetComponentElement(shader, imp, comp, element);
	
	
	mpShaderManager->CreateShader(&shader, "Font Shader");   
	mpShaderManager->CreateShaderImplementation(shader, &imp, "Font Ver 1");
	mpShaderManager->CreateShaderComponent(shader, imp, &comp, "FNT V1, Comp 1");
	mpShaderManager->CreateElement(&element, "Font state block");
	mpShaderManager->SetComponentElement(shader, imp, comp, element);
	
	
	mpRootObj->SetShaderID(mpShaderManager->GetShaderId("Default Shader"));
	mpTextMgr->SetShaderID(mpShaderManager->GetShaderId("Font Shader"), true);
	
	return TargetChanged();
}

HRESULT AGP_Performance::DestroyWorld()
{
	//
	// TargetChanging cleans up most things, so call it first
	//
	TargetChanging();
	
	SAFE_DELETE(mpRootObj);
	
	//
	// Delete the font texture
	//
	SAFE_DELETE(mpFontTexture);      
	
	return S_OK;
}

HRESULT AGP_Performance::RenderWorld()
{
	HRESULT hr;
	hr = S_OK;
	
	mpWrapper->mpDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	mpWrapper->mpDevice->SetRenderState(D3DRS_NORMALIZENORMALS , TRUE);
	//mpWrapper->mpDevice->SetRenderState(D3DRS_CULLMODE , D3DCULL_NONE);
	
	
	
	
	if (mpRootObj)
		hr = mpRootObj->Render(mWorldMat, true);
	
	
	
	
	
	return S_OK;
}

HRESULT AGP_Performance::TargetChanging()
{
	HRESULT hr;
	
	if (!mpWrapper || !(mpWrapper->mpDevice))
		return S_OK;
	
	mpWrapper->mpDevice->SetTexture(0, NULL);
	mpWrapper->mpDevice->SetTexture(1, NULL);
	mpWrapper->mpDevice->SetTexture(2, NULL);
	mpWrapper->mpDevice->SetTexture(3, NULL);
	mpWrapper->mpDevice->SetTexture(4, NULL);
	mpWrapper->mpDevice->SetTexture(5, NULL);
	mpWrapper->mpDevice->SetTexture(6, NULL);
	mpWrapper->mpDevice->SetTexture(7, NULL);
	
	mpShaderManager->TargetChanging(mpWrapper); //this will delete all state blocks and invalidate all shaders
	hr = mpRootObj->TargetChanging(mpWrapper, true);
	
	SAFE_RELEASE(mpFontTexture->mpTexture);      
	
	
	return hr;
	
}

HRESULT AGP_Performance::TargetChanged()
{
	HRESULT hr = 0;
	
	if (mpFontTexture)
		mpFontTexture->Restore();
	
	mpWrapper->mpDevice->SetTransform(D3DTS_WORLD, &mWorldMat.mMx);
	mpWrapper->mpDevice->SetTransform(D3DTS_VIEW, &mViewMat.mMx);
	mpWrapper->mpDevice->SetTransform(D3DTS_PROJECTION, &mProjectionMat.mMx);
	
	//setup a default lightsource
	D3DLIGHT8 templight;
	
	ZeroMemory(&templight, sizeof(D3DLIGHT8));
	
	templight.Specular.a = 1.0f;
	templight.Specular.b = 0.0f;
	templight.Specular.g = 0.0f;
	templight.Specular.r = 0.0f;
	
	templight.Diffuse.a = 1.0f;
	templight.Diffuse.b = 0.8f;
	templight.Diffuse.g = 0.8f;
	templight.Diffuse.r = 0.8f;
	
	templight.Ambient.a = 1.0f;
	templight.Ambient.b = 0.2f;
	templight.Ambient.g = 0.2f;
	templight.Ambient.r = 0.2f;
	
	templight.Type = D3DLIGHT_DIRECTIONAL;
	
	templight.Direction.x = 1.0f;
	templight.Direction.y = -0.75f;
	templight.Direction.z = 0.5f;
	
	templight.Position.x = -2.0f;
	templight.Position.y = 0.0f;
	templight.Position.z = 0.0f;
	
	templight.Range = 1000;
	
	
	mpWrapper->mpDevice->SetLight(0, &templight);
	mpWrapper->mpDevice->LightEnable(0, TRUE);
	
	
	D3DMATERIAL8 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL8));
	mtrl.Diffuse.r = 1.0f; mtrl.Ambient.r = 0.0f;
	mtrl.Diffuse.g = 1.0f; mtrl.Ambient.g = 0.0f;
	mtrl.Diffuse.b = 1.0f; mtrl.Ambient.b = 0.0f;
	mtrl.Diffuse.a = 1.0f; mtrl.Ambient.a = 0.0f;
	mpWrapper->mpDevice->SetMaterial(&mtrl);
	
	mpWrapper->mpDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
	
	//set default renderstates  THIS SHOULD BE A SEPARATE FUNCTION WHERE WE SET THEM ALL
	mpWrapper->mpDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	
	mpRootObj->TargetChanged(mpWrapper, mpShaderManager, true);
	
	SetupShaders();
	
	
	
	return hr;
	
}

//The shader hierarchy has been set up in InitWorld() already. This is where the device
// dependant components get defined/setup
HRESULT AGP_Performance::SetupShaders()
{
	HRESULT hr;
	
	mpShaderManager->TargetChanging(mpWrapper);
	
	DWORD dwDefaultShaderStateBlock;
	
	hr = mpWrapper->mpDevice->BeginStateBlock();
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    hr = mpWrapper->mpDevice->SetTexture(0,NULL);
    hr = mpWrapper->mpDevice->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); //disable other stages
    hr = mpWrapper->mpDevice->SetTextureStageState(1,D3DTSS_ALPHAOP, D3DTOP_DISABLE); //disable other stages
    hr = mpWrapper->mpDevice->SetTexture(1,NULL);
    
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    
    hr = mpWrapper->mpDevice->EndStateBlock(&dwDefaultShaderStateBlock);
    
    hr = mpShaderManager->SetElementStateBlock("Default renderstate", dwDefaultShaderStateBlock);
	
    
    DWORD dwFontShaderStateBlock;
    
    hr = mpWrapper->mpDevice->BeginStateBlock();
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    hr = mpWrapper->mpDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    hr = mpWrapper->mpDevice->SetTexture(0,mpFontTexture->mpTexture);
    hr = mpWrapper->mpDevice->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); //disable other stages
    hr = mpWrapper->mpDevice->SetTextureStageState(1,D3DTSS_ALPHAOP, D3DTOP_DISABLE); //disable other stages
    hr = mpWrapper->mpDevice->SetTexture(1,NULL);
    
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    hr = mpWrapper->mpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    
    hr = mpWrapper->mpDevice->EndStateBlock(&dwFontShaderStateBlock);
    
    hr = mpShaderManager->SetElementStateBlock("Font state block", dwFontShaderStateBlock);
    
    
    return S_OK;
}




void AGP_Performance::KeyDown(WPARAM wParam, bool bShift )
{ 
	char szTemp[50];
	switch (wParam) 
	{
	case '0':
	case '1': 
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		mFloatsToTouch = (int)wParam - (int)'0';
		if( bShift )
		{
			mFloatsToTouch += 8;
			mVertexSkip = 2;
		}
		sprintf( szTemp, "Touch %-1d Floats at", mFloatsToTouch );
		mpTextMgr->ChangeStringContent( mStringId[2], szTemp );
		if( mVertexSkip == 1 )
			mpTextMgr->ChangeStringContent( mStringId[3], "Every Vertex      " );
		else
			mpTextMgr->ChangeStringContent( mStringId[3], "Every Other Vertex" );
		break;
	case 'v':
	case 'V':
		mVertexSkip = 3 - mVertexSkip;
		if( mVertexSkip == 1 )
		{
			if( mFloatsToTouch > 8 )
			{
				mFloatsToTouch -= 8;
				sprintf( szTemp, "Touch %-1d Floats at", mFloatsToTouch );
				mpTextMgr->ChangeStringContent( mStringId[2], szTemp );
			}
			mpTextMgr->ChangeStringContent( mStringId[3], "Every Vertex      " );
		}
		else
			mpTextMgr->ChangeStringContent( mStringId[3], "Every Other Vertex" );
		break;
	}
	return;
}

void AGP_Performance::KeyUp(WPARAM wParam)
{ 
	return;
}

void AGP_Performance::MouseLeftUp(WPARAM wParam, LPARAM lParam)
{ 
	return;
}

void AGP_Performance::MouseLeftDown(WPARAM wParam, LPARAM lParam)
{   
	return;
}

void AGP_Performance::MouseRightUp(WPARAM wParam, LPARAM lParam)
{ 
	return;
}

void AGP_Performance::MouseRightDown(WPARAM wParam, LPARAM lParam)
{   
	return;
}

void AGP_Performance::MouseMove(WPARAM wParam, LPARAM lParam)
{
	if (wParam & MK_LBUTTON)  // Left button down plus mouse move == rotate
	{
		//we need a variable to modify our rotation speed with frame rate 
		float scaler = 1.0f;
		if (mpWrapper) //we could get into this loop after the wrapper has been invalidates
		{
			scaler = 2.0f / 640;//max(mpWrapper->m_dwRenderWidth, mpWrapper->m_dwRenderHeight);  
		}
		
		//we need to compare the last position to the current position
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		
		if (x - mMousePrevX)
		{
			mViewMat.Rotate(mViewMat.mMx._12, mViewMat.mMx._22, mViewMat.mMx._32, 3.14159f*scaler*(float)(x - mMousePrevX));
		}
		
		if (y - mMousePrevY)
		{
			mViewMat.Rotate(mViewMat.mMx._11, mViewMat.mMx._21, mViewMat.mMx._31, 3.14159f*scaler*(float)(y - mMousePrevY));
		}
		
		mpWrapper->mpDevice->SetTransform(D3DTS_WORLD, &mWorldMat.mMx);
		mpWrapper->mpDevice->SetTransform(D3DTS_VIEW, &mViewMat.mMx);
		
		
		mMousePrevX = x;
		mMousePrevY = y;
		
	}
	else if (wParam & MK_RBUTTON) // Right button down plus mouse move == Zoom camera in/out
	{
		//we need a variable to modify our rotation speed with frame rate 
		float scaler = 1.0f;
		if (mpWrapper) //we could get into this loop after the wrapper has been invalidates
		{
			scaler = 5.0f / 640.0f;//max(mpWrapper->m_dwRenderWidth, mpWrapper->m_dwRenderHeight);  
		}
		
		//we need to compare the last position to the current position
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		
		if (y - mMousePrevY)
		{
			IawMatrix Temp(true);
			
			Temp.Translate(0,0,scaler * (y - mMousePrevY));
			mViewMat.PostMultiply(Temp);
		}
		
		mpWrapper->mpDevice->SetTransform(D3DTS_WORLD, &mWorldMat.mMx);
		mpWrapper->mpDevice->SetTransform(D3DTS_VIEW, &mViewMat.mMx);
		
		
		mMousePrevX = x;
		mMousePrevY = y;
		
	}
	else
	{
		//need to update these, or the next time someone clicks, the delta is massive
		mMousePrevX = LOWORD(lParam);
		mMousePrevY = HIWORD(lParam); 
	}
	return;
}

