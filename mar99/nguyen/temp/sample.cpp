// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : sample.cpp
// Description : source with an simple example which shows every steps 
//				needed to use shadow-textures.
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------

#pragma warning(disable:4305)

#include "copybuf.hpp"
#include "sample.hpp"
#include "tools.hpp"
#include <math.h>


bool		Capture=false;
Mesh		Scenery;
Mesh		Man;
// debug
Mesh		Cube;
Mesh		Quad;
// debug
GrVertex	v1, v2, v3;

Camera		MyCamera;
Camera		Spot;
Camera*		CurrentCam=&MyCamera;

unsigned short ShadowBitmap[256*256];
unsigned short ScreenShoot[640*480];

extern float ReceiverVertices[8*3];
extern int ReceiverTriangles[12*3];

// debug
extern float CubeVertices[8*3];
extern int CubeTriangles[12*3];
extern float QuadVertices[8*3];
extern int QuadTriangles[12*3];
// debug

extern float ManVertices[899*3];
extern int ManTriangles[648*3];



void InitScene(void)
{
	OpenRaster();

	Scenery.Create(234, 444, ReceiverVertices, ReceiverTriangles);
	Scenery.SetPosition(0,0,0000);

	Man.Create(899, 648, ManVertices, ManTriangles);
	Man.SetPosition(310, -720, 695);

	// debug
	Cube.Create(8, 12, CubeVertices, CubeTriangles);
	Quad.Create(4, 2, QuadVertices, QuadTriangles);
	// debug

	MyCamera.Euler(0,0,0);
	MyCamera.SetPosition(0,0,-1000);

	MyCamera.Orient.xx =  0.879228;
	MyCamera.Orient.xy =  0.471921;
	MyCamera.Orient.xz = -0.0663689;
	MyCamera.Orient.yx =  0.148030;
	MyCamera.Orient.yy = -0.402822;
	MyCamera.Orient.yz = -0.903318;
	MyCamera.Orient.zx = -0.452983;
	MyCamera.Orient.zy =  0.784357;
	MyCamera.Orient.zz = -0.424002;
	MyCamera.Orient.tx =  1231.88;
	MyCamera.Orient.ty = -1600.77;
	MyCamera.Orient.tz =  1904.49;
	MyCamera.Orient.xw =  0.0;
	MyCamera.Orient.yw =  0.0;
	MyCamera.Orient.zw =  0.0;
	MyCamera.Orient.tw =  1.0;

	MyCamera.SetViewport(640, 480);
	MyCamera.SetFocus(90);

	Spot.Orient.xx =  0.924317;
	Spot.Orient.xy = -0.379898;
	Spot.Orient.xz =  0.0380050;
	Spot.Orient.yx = -0.183297;
	Spot.Orient.yy = -0.528908;
	Spot.Orient.yz = -0.828739;
	Spot.Orient.zx =  0.334908;
	Spot.Orient.zy =  0.759026;
	Spot.Orient.zz = -0.558493;
	Spot.Orient.tx =    64.9200;
	Spot.Orient.ty = -1367.73;
	Spot.Orient.tz =  1219.71;
	Spot.Orient.xw = 0.000000;
	Spot.Orient.yw = 0.000000;
	Spot.Orient.zw = 0.000000;
	Spot.Orient.tw = 1.00000;

	Spot.SetViewport(256, 256);
	Spot.SetFocus(70);
}



Hmatrix4x4 InvLocal;
Hvector CamInMeshFrame;

void DrawFlatMesh(Mesh &m, int Black)
{
	unsigned int constant;
	unsigned char r,g,b;
	float prod;

	InvLocal = m.Local;
	InvLocal.Inverse();
	CamInMeshFrame = Spot.GetZAxis();
	CamInMeshFrame = Product(InvLocal, CamInMeshFrame);

	for (int i=0; i<m.nTriangles; i++)
	{
		int i1 = m.TriangleArray[i*3+0];
		int i2 = m.TriangleArray[i*3+1];
		int i3 = m.TriangleArray[i*3+2];

		if (
			((m.VertexArray[ i1 ].flags | m.VertexArray[ i2 ].flags | m.VertexArray[ i3 ].flags) 
			& Vertex::CLIPZ) == 0)
		{
			prod = 
			(
			 ((m.VertexArray[i3].px-m.VertexArray[i1].px)*(m.VertexArray[i2].py-m.VertexArray[i1].py))
			-((m.VertexArray[i2].px-m.VertexArray[i1].px)*(m.VertexArray[i3].py-m.VertexArray[i1].py))
			);
			if (prod>0.0f)
			{
				v1.x	= m.VertexArray[ m.TriangleArray[i*3+0] ].px;
				v1.y	= m.VertexArray[ m.TriangleArray[i*3+0] ].py;
				v1.ooz	= m.VertexArray[ m.TriangleArray[i*3+0] ].pz * 65536.0f;
				v1.oow	= m.VertexArray[ m.TriangleArray[i*3+0] ].pz;
				v1.a = 0.0;
				v1.r = 0.0;
				v1.g = 0.0;
				v1.b = 0.0;
				
				v2.x	= m.VertexArray[ m.TriangleArray[i*3+1] ].px;
				v2.y	= m.VertexArray[ m.TriangleArray[i*3+1] ].py;
				v2.ooz	= m.VertexArray[ m.TriangleArray[i*3+1] ].pz * 65536.0f;
				v2.oow	= m.VertexArray[ m.TriangleArray[i*3+1] ].pz;
				v2.a = 0.0;
				v2.r = 0.0;
				v2.g = 0.0;
				v2.b = 0.0;

				v3.x	= m.VertexArray[ m.TriangleArray[i*3+2] ].px;
				v3.y	= m.VertexArray[ m.TriangleArray[i*3+2] ].py;
				v3.ooz	= m.VertexArray[ m.TriangleArray[i*3+2] ].pz * 65536.0f;
				v3.oow	= m.VertexArray[ m.TriangleArray[i*3+2] ].pz;
				v2.a = 0.0;
				v2.r = 0.0;
				v2.g = 0.0;
				v2.b = 0.0;

				if (Black!=0)
				{
					// Render the objects with lighting
					// used for STEP-4
					Hvector &Normal = m.TriangleNormalArray[i];
					float result = - Dot_product(Normal, CamInMeshFrame);
					if (result<0.0f) result=0.0f;

					r = g = b = (unsigned int) (result * 255);
					constant = (r<<16)+(g<<8)+b;
				}
				else
				{
					// Render the objects with a "special"
					// color, used for STEP-1
					unsigned char darkness=30;	// the "darkness" variable allow you to
												// adjust the "transparency" of the shadow
												// 30 = fully opaque
												//  0 = totally transparent
					darkness = (darkness<<3);
					constant = 0xFF000000 | darkness << 16;
				}
				grConstantColorValue( constant);
				guDrawTriangleWithClip( &v1, &v2, &v3);
			} // if (prod>0.0f)
		} // Vertex::CLIPZ
	} // for
} // function



void LoadTextureOnVoodoo(void)
{
	GrTexInfo info;
	
	info.smallLod	 = GR_LOD_256;
	info.largeLod	 = GR_LOD_256;
	info.aspectRatio = GR_ASPECT_1x1;
	info.format	     = GR_TEXFMT_ARGB_4444;
	info.data		 = ShadowBitmap;

	grTexDownloadMipMap(
		GR_TMU0,
		0,									// start addr
		GR_MIPMAPLEVELMASK_BOTH,
		&info					
		);

	// this should be done only once only since
	// I always use the same adress and texture format
	grTexSource(GR_TMU0,					
				0,							// startaddr
				GR_MIPMAPLEVELMASK_BOTH,
				&info
				);
}



void CastOnMesh(Mesh &m)
{
	float prod;

	for (int i=0; i<m.nTriangles; i++)
	{
		int v1= m.TriangleArray[i*3+0];
		int v2= m.TriangleArray[i*3+1];
		int v3= m.TriangleArray[i*3+2];
		m.FlagArray[i] = 0;								// by default, reset the flag

		// Backface Culling
		//((v3.x - v1.x) * (v2.y - v1.y)) - ( (v2.x-v1.x)*(v3.y-v1.y) )
		prod = 
		(
		 ((m.VertexArray[v3].px-m.VertexArray[v1].px)*(m.VertexArray[v2].py-m.VertexArray[v1].py))
		-((m.VertexArray[v2].px-m.VertexArray[v1].px)*(m.VertexArray[v3].py-m.VertexArray[v1].py))
		);
		if (prod<0.0f) continue;						// reject by BackFace Culling

		if ((m.VertexArray[v1].flags &
			 m.VertexArray[v2].flags &
			 m.VertexArray[v3].flags) != 0) continue;	// reject if the triangle is completely
														// outside of the light POV
		m.VertexArray[ v1 ].u = m.VertexArray[ v1 ].px;
		m.VertexArray[ v1 ].v = m.VertexArray[ v1 ].py;

		m.VertexArray[ v2 ].u = m.VertexArray[ v2 ].px;
		m.VertexArray[ v2 ].v = m.VertexArray[ v2 ].py;

		m.VertexArray[ v3 ].u = m.VertexArray[ v3 ].px;
		m.VertexArray[ v3 ].v = m.VertexArray[ v3 ].py;
		m.FlagArray[i] = 1;								// 1= draw this triangle for the current frame
	}
}



void DrawShadow(Mesh &m)
{
	guAlphaSource( GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA );

	grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
						GR_BLEND_ONE_MINUS_SRC_ALPHA,
						GR_BLEND_ONE,
						GR_BLEND_ZERO );

	grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_ADD);
	
	guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
	grColorCombine(
		GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_LOCAL,
		GR_COMBINE_LOCAL_ITERATED,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);

	grTexClampMode(GR_TMU0,GR_TEXTURECLAMP_CLAMP,GR_TEXTURECLAMP_CLAMP);

	for (int i=0; i<m.nTriangles; i++)
	{
		int i1 = m.TriangleArray[i*3+0];
		int i2 = m.TriangleArray[i*3+1];
		int i3 = m.TriangleArray[i*3+2];

		if ( m.FlagArray[i] == 0) continue;

		if (
			((m.VertexArray[ i1 ].flags | m.VertexArray[ i2 ].flags | m.VertexArray[ i3 ].flags) 
			& Vertex::CLIPZ) == 0)
		{
			v1.x	= m.VertexArray[ i1 ].px;
			v1.y	= m.VertexArray[ i1 ].py;
			v1.oow	= m.VertexArray[ i1 ].pz;
			v1.tmuvtx[0].sow = m.VertexArray[ i1 ].u * v1.oow;
			v1.tmuvtx[0].tow = m.VertexArray[ i1 ].v * v1.oow;
			v1.r = 255.0f;
			v1.g = 255.0f;
			v1.b = 255.0f;

			v2.x	= m.VertexArray[ i2 ].px;
			v2.y	= m.VertexArray[ i2 ].py;
			v2.oow	= m.VertexArray[ i2 ].pz;
			v2.tmuvtx[0].sow = m.VertexArray[ i2 ].u * v2.oow;
			v2.tmuvtx[0].tow = m.VertexArray[ i2 ].v * v2.oow;
			v2.r = 255.0f;
			v2.g = 255.0f;
			v2.b = 255.0f;

			v3.x	= m.VertexArray[ i3 ].px;
			v3.y	= m.VertexArray[ i3 ].py;
			v3.oow	= m.VertexArray[ i3 ].pz;
			v3.tmuvtx[0].sow = m.VertexArray[ i3 ].u * v3.oow;
			v3.tmuvtx[0].tow = m.VertexArray[ i3 ].v * v3.oow;
			v3.r = 255.0f;
			v3.g = 255.0f;
			v3.b = 255.0f;

			v1.a = 170.0f;	// use the "Iterated Alpha" to adjust the 
			v2.a = 170.0f;  // "transparency" of the shadow for performing 
			v3.a = 170.0f;	// special effects like gradients and so on...

			grConstantColorValue( 0xFFAA9977 );
			guDrawTriangleWithClip( &v1, &v2, &v3);
		}
	}
    grColorCombine( GR_COMBINE_FUNCTION_LOCAL,	// turn OFF the "texture mapping"
                    GR_COMBINE_FACTOR_NONE,
                    GR_COMBINE_LOCAL_CONSTANT,
					GR_COMBINE_OTHER_NONE,
                    FXFALSE );

	grAlphaBlendFunction( GR_BLEND_ONE,			// disable the "alpha blending"
						GR_BLEND_ZERO,
						GR_BLEND_ONE,
						GR_BLEND_ZERO );
}


void Convert_565to1555(unsigned short *Screen, int Width, int Height)
{
	int r,g,b;
	for (int i=0; i<Width*Height; i++)
	{
		r = (Screen[i] >> 11) & 31;
		g = (Screen[i] >> 6) & 31;
		b = (Screen[i]) & 31;
		Screen[i] = (r<<10 | g<<5 | b);
	}
}





void InnerLoop(void)
{
	static int ManAngle=0;

	// STEP-1 
	// Render Shadow Texture
	//----------------------
	SetSpotCamera();
	grClipWindow(0,0,256,256);					// clips only a 256x256 viewport
	grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
	ClearBuffer(0xFF000000);					// clears a 256x256 viewport without clearing Zbuffer
	Scenery.Rotate(*CurrentCam);	
//	DrawFlatMesh(Scenery,1);		// debug

	Man.Euler(0,0,ManAngle);
	ManAngle+=20;								// increase an Euler angle to animate the character
	Man.Rotate(*CurrentCam);
	DrawFlatMesh(Man,0);
	grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
	grClipWindow(0,0,640,480); // clips a 640x480 viewport

	
	// STEP-2 
	// read the rendered picture from STEP-1 
	// and use it as a texture
	//-------------------------------------------
	grLfbReadRegion(GR_BUFFER_BACKBUFFER,	// copy from the LFB to a buffer in
		0,			// x					// SYSTEM MEMORY
		0,			// y
		256,		// width
		256,		// height
		256*2,		// stride in bytes
		ShadowBitmap
		);

	LoadTextureOnVoodoo();					// download from the buffer in SYSTEM MEMORY
											// to Voodoo Texture Memory
	// copy2video(...) is optional.
//	Convert_565to1555(ShadowBitmap, 256, 256);
//	copy2video((unsigned short *)ShadowBitmap, Hrect(0,0,256,256), Hrect(0,0,256,256), 16);
	

	
	// STEP-3
	// Calculate Text Coords for Shadow Mapping
	//-----------------------------------------

	CastOnMesh(Scenery);	// WARNING !, this operation uses the results of 
							// the previous rotation performed on this 
							// mesh during STEP-1



	// STEP-4
	// Render the Whole scenery
	//-----------------------------------------
	SetScreenCamera();
	ClearBuffer(0x003F3F3F);

	Man.Rotate(*CurrentCam);
	DrawFlatMesh(Man,1);

	Scenery.Rotate(*CurrentCam);
	DrawFlatMesh(Scenery,1);


	
	
	// STEP-5
	// Render the nessecery polys with the
	// shadow texture.
	//-----------------------------------------
	DrawShadow(Scenery);


	
	
	// You can see the results now
	// just swap !!
	//---------------------------
	grBufferSwap( 1 );

	if (Capture)
	{
		grLfbReadRegion(GR_BUFFER_BACKBUFFER,	// copy from the LFB to a buffer in
			0,			// x					// SYSTEM MEMORY
			0,			// y
			640,		// width
			480,		// height
			640*2,		// stride in bytes
			ScreenShoot
			);

		Convert_565to1555(ScreenShoot, 640, 480);
		copy2video((unsigned short *)ScreenShoot, Hrect(0,0,640,480), Hrect(0,0,640,480), 16);
	}
}



void CameraControl(int Movement, int Value)
{
	switch(Movement)
	{
	case Camera::LEFT:
		(*CurrentCam).Rotate(Movement, 5);
		break;
	case Camera::RIGHT:
		(*CurrentCam).Rotate(Movement, 5);
		break;
	case Camera::UP:
		(*CurrentCam).Rotate(Movement, 5);
		break;
	case Camera::DOWN:
		(*CurrentCam).Rotate(Movement, 5);
		break;
	case Camera::ROLL_RIGHT:
		(*CurrentCam).Rotate(Movement, 5);
		break;
	case Camera::ROLL_LEFT:
		(*CurrentCam).Rotate(Movement, 5);
		break;
	case Camera::FORWARD:
		(*CurrentCam).Move(Movement,5);
		break;
	case Camera::BACKWARD:
		(*CurrentCam).Move(Movement,5);
		break;
	}
}




void MeshControl(int Movement, float Value)
{
	Hvector OldPos = Man.GetPosition();
	Man.Move(Movement, Value);
	Hvector NewPos = Man.GetPosition();

	Hvector Delta  = NewPos - OldPos;
	Hvector SpotPos = Spot.GetPosition();
	Spot.SetPosition(SpotPos+Delta);
}




void SetSpotCamera(void)
{
	CurrentCam = &Spot;
}





void SetScreenCamera(void)
{
	CurrentCam = &MyCamera;
}





void ToggleCapture(void)
{
	if (Capture)
		Capture=false;
	else
		Capture=true;
}















