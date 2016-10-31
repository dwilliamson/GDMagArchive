////////////////////////////////////////////////////////////////////////////
// 
//
// Copyright (c) 2006 Mick West
// http://mickwest.com/
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// HOW TO COMPILE
//
// You should have Visual studio files stroke.sln and stroke.vcproj which will allow
// you to compile under visual studio 2005.
// if not, then the following info should be enough to get you compiling.
//
// The entire project consists of two source files:
//   fluid.cpp (this file)
//   vector2.h  (the 2D vector library)
//
// This code will compile under Visual Studio 2005, including the express 
// edition, available free at:
// http://msdn.microsoft.com/vstudio/express/
//
// You also need the windows SDK
// http://www.microsoft.com/downloads/details.aspx?FamilyId=A55B6B43-E24F-4EA3-A93E-40C0EC4F68E5&displaylang=en
//
// And the DirectX SDK
// http://msdn.microsoft.com/directx/sdk/
//
// Link with the following libraries:
// d3dxof.lib dxguid.lib d3dx9.lib d3d9.lib dxerr9.lib dinput8.lib winmm.lib user32.lib
//  
// You will also need to set the correct include and lib path for the Windows SDK and DirectX SDK.
// these vary by install, but for me were:
//
// C:\Program Files\Microsoft DirectX SDK (December 2005)\Include
// C:\Program Files\Microsoft Platform SDK\Include
// C:\Program Files\Microsoft DirectX SDK (December 2005)\Lib\x86
// C:\Program Files\Microsoft Platform SDK\Lib
//
// Use the ReleaseOPT configuration, debug mode is way too slow
//
// Draw on the square on the left, and you actions will be mirroed on the right.
// right drag to swirl
//
// Important funtions for tweaking:
// 
// CFluid::MouseInput() - Creates the input from the mouse by adding values to the field.
// InitFluids() - sets the initial values for both fluids (does g_fluid1 and copies to g_fluid2)
// WinMain(...) - the main loop, here I hacked in the A/B comparison by making changes to g_fluid2 as needed
//
// Note you can also do A/B comparison with code modifications by using:
//  if (this == &g_fluid2) ....
// There are a few examples of this in the code
//







//#define	FIGURES  // for screenshot figures

// Fluid wrapping needs a bunch of work so that ABCD squares can overlap the right and bottom edges
// which might effectly make the usable area one square bigger.  Leave alone for now.
//#define FLUID_WRAPPING  // NOT QUITE WORKING creates nice tesselating animating turbulent


float g_friction = 1.0f;



#pragma warning(disable: 4995)          // don't warn about deprecated functions
#define _CRT_SECURE_NO_DEPRECATE		// ditto

void    debug_log( const char* text, ...);

//const int   g_viewport_width = 1024;
//const int   g_viewport_height = 768;

const int   g_viewport_width = 1600;
const int   g_viewport_height = 1200;

int has_focus = 1;


bool    g_resize = false;

// System and DirectX includes.
#include <d3d9.h>
#include <d3dx9.h>
#include <strsafe.h>
#include <math.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// local includes
#include "vector2.h"
#include "ICol2d.h"

CRITICAL_SECTION	debug_CS;

#if 1
inline void swap(float*&p0, float *&p1)
{
	float *t = p0;
	p0 = p1;
	p1 = t;
}
#else
#define swap(A,B) {float *t=A;A=B;B=t;}
#endif


bool	dragging=0;
bool	r_dragging=0;
int		mouse_x=0;
int		mouse_y=0;

int	key_left =0;
int key_right = 0;
int key_up = 0;
int key_down = 0;
int key_w=0;
int key_a=0;
int key_s=0;
int key_d=0;
int key_space=0;

bool LineCircleIntersect(Vector2 &start, Vector2 &end, float radius, Vector2 &p1, Vector2 &p2);


class CFluid	
{

public:

	CFluid();
	~CFluid();
	void	Init(int w, int h);

	void ReverseAdvection(float *p_in, float *p_out, float scale);
	void ReverseSignedAdvection(float *p_in, float *p_out, float scale);
	void ForwardAdvection(float *p_in, float *p_out, float scale);
	void PartialForwardAdvection(float *p_in, float *p_out, float scale, float partial);
	void BFECCForwardAdvection(float *p_in, float*p_out, float scale);
	void PressureAcceleration(float force);
	void Diffusion(float *p_in, float *p_out, float scale);
	void GlobalDiffusion(float *p_in, float *p_out, float scale);
	void DiffusionLR(float *p_in, float *p_out, float scale);
	void DiffusionUD(float *p_in, float *p_out, float scale);
	void VelocityFriction(float a, float b, float c);
	void QuadraticDecay(float *p_in, float *p_out, float a, float b, float c);
	void ClampVelocity(float max_v);
	void Render(float x, float y, float w, float h);
	void Update(float dt);
	void MouseInput(); 
	void Reset();
	void ZeroEdge(float *p_in);
	void PushEdge(float *p_in);
	void CopyEdge(float *p_in, float *p_out);
	void CopyField(float *p_in, float *p_out);
	void AddFields(float *p_a, float *p_b, float *p_out);
	void SubFields(float *p_a, float *p_b, float *p_out);
	void AddField(float *p_a, float f, float *p_out);
	void MulField(float *p_a, float f, float *p_out);
	void MulFields(float *p_a, float *p_b, float *p_out);
	void SetField(float *p_a, float f);
	void ZeroField(float *p_field);
	void AddValue(float *p_in, float x, float y, float v);
	float GetValue(float *p_in, float x, float y);
	void SetValue(float *p_in, float x, float y, float v);
	void EdgeVelocities();
	void ForceFrom(float *p_from, float *p_to, float f);
	void ApplyForce(float *p_to, float f);
	void EdgeForce(float *p_to, float left, float right, float top, float bot);
	float Curl(int i, int j);
	void VorticityConfinement(float scale);
	void Collide(float x,float y, float &x1, float &y1);

	bool	m_field_test;

	// width and height
	int	m_w;
	int	m_h;
	float m_dt;

	float m_screen_x, m_screen_y, m_screen_w, m_screen_h;

	int Cell(int x, int y) {return x + m_w *y;}

	// Old and new x and y velocities
	float*	mp_xv0;
	float*	mp_xv1;
	float*	mp_xv2;
	float*	mp_yv0;
	float*	mp_yv1;
	float*	mp_yv2;
	// Old and new pressures
	float*		mp_p0;
	float*		mp_p1;
	// Old and new ink
	float*		mp_ink0;
	float*		mp_ink1;

	float*		mp_heat0;
	float*		mp_heat1;

	// temp accumulators of data for advection
	int*		mp_sources;
	float*		mp_source_fractions;
	float*		mp_fraction;

	// BFECC needs its own buffer
	// can't reuse other, since they might be used by the advection step
	float*		mp_BFECC;

	float*		mp_balance;

	int m_init;
	int m_last_x;
	int m_last_y;


	// "Physical" constants
		float m_velocity_diffusion;

	int	m_diffusion_iterations;

	// With no pressure diffusion, waves of pressure keep moving
	// But only if pressure is advected ahead of velocity
	float m_pressure_diffusion;

	float m_heat_diffusion;
	
	float m_ink_diffusion;

	// friction applied to velocity.  As a simple fraction each time step
	// so 1.0f will give you no friction

	// Friction is very important to the realism of the simulation
	// changes to these coeffficinets will GREATLY affect the apperence of the simulation.
	// the high factor friction (a) wich is applied to the square of the velocity,
	// acts as a smooth limit to velocity, which causes streams to break up into turbulent flow 
	// The c term will make things stop, which then shows up our border artefacts
	// but a high enough (>0.01) c term is needed to stop things just dissipating too fast
	// Friction is dt*(a*v^2 + b*v + c)
	float m_velocity_friction_a;
	float m_velocity_friction_b;
	float m_velocity_friction_c;
	// Pressure accelleration.  Larger values (>10) are more realistic (like water)
	// values that are too large lead to chaotic waves
	float m_vorticity;
	float m_pressure_acc;
	float m_ink_heat;
	float m_heat_force;
	float m_heat_friction_a;
	float m_heat_friction_b;
	float m_heat_friction_c;
// high ink advection allows fast moving nice swirlys
	float m_ink_advection;
	// seems nice if both velocity and pressure at same value, which makes sense
	float m_velocity_advection;
	float m_pressure_advection;
	float m_heat_advection;


};


// Two fluids
CFluid	g_fluid1;
CFluid	g_fluid2;




#ifdef		CRITICAL_SECTION_RND
CRITICAL_SECTION	rnd_CS;
#endif

////
// since rand is weak, I implement my own random numbers

unsigned int	rnd()				   
{

#ifdef		CRITICAL_SECTION_RND
	EnterCriticalSection(&rnd_CS);
#endif
#ifdef	FAST_RND
	static int rnd_a = 12345678;
	static int rnd_b = 12393455;

	rnd_a = rnd_a ^ 0x10010100;
	rnd_a = (rnd_a << 1) | ((rnd_a>> 31)&1);
	rnd_a ^= rnd_b;
	rnd_b = rnd_b * 255 + 32769;	
	unsigned int return_value = rnd_a;
#else

	static int rnd_a = 12345678;
	static int rnd_b = 12393455;
	static int rnd_c = 45432838;	   

	rnd_a = rnd_a ^ 0x10010100;
	rnd_a = (rnd_a << 1) | ((rnd_a>> 31)&1);
	rnd_a ^= rnd_b ^ rnd_c;
	rnd_b = rnd_b * 255 + 32769;
	rnd_c = rnd_a + rnd_b + rnd_c + 1;
	unsigned int return_value = rnd_a;
#endif
#ifdef		CRITICAL_SECTION_RND
	LeaveCriticalSection(&rnd_CS);
#endif
	return return_value;
}

// return random number in the range 0 .. a-1 
int rnd (unsigned int a)
{
	return rnd() % a;
}

// return random number in the range 0 .. f
float rndf(float f = 1.0f)
{
	return f*(float)rnd()/4294967296.0f;
}

float rndf(float f1, float f2)
{
	return f1 + (f2-f1)*(float)rnd()/4294967296.0f;
}

#define GRID_W		20
#define	GRID_H		20






//-----------------------------------------------------------------------------
// Function-prototypes for directinput handlers
//-----------------------------------------------------------------------------
BOOL CALLBACK    EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
HRESULT InitDirectInput( HWND hDlg );
VOID    FreeDirectInput();



LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; // Buffer to hold Vertices
LPDIRECT3DVERTEXBUFFER9 g_pVB2       = NULL; // Another Buffer to hold Vertices for lines
ID3DXFont*              g_pFont      = NULL;

RECT                    window_rect;
int                     g_window_width,g_window_height;

//inline float  scale_x(float x) {return x * (float)g_window_width / (float) g_viewport_width;}
//inline float  scale_y(float y) {return y * (float)g_window_height / (float) g_viewport_height;}
inline float  scale_x(float x) {return x;}
inline float  scale_y(float y) {return y;}

struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

static 	LARGE_INTEGER Freq;
static 	LARGE_INTEGER BaseTime;
static 	LARGE_INTEGER PausedTime;
static 	LARGE_INTEGER UnPausedTime;


void Timer_Reset()
{
	QueryPerformanceCounter(&BaseTime);
}

void Timer_Init()
{
	QueryPerformanceFrequency(&Freq);
	Timer_Reset();
}

float Timer_Seconds()
{
	static 	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	return (float)(Time.QuadPart-BaseTime.QuadPart)/(float)(Freq.QuadPart);
}

void Timer_Pause()
{
	QueryPerformanceCounter(&PausedTime);
}
void Timer_Resume()
{
	QueryPerformanceCounter(&UnPausedTime);
    // adjust the base time by the time we have been paused 
    BaseTime.QuadPart += (UnPausedTime.QuadPart - PausedTime.QuadPart); 
}

// Buffers for lines and triangles 
CUSTOMVERTEX *g_pTriVerts;
const int MAX_TRIS=5000000;
int	g_nTris = 0;

CUSTOMVERTEX *g_pLineVerts;
const int MAX_LINES=500000;
int	g_nLines = 0;

// SVG (Scalable Vector Graphics) file generation
// output things to a .SVG file, for easy input into Adobe Illustrator
/*
<?xml version="1.0" encoding="utf-8"?>
<!-- Generator: Adobe Illustrator 12.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 51448)  -->
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd" [
	<!ENTITY ns_svg "http://www.w3.org/2000/svg">
	<!ENTITY ns_xlink "http://www.w3.org/1999/xlink">
]>
<svg  version="1.1" id="Layer_1" xmlns="&ns_svg;" xmlns:xlink="&ns_xlink;" width="315.19" height="288" viewBox="0 0 315.19 288"
	 overflow="visible" enable-background="new 0 0 315.19 288" xml:space="preserve">
<line fill="none" stroke="#6C9D31" stroke-width="3" x1="72" y1="0" x2="72" y2="288"/>
<line fill="none" stroke="#6C9D31" stroke-width="3" x1="0" y1="72" x2="288" y2="72"/>
<g>
	<g>
		<line fill="none" stroke="#2E3192" stroke-width="3" x1="72" y1="72" x2="126" y2="27"/>
		<path fill="#2E3192" d="M129.475,20.102c-4.77,2.024-8.13,3.028-12.208,4.348l9.688,11.626c0.661-1.432,3.626-6.899,6.479-11.224
			c3.051-4.631,6.047-8.555,8.194-10.854C138.979,15.696,134.579,17.935,129.475,20.102z"/>
	</g>
</g>
<g>
	<g>
		<line fill="none" stroke="#2E3192" stroke-width="3" x1="135" y1="243" x2="297" y2="162"/>
		<path fill="#2E3192" d="M301.962,156.081c-5.106,0.878-8.606,1.085-12.88,1.437l6.769,13.536c0.971-1.242,5.11-5.885,8.877-9.442
			c4.031-3.809,7.846-6.942,10.463-8.688C312.224,153.969,307.427,155.141,301.962,156.081z"/>
	</g>
</g>
</svg>
*/


FILE *p_svg = NULL;

void SVG_Start(const char* p_name)
{
	p_svg = fopen(p_name,"wb");
	fprintf(p_svg,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(p_svg,"<!-- Generator: Adobe Illustrator 12.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 51448)  -->\n");
	fprintf(p_svg,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\" [\n");
	fprintf(p_svg,"	<!ENTITY ns_svg \"http://www.w3.org/2000/svg\">\n");
	fprintf(p_svg,"	<!ENTITY ns_xlink \"http://www.w3.org/1999/xlink\">\n");
	fprintf(p_svg,"]>\n");
	fprintf(p_svg,"<svg  version=\"1.1\" id=\"Layer_1\" xmlns=\"&ns_svg;\" xmlns:xlink=\"&ns_xlink;\" width=\"315.19\" height=\"288\" viewBox=\"0 0 315.19 288\"\n");
	fprintf(p_svg,"	 overflow=\"visible\" enable-background=\"new 0 0 315.19 288\" xml:space=\"preserve\">\n");
}

void SVG_End()
{
	fprintf(p_svg,"</svg>\n");
	fclose(p_svg);
}

void SVG_Line(unsigned int stroke, float width, float x1, float y1, float x2, float y2)
{
		fprintf(p_svg,"<line fill=\"none\" stroke=\"#%6X\" stroke-width=\"%f\" x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\"/>\n",
			stroke, width, x1,y1,x2,y2);
}

// head w and l are as fraction of the line length
void SVG_Arrow(unsigned int stroke, float width, float x1, float y1, float x2, float y2, float head_l, float head_w)
{
	SVG_Line(stroke,width,x1,y1,x2,y2);
	Vector2 end = Vector2(x2,y2);
	Vector2 line = Vector2(x2,y2) - Vector2(x1,y1);
	Vector2 normal = line.Normal();
	Vector2 side = Vector2(normal.y,-normal.x);
	float len = line.Length();
	Vector2 base = end - head_l * normal * len;
	Vector2 left = base + head_w * side * len; 
	Vector2 right = base - head_w * side * len; 
	SVG_Line(stroke,width,left.x,left.y,end.x,end.y);
	SVG_Line(stroke,width,right.x,right.y,end.x,end.y);

}


void SVG_Dot(unsigned int stroke, float cx, float cy, float r)
{
	fprintf(p_svg,"<circle fill=\"#%6X\" stroke=\"#%6X\" cx=\"%f\" cy=\"%f\" r=\"%f\"/>\n", stroke, stroke, cx,cy,r);
}

void SVG_Test()
{
	SVG_Start("C:\\Mick\\Docs\\GamesIndustry\\GameDeveloperArticles\\Fluid and Gas  Dynamics\\svg_test.svg");
	SVG_Line(0x604323,3,20,20,50,80);
	SVG_End();
}


//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    if ( NULL == g_pD3D)
    {
        // Create the D3D object.
        if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
            return E_FAIL;
    }

    // We might be re-creating the font and device, so release them if so.
    if ( g_pFont != NULL )
    {
      g_pFont->Release();     
      g_pFont = NULL;
    }
    if( g_pd3dDevice != NULL ) 
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }
    
    // Set up the structure used to create the D3DDevice
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
	
    d3dpp.BackBufferWidth = g_window_width;
	d3dpp.BackBufferHeight = g_window_height;
    
	d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //D3DPRESENT_INTERVAL_ONE;
    //d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }
    
               
    if( FAILED( D3DXCreateFont( g_pd3dDevice, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         (LPCSTR)"Arial", &g_pFont ) ) )
    {
        return E_FAIL;
    }


    return S_OK;
}



const int max_strings = 200;
const int max_string_length = 255;
struct SDrawText {
    float x,y;
    char text[max_string_length+1];
    DWORD color;
};

int num_draw_strings;

SDrawText   texts_to_draw[max_strings];
void    DrawString(float x, float y, const char *p_text, DWORD color = 0xff000000)
{
    if (num_draw_strings == max_strings)
        return;
    texts_to_draw[num_draw_strings].x = scale_x(x);    
    texts_to_draw[num_draw_strings].y = scale_y(y);    
    texts_to_draw[num_draw_strings].color = color;
    strncpy(texts_to_draw[num_draw_strings].text,p_text,max_string_length);    
    texts_to_draw[num_draw_strings].text[max_string_length]='\0'; // NULL terminator for iff p_text is >255 chars
    num_draw_strings++;
    
}


//-----------------------------------------------------------------------------
// Name: InitVB()
// Desc: Creates a vertex buffer and fills it with our Vertices. The vertex
//       buffer is basically just a chuck of memory that holds Vertices. After
//       creating it, we must Lock()/Unlock() it to fill it. For indices, D3D
//       also uses index buffers. The special thing about vertex and index
//       buffers is that they can be created in device memory, allowing some
//       cards to process them in hardware, resulting in a dramatic
//       performance gain.
//-----------------------------------------------------------------------------
HRESULT InitVB()
{

	g_pTriVerts = new CUSTOMVERTEX[MAX_TRIS * 3];
	g_nTris = 0;
	g_pLineVerts = new CUSTOMVERTEX[MAX_LINES * 2];
	g_nLines = 0;

    // Create the vertex buffer.  We also
    // specify the FVF, so the vertex buffer knows what data it contains.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( MAX_TRIS*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    {
        return E_FAIL;
    }
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( MAX_LINES*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB2, NULL ) ) )
    {
        return E_FAIL;
    }


    return S_OK;
}


HRESULT FillVB()
{
	// Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the Vertices. This mechanism is required becuase vertex
    // buffers may be in device memory.
    VOID* pVertices;
    if( FAILED( g_pVB->Lock( 0, g_nTris * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    memcpy( pVertices, g_pTriVerts, g_nTris * sizeof(CUSTOMVERTEX) );
    g_pVB->Unlock();

    // Repeat for lines    
    if( FAILED( g_pVB2->Lock( 0, g_nLines * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    memcpy( pVertices, g_pLineVerts, g_nLines * sizeof(CUSTOMVERTEX) );
    g_pVB->Unlock();
	
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pVB != NULL )        
        g_pVB->Release();
    if( g_pVB2 != NULL )        
        g_pVB2->Release();
        
    if ( g_pFont != NULL )
      g_pFont->Release();     

    if( g_pd3dDevice != NULL ) 
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )       
        g_pD3D->Release();

	if (g_pTriVerts != NULL)
		delete g_pTriVerts;
	if (g_pLineVerts != NULL)
		delete g_pLineVerts;
        
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer to a neutral color
    //g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(180,180,200), 1.0f, 0 );
    //g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255,255,255), 1.0f, 0 );

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {

		g_fluid1.Render(40,40,440,440*g_fluid1.m_h/g_fluid1.m_w);
		g_fluid2.Render(550,40,440,440*g_fluid2.m_h/g_fluid2.m_w);
        
        // Refill the VB, allowing us to draw whatever
		FillVB();
		
        //g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
        //g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        //g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nTris/3 );


		static int batch_size = 20000;  // number of triangles to send at once

		//batch_size = g_Caps.MaxPrimitiveCount/3;

		int tris_left = g_nTris/3;
		g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

		int first_prim = 0;

		while (tris_left > 0)
		{
			int chunk;
			if (tris_left <= batch_size)
				chunk = tris_left;
			else
				chunk = batch_size;

			// Render the triangle list vertex buffer contents
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, first_prim*3, chunk );
			first_prim += chunk;
			tris_left -= chunk;
		}


        // And the lines
        g_pd3dDevice->SetStreamSource( 0, g_pVB2, 0, sizeof(CUSTOMVERTEX) );
        g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, g_nLines/2 );

        RECT rc;
        for (int i=0;i<num_draw_strings;i++)
        {
            SetRect( &rc, (int)texts_to_draw[i].x, (int)texts_to_draw[i].y, 0, 0 );        
            g_pFont->DrawText(NULL, (LPCSTR)texts_to_draw[i].text, -1, &rc, DT_NOCLIP, texts_to_draw[i].color);
        }
        num_draw_strings = 0;

        // and reset
        g_nTris = 0;
        g_nLines = 0;

        // End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

void	InitFluids();

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
// Windows application initialization and message handling
// Based on DirectX SDK example applications



//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "Fluid", NULL };
    RegisterClassEx( &wc );

	InitializeCriticalSection(&debug_CS);

    // Create the application's window
	HWND hWnd = CreateWindow( "Fluid", "Mick West: Fluid Physics Example",
                              WS_OVERLAPPEDWINDOW, 0, 0, g_viewport_width, g_viewport_height,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );
                              
	// Get the size of the renderable rectangle
	GetClientRect(hWnd,&window_rect);
    g_window_width = window_rect.right-window_rect.left;
	g_window_height = window_rect.bottom-window_rect.top;

    debug_log("ClientRect g_window Window = %dx%d",g_window_width,g_window_height);
                              



    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        if( SUCCEEDED( InitVB() ) )
        {
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );



			InitFluids();

            // Enter the message loop
            MSG msg;
            ZeroMemory( &msg, sizeof(msg) );
            Timer_Init();
            while( msg.message!=WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                {
					if (has_focus)
					{
						static float fps = 0.0f;
						float   start = Timer_Seconds();
						static float last_time = 0.0f;
						float time_step = start-last_time;
						last_time = start;
						
						// clamp timestep to 1/50th of a second max
						if (time_step > 0.02)
							time_step = 0.02f;

						float fluid_start = Timer_Seconds();
						float fluid_start1 = Timer_Seconds();
						g_fluid1.Update(time_step*1.0f);
						float fluid_end1 = Timer_Seconds();

						float fluid_start2 = Timer_Seconds();
#if 1
						// THIS IS WHERE I SET THE DIFFERENCES FOR A/B COMPARISONS
						// g_fluid2 is the one on the right
						g_fluid2.m_pressure_diffusion = 10.0f; 
						g_fluid2.m_velocity_diffusion = 1.0f;
						g_fluid2.m_diffusion_iterations = 1;
#endif
						g_fluid2.Update(time_step*1.0f);
						float fluid_end2 = Timer_Seconds();

						g_fluid1.MouseInput();
						mouse_x += 510;
						g_fluid2.MouseInput();
						mouse_x -= 510;
						float fluid_end = Timer_Seconds();

						Render();

						char buf[100];
						sprintf(buf, "FPS = %4.2f, update1 = %.4f, update2 = %.4f",fps,fluid_end1-fluid_start1, fluid_end2-fluid_start2);
						DrawString(5,20,buf,0xff202020);
						sprintf(buf, "SPACE = Pause, A = Display Velocity, S = Reset, D = Display Gradient, W = Display Density");
						DrawString(300,20,buf,0xff202020);
						float end = Timer_Seconds();
						fps = 1.0f / (end - start);
					}
					else
					{
						Sleep(1);
					}
                        
                }
            }            
        }
    }

    UnregisterClass( "Fluid", wc.hInstance );
    return 0;
}

// end of Windows Stuff

// The following function uses the directX wrapper stuff directly

// triangle with clockwise ordered points
void DrawTri(float x0,float y0,float x1,float y1,float x2,float y2, DWORD color)
{
    if (g_nTris > MAX_TRIS-3)
    {
        // Error - run out of triangle buffer
    }

	g_pTriVerts[g_nTris+0].x = scale_x(x0);
	g_pTriVerts[g_nTris+0].y = scale_y(y0);
	g_pTriVerts[g_nTris+0].z = 0.5f;
	g_pTriVerts[g_nTris+0].rhw = 1.0f;
	g_pTriVerts[g_nTris+0].color = color;

	g_pTriVerts[g_nTris+1].x = scale_x(x1);
	g_pTriVerts[g_nTris+1].y = scale_y(y1);
	g_pTriVerts[g_nTris+1].z = 0.5f;
	g_pTriVerts[g_nTris+1].rhw = 1.0f;
	g_pTriVerts[g_nTris+1].color = color;

	g_pTriVerts[g_nTris+2].x = scale_x(x2);
	g_pTriVerts[g_nTris+2].y = scale_y(y2);
	g_pTriVerts[g_nTris+2].z = 0.5f;
	g_pTriVerts[g_nTris+2].rhw = 1.0f;
	g_pTriVerts[g_nTris+2].color = color;
    
    g_nTris+=3;    
    
}

void DrawTri(float x0,float y0,float x1,float y1,float x2,float y2, DWORD color0, DWORD color1, DWORD color2)
{
    if (g_nTris > MAX_TRIS-3)
    {
        // Error - run out of triangle buffer
    }

	g_pTriVerts[g_nTris+0].x = scale_x(x0);
	g_pTriVerts[g_nTris+0].y = scale_y(y0);
	g_pTriVerts[g_nTris+0].z = 0.5f;
	g_pTriVerts[g_nTris+0].rhw = 1.0f;
	g_pTriVerts[g_nTris+0].color = color0;

	g_pTriVerts[g_nTris+1].x = scale_x(x1);
	g_pTriVerts[g_nTris+1].y = scale_y(y1);
	g_pTriVerts[g_nTris+1].z = 0.5f;
	g_pTriVerts[g_nTris+1].rhw = 1.0f;
	g_pTriVerts[g_nTris+1].color = color1;

	g_pTriVerts[g_nTris+2].x = scale_x(x2);
	g_pTriVerts[g_nTris+2].y = scale_y(y2);
	g_pTriVerts[g_nTris+2].z = 0.5f;
	g_pTriVerts[g_nTris+2].rhw = 1.0f;
	g_pTriVerts[g_nTris+2].color = color2;
    
    g_nTris+=3;    
    
}


void DrawLine2(float x0,float y0,float x1,float y1, DWORD color0, DWORD color1)
{
    if (g_nLines > MAX_LINES-2)
    {
        // Error - run out of line buffer
    }
	g_pLineVerts[g_nLines+0].x = scale_x(x0);
	g_pLineVerts[g_nLines+0].y = scale_y(y0);
	g_pLineVerts[g_nLines+0].z = 0.5f;
	g_pLineVerts[g_nLines+0].rhw = 1.0f;
	g_pLineVerts[g_nLines+0].color = color0;
                                         
	g_pLineVerts[g_nLines+1].x = scale_x(x1);
	g_pLineVerts[g_nLines+1].y = scale_y(y1);
	g_pLineVerts[g_nLines+1].z = 0.5f;
	g_pLineVerts[g_nLines+1].rhw = 1.0f;
	g_pLineVerts[g_nLines+1].color = color1;
    
    g_nLines+=2;    
}


void DrawLine(float x0,float y0,float x1,float y1, DWORD color=0xff000000)
{
    DrawLine2(x0,y0,x1,y1,color,color);
}

void	DrawDiamond(float x, float y, int w, DWORD color = 0xff000000)
{
	DrawTri(x-w,y,x,y-w,x+w,y,color);
	DrawTri(x+w,y,x,y+w,x-w,y,color);
	
}

void DrawDiamond(Vector2 pos, int w, DWORD color=0xff000000)
{
	DrawDiamond(pos.x,pos.y,w,color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Basic 2d Primitive rendering code - Triangles and Lines


void DrawLine(Vector2 start , Vector2 end, DWORD color)
{
    DrawLine(start.x, start.y, end.x, end.y,color );
}


void DrawX(Vector2 pos, float size, DWORD color)
{
    DrawLine(pos.x-size, pos.y-size, pos.x+size, pos.y+size,color);
    DrawLine(pos.x-size, pos.y+size, pos.x+size, pos.y-size,color);
}

// Draw a quad with four points in clockwise order
void DrawQuad(float x0,float y0,float x1,float y1,float x2,float y2, float x3, float y3, DWORD color)
{
	DrawTri(x0,y0,x1,y1,x2,y2, color);
	DrawTri(x2,y2,x3,y3,x0,y0, color);
}

// Draw a quad with four points in clockwise order per-vert color
void DrawQuad(float x0,float y0,float x1,float y1,float x2,float y2, float x3, float y3, DWORD color0, DWORD color1, DWORD color2, DWORD color3)
{
	DrawTri(x0,y0,x1,y1,x2,y2, color0, color1, color2);
	DrawTri(x2,y2,x3,y3,x0,y0, color2, color3, color0);
}


// Draw an axis aligned rectangle
void DrawRect(float x, float y, float w, float h, DWORD color)
{
	DrawQuad(x,y,x+w,y,x+w,y+h,x,y+h,color);
}

// Draw an axis aligned rectangle
void DrawRect(float x, float y, float w, float h, DWORD color0, DWORD color1, DWORD color2, DWORD color3)
{
	DrawQuad(x,y,x+w,y,x+w,y+h,x,y+h,color0, color1, color2, color3);
}



CFluid::CFluid()
{

}

CFluid::~CFluid()
{
	if (m_w > 0)
	{
		delete mp_xv0;
		delete mp_yv0;
		delete mp_xv1;
		delete mp_yv2;
		delete mp_xv2;
		delete mp_yv1;
		delete mp_p0;
		delete mp_p1;
		delete mp_ink0;
		delete mp_ink1;
		delete mp_heat0;
		delete mp_heat1;
		delete mp_sources;
		delete mp_source_fractions;
		delete mp_fraction;
		delete mp_BFECC;
		delete mp_balance;
	}
}

void	CFluid::Init(int w, int h)
{
	m_w = w;
	m_h = h;
	int size = m_w * m_h;
	mp_xv0 = new float[size];
	mp_yv0 = new float[size];
	mp_xv1 = new float[size];
	mp_yv1 = new float[size];
	mp_xv2 = new float[size];
	mp_yv2 = new float[size];
	mp_p0  = new float[size];
	mp_p1  = new float[size];
	mp_ink0  = new float[size];
	mp_ink1  = new float[size];
	mp_heat0  = new float[size];
	mp_heat1  = new float[size];
	mp_sources = new int[size];
	mp_source_fractions = new float[size*4];
	mp_fraction = new float[size];
	mp_BFECC = new float[size];
	mp_balance = new float[size];
	Reset();


	// add a blob for initial tests

	for (int x = 10; x<20;x++)
	{
		for (int y=10;y<20;y++)
		{
		//	if (x == 15 && y == 15)
		//		mp_p0[Cell(x,y)] = 1;
		//	else
		//		mp_p0[Cell(x,y)] = 1/sqrtf((15-x)*(15-x) + (15-y)*(15-y));
		}
	//	mp_p0[Cell(30,x)] = 1;
	}
}

void CFluid::Reset()
{
	int size = m_w * m_h;
	for (int i = 0; i<size; i++)
	{
		mp_xv0[i] = 0.0f;
		mp_yv0[i] = 0.0f;
		mp_xv1[i] = 0.0f;
		mp_yv1[i] = 0.0f;
		mp_xv2[i] = 0.0f;
		mp_yv2[i] = 0.0f;
		mp_p0[i]  = 1.0f; // should be 1.0f, lower is more like gas, higher less compressible
		mp_p1[i]  = 0.0f;
		mp_heat0[i]  = 0.00f;
		mp_heat1[i]  = 0.00f;
		if (i>m_w*(m_h/2))
			mp_ink0[i]  = 0.00f;  // even spread of initial ink
		else
			mp_ink0[i]  = 0.00f;  // even spread of initial ink
	}
}

/*
	Cells adjacent to border cells have a different diffusion.  They still take in the four surrounding cells
	but since the border cells are not updated, the adjacent cells should emit less.
	We could use a lookup table for this
	or just calculate an adjaceny figure 

	// ACTUALLY - NOW WE DO UPDATE ALL THE CELLS, SO BORDER CONDITION NOW CHANGED

*/

void CFluid::Diffusion(float *p_in, float *p_out, float scale)
{
	// Build the new pressure values in p1, then swap p0 and p1


	float a = m_dt * scale ;
	int cell;
#if 0
	// Copy edge as not diffusing into edge
	CopyEdge(p_in,p_out);
#else
// top and bot edges
	for (int x = 1; x < m_w-1; x++)
	{
		cell = Cell(x,0);
		p_out[cell] = p_in[cell] + a * (p_in[cell-1] + p_in[cell+1] + p_in[cell+m_h] - 3.0f * p_in[cell]);
		cell = Cell(x,m_h-1);
		p_out[cell] = p_in[cell] + a * (p_in[cell-1] + p_in[cell+1] + p_in[cell-m_h] - 3.0f * p_in[cell]);
	}
// left and right edges
	for (int y = 1; y < m_h-1; y++)
	{
		cell = Cell(0,y);
		p_out[cell] = p_in[cell] + a * (p_in[cell-m_w] + p_in[cell+m_w] + p_in[cell+1] - 3.0f * p_in[cell]);
		cell = Cell(m_w-1,y);
		p_out[cell] = p_in[cell] + a * (p_in[cell-m_w] + p_in[cell+m_w] + p_in[cell-1] - 3.0f * p_in[cell]);
	}
// corners
		cell = Cell(0,0);
		p_out[cell] = p_in[cell] + a * (p_in[cell+1] + p_in[cell+m_w] - 2.0f * p_in[cell]);
		cell = Cell(m_w-1,0);
		p_out[cell] = p_in[cell] + a * (p_in[cell-1] + p_in[cell+m_w] - 2.0f * p_in[cell]);
		cell = Cell(0,m_h-1);
		p_out[cell] = p_in[cell] + a * (p_in[cell+1] + p_in[cell-m_w] - 2.0f * p_in[cell]);
		cell = Cell(m_w-1,m_h-1);
		p_out[cell] = p_in[cell] + a * (p_in[cell-1] + p_in[cell-m_w] - 2.0f * p_in[cell]);
#endif
	for (int x = 1; x < m_w-1; x++)
	{
		for (int y = 1; y < m_h-1; y++)
		{
			cell = Cell(x,y);
			p_out[cell] = p_in[cell] + a * (p_in[Cell(x,y+1)] + p_in[Cell(x,y-1)] + p_in[Cell(x+1,y)] + p_in[Cell(x-1,y)] - 4.0f * p_in[cell]);

		}
	}

}

void CFluid::DiffusionLR(float *p_in, float *p_out, float scale)
{
	// Build the new pressure values in p1, then swap p0 and p1


	float a = m_dt * scale ;

	for (int y = 1; y < m_h; y++)
	{
		for (int x = 0; x < m_w-1; x++)
		{
			int cell = Cell(x,y);
			float m = p_in[cell];
			float n = p_in[cell+1];
			p_out[cell]   = m + a*(n-m);
			p_out[cell+1] = n + a*(m-n);
			//p_out[cell] = p_in[cell] + a * (p_in[Cell(x,y+1)] + p_in[Cell(x,y-1)] + p_in[Cell(x+1,y)] + p_in[Cell(x-1,y)] - adj * p_in[cell]);
		}
	}
}

void CFluid::GlobalDiffusion(float *p_in, float *p_out, float scale)
{
	float a = scale * m_dt;
	int size = m_w*m_h;
	float tot = 0;
	for (int cell=0;cell<size;cell++)
	{
		tot += p_in[cell];
	}

	float avg = tot/size;

	for (int cell=0;cell<size;cell++)
	{
		p_out[cell] = (1.0f-a) * p_in[cell] + a*avg;
	}
}


void CFluid::DiffusionUD(float *p_in, float *p_out, float scale)
{
}


void CFluid::CopyEdge(float *p_in, float *p_out)
{
	for (int x = 0; x<m_w;x++)
	{
		p_out[x] = p_in[x];
		p_out[m_w * (m_h-1) + x] = p_in[m_w * (m_h-1) + x];
	}

	for (int y = 1; y<m_h-1;y++)
	{
		p_out[y * m_w] = p_in[y * m_w];
		p_out[y * m_w + (m_w-1)] = p_in[y * m_w + (m_w-1)];
	}
}

void CFluid::CopyField(float *p_in, float *p_out)
{
	int size = m_w * m_h;

#if 0
	for (int x = 0; x<size;x++)
	{
		p_out[x] = p_in[x];
	}
#else
	memcpy(p_out,p_in,size*sizeof(float));
#endif

}

void CFluid::AddFields(float *p_a, float *p_b, float *p_out)
{
	int size = m_w * m_h;
	for (int x = 0; x<size;x++)
	{
		p_out[x] = p_a[x] + p_b[x];
	}
}

void CFluid::SubFields(float *p_a, float *p_b, float *p_out)
{
	int size = m_w * m_h;
	for (int x = 0; x<size;x++)
	{
		p_out[x] = p_a[x] - p_b[x];
	}
}

void CFluid::MulFields(float *p_a, float *p_b, float *p_out)
{
	int size = m_w * m_h;
	for (int x = 0; x<size;x++)
	{
		p_out[x] = p_a[x] * p_b[x];
	}
}

void CFluid::MulField(float *p_a, float f, float *p_out)
{
	int size = m_w * m_h;
	for (int x = 0; x<size;x++)
	{
		p_out[x] = p_a[x] * f;
	}

}

void CFluid::AddField(float *p_a, float f, float *p_out)
{
	int size = m_w * m_h;
	for (int x = 0; x<size;x++)
	{
		p_out[x] = p_a[x] + f;
	}

}




void CFluid::ZeroField(float *p_field)
{
	int size = m_w*m_h;
	memset((void*)p_field,0,size*sizeof(float));
}

void CFluid::SetField(float *p_field, float f)
{
	int size = m_w * m_h;
	for (int x = 0; x<size;x++)
	{
		p_field[x] = f;
	}
}


void CFluid::ZeroEdge(float *p_in)
{
	for (int x = 0; x<m_w;x++)
	{
		p_in[x] = 0;
		p_in[m_w * (m_h-1) + x] = 0;
	}

	for (int y = 1; y<m_h-1;y++)
	{
		p_in[y * m_w] = 0;
		p_in[y * m_w + (m_w-1)] = 0;
	}
}

void CFluid::PushEdge(float *p_in)
{
	for (int x = 0; x<m_w;x++)
	{
		p_in[x+m_w]             += p_in[x];
		p_in[m_w * (m_h-2) + x] += p_in[m_w * (m_h-1) + x];
	}

	for (int y = 1; y<m_h-1;y++)
	{
		p_in[y * m_w+1] += p_in[y * m_w];
		p_in[y * m_w + (m_w-1)-1] += p_in[y * m_w + (m_w-1)];
	}
}

void CFluid::EdgeForce(float *p_in, float left, float right, float top, float bot)
{
	for (int x = 0; x<m_w;x++)
	{
		p_in[x]                 += top;
		p_in[m_w * (m_h-1) + x] += bot;
	}

	for (int y = 0; y<m_h;y++)
	{
		p_in[y * m_w] += left;
		p_in[y * m_w + (m_w-1)]+=right;
	}

}

// any velocities that are facing outwards at the edge cells, face them inwards
void CFluid::EdgeVelocities()
{
	for (int y = 0; y<m_h;y++)
	{
		int left_cell = Cell(0,y); 
		if (mp_xv0[left_cell] < 0.0f)
		{
			mp_xv0[left_cell] = -mp_xv0[left_cell];
		}
		int right_cell = Cell(m_w-1,y); 
		if (mp_xv0[right_cell] > 0.0f)
		{
			mp_xv0[right_cell] = -mp_xv0[right_cell];
		}
	}

	for (int x = 0; x<m_w;x++)
	{
		int top_cell = Cell(x,0); 
		if (mp_yv0[top_cell] < 0.0f)
		{
			mp_yv0[top_cell] = -mp_yv0[top_cell];
		}
		int bot_cell = Cell(x,m_h-1); 
		if (mp_yv0[bot_cell] > 0.0f)
		{
			mp_yv0[bot_cell] = -mp_yv0[bot_cell];
		}
	}

}

inline void CFluid::Collide(float x,float y, float &x1, float &y1)
{
	float left_bound = m_w-1.0001f;
	float bot_bound = m_h-1.0001f;
#if 0
	// simple clamping
	if (x1<0) x1 = 0;
	else if (x1>left_bound) x1=left_bound;  // allow to go just into the far edge of last but one cell
	if (y1<0) y1 = 0;
	else if (y1>bot_bound) y1=bot_bound;
#endif
#ifdef FLUID_WRAPPING
	// Simple wrapping - need to also wrap pressure accelleration if doing this
	if (x1<0) x1 = left_bound + x1;
	if (x1>left_bound) x1 -= left_bound; 
	if (y1<0) y1 += bot_bound;
	if (y1>bot_bound) y1 -= bot_bound;
#endif
#ifndef FLUID_WRAPPING
	// proper reflection
	if (x1<0) x1 = -x1;
	else if (x1>left_bound) x1=left_bound - (x1 - left_bound);
	if (y1<0) y1 = -y1;
	else if (y1>bot_bound) y1=bot_bound - (y1 - bot_bound);
#endif
#if 0
	// collision with a parametric circle
	// This does not work as is, since the circle edges are not on cell boundries
	// so edge cells are both inside and outside, and so they fill up and bleed into the center cells
	// we also don't have a mechanism for not updating cells inside the circle
	// we need to either define collisions based purely on the rectangular cells
	// or work on an arbitary triangle grid, which might be interesting.
	Vector2 center = Vector2(50.0f, 50.0f);
	Vector2 end = Vector2(x1, y1)-center;
	Vector2 start = Vector2(x,y)-center;

	float end_len2 = end.Length2();
	float start_len2 = start.Length2();
	float radius = 20.0f;
	if (end_len2 < radius*radius && start_len2>=radius*radius)
	{

		// end is inside the circle
		// Get start point relative to the center
		Vector2 p1, p2;
		if (LineCircleIntersect(start, end, radius, p1, p2))
		{
			// p2 is the collision point, still in relative coordinates
			// so we need to reflect start->end about 0->p2
			// we already know that p2 is on the circle, so we have its length alreadys
			Vector2 p2_to_end = end-p2;
			Vector2 reflection_axis = p2/radius;
			float p2_to_end_outwards = DotProduct(p2_to_end,reflection_axis);
			// Adjust the end point
			end = end - reflection_axis * p2_to_end_outwards * 2.0f; 
			// And finally return the absolute coordinates
			end = end + center;
			x1 = end.x;
			y1 = end.y;
			//x1 = x;
			//y1 = y;
		}
	}
#endif
}


#if 0
// OLD - cells on either side of a cell affect the middle cell.
// The force of pressure affects velocity
// THIS DOES NOT GIVE VELOCITY TO THE EDGES, LEADING TO STUCK CELLS
// (stuck cells only if velocity is advected before pressure)
void CFluid::PressureAcceleration(float force)
{

	// Pressure differential between points  
	// to get an accelleration force.
	float a = m_dt * force ;  

	// must copy the velocity field edges are they are not updated
	CopyEdge(mp_xv0,mp_xv1);
	CopyEdge(mp_yv0,mp_yv1);

	for (int iterate = 0; iterate<1;iterate++)
	{
		for (int x = 1; x < m_w-1; x++)
		{
			for (int y = 1; y < m_h-1; y++)
			{
				int cell = Cell(x,y);

			//	float	adj = 4.0f;
			//	if (x == 1 || x == m_w-2) adj -= 1.0f;
			//	if (y == 1 || y == m_h-2) adj -= 1.0f;

				// the pressure of this cell cancels out l/r u/d 
				// so it's just the pressure diffential of points on either side
				float force_x = -mp_p0[cell-1] + mp_p0[cell+1];  
				float force_y = -mp_p0[cell-m_w] + mp_p0[cell+m_w];  
		
				mp_xv1[cell] = mp_xv0[cell] - a * force_x;
				mp_yv1[cell] = mp_yv0[cell] - a * force_y;

				// A		B		C		D
				// 		   C-A     D-B 	

			}
		}
		float *t = mp_xv1;
		mp_xv1 = mp_xv0;
		mp_xv0 = t;
		t = mp_yv1;
		mp_yv1 = mp_yv0;
		mp_yv0 = t;
	}

}
#else
// NEW - treat cells pairwise, which allows us to handle edge cells
// updates ALL cells
void CFluid::PressureAcceleration(float force)
{

	// Pressure differential between points  
	// to get an accelleration force.
	float a = m_dt * force ; 

	// First copy the values
	for (int x = 0; x < m_w; x++)
	{
		for (int y = 0; y < m_h; y++)
		{
			int cell = Cell(x,y);
			mp_xv1[cell] = mp_xv0[cell];
			mp_yv1[cell] = mp_yv0[cell];
		}
	}


//	for (int iterate = 0; iterate<1;iterate++)
	{
#ifndef FLUID_WRAPPING
		for (int x = 0; x < m_w-1; x++)
		{
			for (int y = 0; y < m_h-1; y++)
			{
				int cell = Cell(x,y);

				float force_x =  mp_p0[cell]   - mp_p0[cell+1];  
				float force_y =  mp_p0[cell]   - mp_p0[cell+m_w];  
#if 0

				
				float pressure_c = mp_p0[cell];
				float pressure_scale_c = pressure_c*pressure_c*pressure_c - 0.125f;

				float pressure_l = mp_p0[cell+1];
				float pressure_scale_l = pressure_l*pressure_l*pressure_l - 0.125f;
		
				float pressure_d = mp_p0[cell+m_w];
				float pressure_scale_d = pressure_d*pressure_d*pressure_d - 0.125f;

				// forces act in same direction  on both cells
				mp_xv1[cell]     +=  a * force_x  * pressure_scale_c;
				mp_xv1[cell+1]   +=  a * force_x  * pressure_scale_l;  // B-A
				
				mp_yv1[cell]     +=  a * force_y  * pressure_scale_c;
				mp_yv1[cell+m_w] +=  a * force_y  * pressure_scale_d;
#else
				// forces act in same direction  on both cells
				if (1 || this == &g_fluid1)
				{
					mp_xv1[cell]     +=  a * force_x;
					mp_xv1[cell+1]   +=  a * force_x;
					
					mp_yv1[cell]     +=  a * force_y;
					mp_yv1[cell+m_w] +=  a * force_y;
				}
				else
				{
					if (force_x>0.0f)
					{
					mp_xv1[cell]     +=  a * force_x * force_x;
					mp_xv1[cell+1]   +=  a * force_x * force_x;
					}
					else
					{
					mp_xv1[cell]     -=  a * force_x * force_x;
					mp_xv1[cell+1]   -=  a * force_x * force_x;
					}

					if (force_y>0)
					{
					mp_yv1[cell]     +=  a * force_y * force_y;
					mp_yv1[cell+m_w] +=  a * force_y * force_y;
					}
					else
					{
					mp_yv1[cell]     -=  a * force_y * force_y;
					mp_yv1[cell+m_w] -=  a * force_y * force_y;
					}
				}
#endif
			}
		}
#else
		for (int x = 0; x < m_w; x++)
		{
			for (int y = 0; y < m_h; y++)
			{
				int cell = Cell(x,y);

				int nextx = cell+1;
				if (x==m_w-1) nextx = Cell(0,y);

				int nexty = cell+m_w;
				if (y==m_h-1) nexty = Cell(x,0);


				float force_x =  mp_p0[cell]   - mp_p0[nextx];  
				float force_y =  mp_p0[cell]   - mp_p0[nexty];  
		
				// forces act in same direction  on both cells
				mp_xv1[cell]     +=  a * force_x;
				mp_xv1[nextx]    +=  a * force_x;  // B-A
				
				mp_yv1[cell]     +=  a * force_y;
				mp_yv1[nexty]    +=  a * force_y;
			}
		}

#endif

		float *t = mp_xv1;
		mp_xv1 = mp_xv0;
		mp_xv0 = t;

		t = mp_yv1;
		mp_yv1 = mp_yv0;
		mp_yv0 = t;
	}

}
#endif

// The force of pressure affects velocity
void CFluid::VelocityFriction(float a, float b, float c )
{

	for (int iterate = 0; iterate<1;iterate++)
	{
		// NOTE: We include the border cells in global friction
		for (int x = 0; x < m_w; x++)
		{
			for (int y = 0; y < m_h; y++)
			{
				int cell = Cell(x,y);

				Vector2  v = Vector2(mp_xv0[cell],mp_yv0[cell]);
				float len2 = v.Length2();
				float len = sqrtf(len2);
				float sign = 1.0f;
				if (len <0.0f)
				{
					len = -len;
					sign = -1.0f;
				}
#if 1
				len -= m_dt*(a*len2 + b*len +c);
#else
				// Scale by pressure
				if (this == &g_fluid1)
				len -= m_dt*(a*len2 + b*len +c);
				else
				len -= m_dt*(a*len2 + b*len +c)*mp_p0[cell];
#endif
				if (len<0.0f)
					len = 0.0f;

				if (len < 0.0f) len = 0.0f;
				v = v.Normal()*len;
				mp_xv0[cell] = v.x;
				mp_yv0[cell] = v.y;
			}
		}
	}

}

// Decay a positive value towards zero via a quadratic function
void CFluid::QuadraticDecay(float *p_in, float *p_out, float a, float b, float c)
{
	float dt = m_dt;
	int size = m_w*m_h;
	for (int cell=0;cell<size;cell++)
	{
		float v = p_in[cell];
		float v2 = v*v;
		v-= dt*(a*v2 + b*v + c);
		if (v <0.0f)
			v= 0.0f;
		p_in[cell] = v;
	}
}


// Clamp velocity to a maximum magnitude
void CFluid::ClampVelocity(float max_v )
{
	float max_v2 = max_v * max_v;
	int size = m_w * m_h;
	for (int cell = 0; cell < size; cell++)
	{

		Vector2  v = Vector2(mp_xv0[cell],mp_yv0[cell]);
		float v2 = v.Length2();
		if (v2 > max_v2)
		{
			v = v.Normal() * max_v;
			mp_xv0[cell] = v.x;
			mp_yv0[cell] = v.y;
		}
	}
}


// Given a field p_from, and a field p_to, then add f*p_from to p_to
// can be used to apply a heat field to velocity
void CFluid::ForceFrom(float *p_from, float *p_to, float f)
{
	f *= m_dt;
	int size = m_w * m_h;
	for (int cell = 0; cell < size; cell++)
	{
		*p_to++ += *p_from++*f;
	}
}

void CFluid::ApplyForce(float *p_to, float f)
{
	f *= m_dt;
	int size = m_w * m_h;
	for (int cell = 0; cell < size; cell++)
	{
		*p_to++ += f;
	}
}




// Move a scalar along the velocity field

void CFluid::ReverseAdvection(float *p_in, float*p_out, float scale)
{
	// negate advection scale, since it's reverse advection
	float a = - m_dt * scale ;

	int size = m_w*m_h;

	// First copy the scalar values over, since we are adding/subtracting in values, not moving things
	CopyField(p_in,p_out);

	// we need to zero out the fractions 
	ZeroField(mp_fraction);

	// We do all the cells,
	for (int x = 0; x < m_w; x++)
	{
		for (int y = 0; y < m_h; y++)
		{
			int cell = Cell(x,y);
			float vx = mp_xv0[cell];
			float vy = mp_yv0[cell];
			if (vx != 0.0f || vy != 0.0f)
			{
				float x1 = x + vx * a;
				float y1 = y + vy * a;
				Collide(x,y,x1,y1);
				int cell1 = Cell((int)x1,(int)y1);
				
				// get fractional parts
				float fx = x1-(int)x1;
				float fy = y1-(int)y1;

				// we are taking pressure from four cells (one of which might be the target cell
				// and adding it to the target cell
				// hence we need to subtract the appropiate amounts from each cell
				//
				// Cells are like
				// 
				//    A----B
				//    |    |
				//    |    |
				//    C----D
				// 
				// (Should be square)
				// so we work out the bilinear fraction at each of a,b,c,d
				// NOTE: Using p1 here, to ensure consistency
				

/*

By adding the source value into the destination, we handle the problem of multiple destinations
but by subtracting it from the source we gloss ove the problem of multiple sources.
Suppose multiple destinations have the same (partial) source cells, then what happens is the first dest that 
is processed will get all of that source cell (or all of the fraction it needs).  Subsequent dest
cells will get a reduced fraction.  In extreme cases this will lead to holes forming based on 
the update order.

Solution:  Maintain an array for dest cells, and source cells.
For dest cells, store the four source cells and the four fractions
For source cells, store the number of dest cells that source from here, and the total fraction
E.G.  Dest cells A, B, C all source from cell D (and explicit others XYZ, which we don't need to store)
So, dest cells store A->D(0.1)XYZ..., B->D(0.5)XYZ.... C->D(0.7)XYZ...
Source Cell D is updated with A, B then C
Update A:   Dests = 1, Tot = 0.1
Update B:   Dests = 2, Tot = 0.6
Update C:   Dests = 3, Tot = 1.3

How much should go to each of A, B and C? They are asking for a total of 1.3, so should they get it all, or 
should they just get 0.4333 in total?
Ad Hoc answer:  
if total <=1 then they get what they ask for
if total >1 then is is divided between them proportionally.
If there were two at 1.0, they would get 0.5 each
If there were two at 0.5, they would get 0.5 each
If there were two at 0.1, they would get 0.1 each
If there were one at 0.6 and one at 0.8, they would get 0.6/1.4 and 0.8/1.4  (0.429 and 0.571) each

So in our example, total is 1.3, 
A gets 0.1/1.3, B gets 0.6/1.3 C gets 0.7/1.3, all totalling 1.0

SO we need additional arrays
int mp_sources[size]
int mp_source_fractions[size*4]
float mp_fraction[size]

*/
				// Just calculaitng fractions for now
				float ia = (1.0f-fy)*(1.0f-fx);
				float ib = (1.0f-fy)*(fx)     ;
				float ic = (fy)     *(1.0f-fx);
				float id = (fy)     *(fx)     ;
				// Storing the source information for this dest cell (cell)
				mp_sources[cell] = cell1;  // that's cell1, implying cell1+1, cell1+m_w and cell1+1+m_w
				// and the fractions, unless they are all zero
				// which is quite possible if advecting something sparse
				// but since we optimize for a more likely case, this test wastes time
				//if (ia !=0.0f || ib!=0.0f || ic !=0.0f || id != 0.0f)
				{
					mp_source_fractions[cell*4+0] = ia;
					mp_source_fractions[cell*4+1] = ib;
					mp_source_fractions[cell*4+2] = ic;
					mp_source_fractions[cell*4+3] = id;
					
					// Accumullting the fractions for the four sources 
					mp_fraction[cell1]		+= ia;
					mp_fraction[cell1+1]	+= ib;
					mp_fraction[cell1+m_w]	+= ic;
					mp_fraction[cell1+m_w+1]+= id;
				}
			}
			else
			{
				// cell has zero velocity, advecting from itself, flag that, and opimize it out later
				mp_sources[cell] = -1;  // 
			}
		}
	}

// With nice advection, each cell has four sources, and four source fractions

	for (int cell=0;cell<size;cell++)
	{
		// get the block of four source cells (cell1)
		int cell1 = mp_sources[cell];
		if (cell1 != -1)
		{
			// Get the four fractional amounts we earlier interpolated
			float ia = mp_source_fractions[cell*4+0];  // Using "cell", not "cell1", as it's an array of four values
			float ib = mp_source_fractions[cell*4+1];  // and not the grid of four
			float ic = mp_source_fractions[cell*4+2];
			float id = mp_source_fractions[cell*4+3];
			// get the TOTAL fraction requested from each source cell
			float fa = mp_fraction[cell1];
			float fb = mp_fraction[cell1+1];
			float fc = mp_fraction[cell1+m_w];
			float fd = mp_fraction[cell1+1+m_w];
			// if less then 1.0 in total, we can have all we want
#if 1
			if (fa<1.0f) fa = 1.0f;
			if (fb<1.0f) fb = 1.0f;
			if (fc<1.0f) fc = 1.0f;
			if (fd<1.0f) fd = 1.0f;
#else
			// DON'T DO THIS!!!
			// THis is a patch, since is fa is zero then ia will be zero
			// The idea is to ensure ALL of the cells a,b,c,d go somewhere.  
			// But it leads to artifacts.
			if (fa==0.0f) fa = 1.0f;
			if (fb==0.0f) fb = 1.0f;
			if (fc==0.0f) fc = 1.0f;
			if (fd==0.0f) fd = 1.0f;
#endif
			// scale the amount we are transferring
			ia /= fa;
			ib /= fb;
			ic /= fc;
			id /= fd;
			// Give the fraction of the original source, do not alter the original
			// So we are taking fractions from p_in, but not altering those values
			// as they are used again by later cells
			// if the field were mass conserving, then we could simply move the value
			// but if we try that we lose mass
			p_out[cell] += ia * p_in[cell1] + ib * p_in[cell1+1] + ic * p_in[cell1+m_w] + id * p_in[cell1+1+m_w];
			// The accounting happens here, the values we added to the dest in p1 are subtracted from the soruces in p1
			p_out[cell1]      -= ia * p_in[cell1];
			p_out[cell1+1]    -= ib * p_in[cell1+1];
			p_out[cell1+m_w]  -= ic * p_in[cell1+m_w];
			p_out[cell1+m_w+1]-= id * p_in[cell1+m_w+1];

		}
	}
}


// Signed advection is mass conserving, but allows signed quantities 
// so could be used for velocity, since it's faster.
void CFluid::ReverseSignedAdvection(float *p_in, float*p_out, float scale)
{
	// negate advection scale, since it's reverse advection
	float a = - m_dt * scale ;

	int size = m_w*m_h;

	// First copy the scalar values over, since we are adding/subtracting in values, not moving things
	CopyField(p_in,p_out);

	// We do all the cells,
	for (int x = 0; x < m_w; x++)
	{
		for (int y = 0; y < m_h; y++)
		{
			int cell = Cell(x,y);
			float vx = mp_xv0[cell];
			float vy = mp_yv0[cell];
			if (vx != 0.0f || vy != 0.0f)
			{
				float x1 = x + vx * a;
				float y1 = y + vy * a;
				Collide(x,y,x1,y1);
				int cell1 = Cell((int)x1,(int)y1);
				
				// get fractional parts
				float fx = x1-(int)x1;
				float fy = y1-(int)y1;

				// Get amounts from (in) source cells
				float ia = (1.0f-fy)*(1.0f-fx) * p_in[cell1];
				float ib = (1.0f-fy)*(fx)      * p_in[cell1+1];
				float ic = (fy)     *(1.0f-fx) * p_in[cell1+m_w];
				float id = (fy)     *(fx)      * p_in[cell1+m_w+1];

				// add to (out) dest cell
				p_out[cell] += ia + ib + ic + id ;
				// and subtract from (out) dest cells
				p_out[cell1]      -= ia;
				p_out[cell1+1]    -= ib;
				p_out[cell1+m_w]  -= ic;
				p_out[cell1+m_w+1]-= id;

			}
		}
	}
}


// Move scalar along the velocity field
// Forward advection moves the value at a point forward along the vector from the same point
// and dissipates it between four points as needed
void CFluid::ForwardAdvection(float *p_in, float*p_out, float scale)
{
		// Pressure differential between points  
	// to get an accelleration force.
	float a = m_dt * scale ;

	int w=m_w;
	int h=m_h;

	// First copy the values
	CopyField(p_in,p_out);

	if (scale == 0.0f)
		return;


	// We advect all cells
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			int cell = Cell(x,y);
			float vx = mp_xv0[cell];
			float vy = mp_yv0[cell];
			if (vx != 0.0f || vy != 0.0f)
			{
				float x1 = x + vx * a;
				float y1 = y + vy * a;
				Collide(x,y,x1,y1);
				int cell1 = Cell((int)x1,(int)y1);
				// get fractional parts
				float fx = x1-(int)x1;
				float fy = y1-(int)y1;

				// we are taking pressure from four cells (one of which might be the target cell
				// and adding it to the target cell
				// hence we need to subtract the appropiate amounts from each cell
				//
				// Cells are like
				// 
				//    A----B
				//    |    |
				//    |    |
				//    C----D
				// 
				// (Should be square)
				// so we work out the bilinear fraction at each of a,b,c,d
				
				float in = p_in[cell];

				float ia = (1.0f-fy)*(1.0f-fx) * in;
				float ib = (1.0f-fy)*(fx)      * in;
				float ic = (fy)     *(1.0f-fx) * in;
				float id = (fy)     *(fx)      * in;

				// Subtract them from the source cell
				p_out[cell] -= (ia+ib+ic+id);
				// Then add them to the four destination cells
				p_out[cell1]		+= ia;
				p_out[cell1+1]		+= ib;
				p_out[cell1+w]		+= ic;
				p_out[cell1+w+1]	+= id;
			}
		}
	}
}

// Move part of a scalar along the velocity field
// Intended to spread pressure along velocity lines
void CFluid::PartialForwardAdvection(float *p_in, float*p_out, float scale, float partial)
{
		// Pressure differential between points  
	// to get an accelleration force.
	float a = m_dt * scale ;

	int w=m_w;
	int h=m_h;

	// First copy the values
	CopyField(p_in,p_out);

	if (scale == 0.0f)
		return;


	// We advect all cells
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			int cell = Cell(x,y);
			float vx = mp_xv0[cell];
			float vy = mp_yv0[cell];
			if (vx != 0.0f || vy != 0.0f)
			{
				float x1 = x + vx * a;
				float y1 = y + vy * a;
				Collide(x,y,x1,y1);
				int cell1 = Cell((int)x1,(int)y1);
				// get fractional parts
				float fx = x1-(int)x1;
				float fy = y1-(int)y1;

				// we are taking pressure from four cells (one of which might be the target cell
				// and adding it to the target cell
				// hence we need to subtract the appropiate amounts from each cell
				//
				// Cells are like
				// 
				//    A----B
				//    |    |
				//    |    |
				//    C----D
				// 
				// (Should be square)
				// so we work out the bilinear fraction at each of a,b,c,d
				
				float in = p_in[cell] * partial;

				float ia = (1.0f-fy)*(1.0f-fx) * in;
				float ib = (1.0f-fy)*(fx)      * in;
				float ic = (fy)     *(1.0f-fx) * in;
				float id = (fy)     *(fx)      * in;

				// Subtract them from the source cell
				p_out[cell] -= (ia+ib+ic+id);
				// Then add them to the four destination cells
				p_out[cell1]		+= ia;
				p_out[cell1+1]		+= ib;
				p_out[cell1+w]		+= ic;
				p_out[cell1+w+1]	+= id;
			}
		}
	}
}


void CFluid::BFECCForwardAdvection(float *p_in, float*p_out, float scale)
{
	// BFECC Algorithm from Flowfixer, Kim, et al.
	// mp_bfecc = Advect( v,p_in) 
	// p_out = Advect(-v,mp_bfecc) 
	// mp_bfecc = p_in + (p_in-p_out)/2  (Performed in 
	// Return Advect(mp_bfecc,p_out)

// DOES NOT HELP WITH THE WAY I'M DOING THIGNS

#if 0
	ReverseAdvection(p_in,p_out,scale);           // Forwards
	ReverseAdvection(p_out,mp_BFECC,-scale);	  // then backwards should give us the original
	SubFields(p_in,mp_BFECC,mp_BFECC);			  // Subtract it gives us the error
	MulField(mp_BFECC,0.5f,mp_BFECC);			  // half the error
	AddFields(p_in,mp_BFECC,mp_BFECC);			  // add to original
	ReverseAdvection(mp_BFECC,p_out,scale);		  // and advect that
#else
	ForwardAdvection(p_in,p_out,scale);           // Forwards
	ForwardAdvection(p_out,mp_BFECC,-scale);	  // then backwards should give us the original
	SubFields(p_in,mp_BFECC,mp_BFECC);			  // Subtract it gives us the error
	MulField(mp_BFECC,0.5f,mp_BFECC);			  // half the error
	AddFields(p_in,mp_BFECC,mp_BFECC);			  // add to original
	ForwardAdvection(mp_BFECC,p_out,scale);		  // and advect that
#endif
	

}



/**
 * Calculate the curl at position (i, j) in the fluid grid.
 * Physically this represents the vortex strength at the
 * cell. Computed as follows: w = (del x U) where U is the
 * velocity vector at (i, j).
 *
 **/

float CFluid::Curl(int x, int y)
{
	// difference in XV of cells above and below
    // positive number is a counter-clockwise rotation
	float x_curl = (mp_xv0[Cell(x, y + 1)] - mp_xv0[Cell(x, y - 1)]) * 0.5f;

	// difference in YV of cells left and right
    // positive number is a counter-clockwise rotation
    float y_curl = (mp_yv0[Cell(x + 1, y)] - mp_yv0[Cell(x - 1, y)]) * 0.5f;

    return x_curl - y_curl;
}


void CFluid::VorticityConfinement(float scale)
{

	ZeroEdge(mp_p1);
	ZeroField(mp_xv1);
	ZeroField(mp_yv1);

	float *p_abs_curl = mp_p1;

    for (int i = 1; i <= m_w-1; i++)
    {
        for (int j = 1; j <= m_h-1; j++)
        {
            p_abs_curl[Cell(i, j)] = fabs(Curl(i, j));
        }
    }

    for (int x = 2; x < m_w-1; x++)
    {
        for (int y = 2; y < m_h-1; y++)
        {
#if 1
			int cell = Cell(x,y);
			// get curl gradient across this cell, left right
            float lr_curl = (p_abs_curl[cell+1] - p_abs_curl[cell-1]) * 0.5f;
			// and up down
            float ud_curl = (p_abs_curl[cell+m_w] - p_abs_curl[cell+m_h]) * 0.5f;

			// Normalize the derivitive curl vector
            float length = (float) sqrtf(lr_curl * lr_curl + ud_curl * ud_curl) + 0.000001f;
            lr_curl /= length;
            ud_curl /= length;

			// get the magnitude of the curl
            float v = Curl(x, y);

			// (lr,ud) would be perpendicular, so (-ud,lr) is tangential? 
            mp_xv1[Cell(x, y)] = -ud_curl *  v;   
            mp_yv1[Cell(x, y)] =  lr_curl *  v;
#else
	float x_curl = (mp_xv0[Cell(x, y + 1)] - mp_xv0[Cell(x, y - 1)]) * 0.5f;

	// difference in YV of cells left and right
    // positive number is a counter-clockwise rotation
    float y_curl = (mp_yv0[Cell(x + 1, y)] - mp_yv0[Cell(x - 1, y)]) * 0.5f;

			mp_xv1[Cell(x,y)] = y_curl*scale;
			mp_yv1[Cell(x,y)] = x_curl*scale;

#endif

		}
    }
	ForceFrom(mp_xv1,mp_xv0,scale);
	ForceFrom(mp_yv1,mp_yv0,scale);
}

// Given a floating point position within the field, add to it 
// with bilinear interpolation
void CFluid::AddValue(float *p_in, float x, float y, float v)
{
	if (x<0 || y<0 || x>(float)m_w-1.0001f || y>(float)m_h-1.0001f)
		return;

	// get fractional parts
	float fx = x-(int)x;
	float fy = y-(int)y;
	// get the corner cell (a)
	int cell = Cell((int)x,(int)y);

	// get fractions of the values for each target cell
	float ia = (1.0f-fy)*(1.0f-fx) * v;
	float ib = (1.0f-fy)*(fx)      * v;
	float ic = (fy)     *(1.0f-fx) * v;
	float id = (fy)     *(fx)      * v;

	// Add into each cell
	p_in[cell] += ia;
	p_in[cell+1] += ib;
	p_in[cell+m_w] += ic;
	p_in[cell+m_w+1] += id;
}


void CFluid::SetValue(float *p_in, float x, float y, float v)
{
	// get fractional parts
	float fx = x-(int)x;
	float fy = y-(int)y;
	// get the corner cell (a)
	int cell = Cell((int)x,(int)y);

	// get fractions of the values for each target cell
	float ia = (1.0f-fy)*(1.0f-fx) * v;
	float ib = (1.0f-fy)*(fx)      * v;
	float ic = (fy)     *(1.0f-fx) * v;
	float id = (fy)     *(fx)      * v;

	// Set into each cell
	p_in[cell] = ia;
	p_in[cell+1] = ib;
	p_in[cell+m_w] = ic;
	p_in[cell+m_w+1] = id;
}

float CFluid::GetValue(float *p_in, float x, float y)
{
	// get fractional parts
	float fx = x-(int)x;
	float fy = y-(int)y;
	// get the corner cell (a)
	int cell = Cell((int)x,(int)y);

	// get fractions of the values from each cell
	float ia = (1.0f-fy)*(1.0f-fx) * p_in[cell];
	float ib = (1.0f-fy)*(fx)      * p_in[cell+1];
	float ic = (fy)     *(1.0f-fx) * p_in[cell+m_w];
	float id = (fy)     *(fx)      * p_in[cell+m_w+1];

	return ia+ib+ic+id;
}

// render the array on screen as a bunch of rectangles
void CFluid::Render(float x, float y, float w, float h)
{

//	if (key_w)
//		return;

	m_screen_x = x;
	m_screen_y = y;
	m_screen_w = w;
	m_screen_h = h;

	float tot_p = 0;

	float wx = w/m_w;
	float wy = h/m_h;

	float *p_ink = mp_ink0;
	float scale = 100.0f; //255.0f; 
	if (key_w)
	{
		p_ink = mp_p0;
		scale = 128;
	}
	if (key_d)
	{
		p_ink = mp_balance;
		scale = 128;
	}
	
	for (int x0=0;x0<m_w;x0++)
	{
		for (int y0=0;y0<m_h;y0++)
		{
			int cell = Cell(x0,y0);
			if (x0<m_w-1 && y0<m_h-1)
			{
				unsigned int base = 0xff000000;
				int p0 = scale * p_ink[cell];
				if (p0>255) p0 = 255;
				int p1 = scale * p_ink[cell+1];
				if (p1>255) p1 = 255;
				int p2 = scale * p_ink[cell+1+m_w];
				if (p2>255) p2 = 255;
				int p3 = scale * p_ink[cell+m_w];
				if (p3>255) p3 = 255;
				if (p0<0 || p1<0 ||p2<0 || p3<0)
					base = 0xffff0000;

				int h0 = scale * mp_heat0[cell];
				if (h0>255) h0 = 255;
				int h1 = scale * mp_heat0[cell+1];
				if (h1>255) h1 = 255;
				int h2 = scale * mp_heat0[cell+1+m_w];
				if (h2>255) h2 = 255;
				int h3 = scale * mp_heat0[cell+m_w];
				if (h3>255) h3 = 255;
				if (h0<0 || h1<0 ||h2<0 || h3<0)
					base = 0xffff0000;


				if (0 && key_d)
				{
					h0 = p0-h0;
					h1 = p1-h1;
					h2 = p2-h2;
					h3 = p3-h3;
					if (h0<0) h0 = 0;
					if (h0<1) h1 = 0;
					if (h0<2) h2 = 0;
					if (h0<3) h3 = 0;

					p0 = h0 + (h0<<8) + (p0<<16);
					p1 = h1 + (h1<<8) + (p1<<16);
					p2 = h2 + (h2<<8) + (p2<<16);
					p3 = h3 + (h3<<8) + (p3<<16);
				}	
				else
				{
					p0 = p0 + (p0<<8) + (p0<<16);
					p1 = p1 + (p1<<8) + (p1<<16);
					p2 = p2 + (p2<<8) + (p2<<16);
					p3 = p3 + (p3<<8) + (p3<<16);
				}

#if 0
				//unsigned int mask = 0xfff0f0f0;
//				unsigned int mask = 0x00ffffff;
//				p0 ^= mask;
//				p1 ^= mask;
//				p2 ^= mask;
//				p3 ^= mask;

				p1=p0;
				p2=p0;
				p3=p0;
#endif
				DrawRect(x + x0 * wx, y + y0*wy, wx,wy,base + p0, base+p1, base+p2, base+p3 );
			}
			// Draw a velocty line
			Vector2 center = Vector2(x + (x0+0.5f) * wx, y+ (y0+0.5f)*wy);

			Vector2 n = Vector2(mp_xv0[cell],mp_yv0[cell]).Normal();
			unsigned int line_color = 0xff000000 + ((int)(127.0f*(n.y+1.0f))) + ((int)(127.0f*(n.x+1.0f))<<8) + ((int)(127.0f*(n.y * n.x+1.0f))<<16) ;

			if (key_a)
				DrawLine(center, center + Vector2(mp_xv0[cell],mp_yv0[cell])*wx*50.0f,line_color);

		}
	}
}


// Update the fluid with a time step dt
void CFluid::Update(float dt)
{


	static bool update = 1;

	if (key_space)
	{
		key_space = 0;
		update = !update;
	}

	if (!update)
		return;

	if (key_s)
		Reset();

	// Time step stored locally per fluid system 
	m_dt = dt;

#if 1
	if (m_velocity_diffusion != 0.0f)
	{
		for (int i=0;i<m_diffusion_iterations;i++)
		{
			Diffusion(mp_xv0, mp_xv1, m_velocity_diffusion/(float)m_diffusion_iterations);
			swap(mp_xv0,mp_xv1);
			Diffusion(mp_yv0, mp_yv1, m_velocity_diffusion/(float)m_diffusion_iterations);
			swap(mp_yv0,mp_yv1);		}
	}
#endif

	if (m_pressure_diffusion != 0.0f)
	{
		for (int i=0;i<m_diffusion_iterations;i++)
		{
			Diffusion(mp_p0, mp_p1, m_pressure_diffusion/(float)m_diffusion_iterations);
			swap(mp_p0,mp_p1);
		}
	}
	if (m_heat_diffusion != 0.0f)
	{
		for (int i=0;i<m_diffusion_iterations;i++)
		{
			Diffusion(mp_heat0, mp_heat1, m_heat_diffusion/(float)m_diffusion_iterations);
			swap(mp_heat0,mp_heat1);

		}
	}

	if (m_ink_diffusion != 0.0f)
	{
		for (int i=0;i<m_diffusion_iterations;i++)
		{
			Diffusion(mp_ink0, mp_ink1, m_ink_diffusion/(float)m_diffusion_iterations);
			swap(mp_ink0,mp_ink1);
		}
	}


	if (m_ink_heat != 0.0f)
	{
		// TO make smoke rise under its own steam, we make the ink apply an
		// upwards force on the velocity field
		// heat froce from the ink
		ForceFrom(mp_ink0,mp_yv0,m_ink_heat);
		//ForceFrom(mp_p0,mp_yv0,ink_heat);
		// Optional Kill edge ink, so smoke edits to the top
		//ZeroEdge(mp_ink0);  
	
		// Fade ink
		//	ForceFrom(mp_ink0,mp_ink0,-0.1f);

	}

	if (m_heat_force != 0.0f && !m_field_test)
	{
		// TO make smoke rise under its own steam, we make the ink apply an
		// upwards force on the velocity field
		// heat froce from the ink
		ForceFrom(mp_heat0,mp_yv0,m_heat_force);
		//if (this == &g_fluid1)
		//ForceFrom(mp_heat0,mp_xv0,m_heat_force);
		// Kill edge ink, so smoke edits to the top
		//ZeroEdge(mp_ink0);

		// Cooling effect, otherwise things just explode
		//ForceFrom(mp_heat0,mp_heat0,-0.3f);
		if (m_heat_friction_a!=0 || m_heat_friction_b!=0 || m_heat_friction_c != 0)
			QuadraticDecay(mp_heat0, mp_heat0, m_heat_friction_a, m_heat_friction_b, m_heat_friction_c);
	}

	//ApplyForce(mp_yv0,0.01f);
	// Give a litte random force to the edges of the simulation
	// just so things preturb a little
	//EdgeForce(mp_xv0,rndf(0.01),-rndf(0.01),rndf(0.01),-rndf(0.01));
	//EdgeForce(mp_yv0,rndf(0.001),-rndf(0.001),rndf(0.001),-rndf(0.001));

	// Vorticity confinement is a cosmetic patch that accellerates the velocity
	// field in a direction tangential to the curve defined by the surrounding points
	if (m_vorticity != 0.0f)
		VorticityConfinement(m_vorticity);

	// No major diff if we swap the order of advancing pressure acc and friction forces
	if (m_pressure_acc != 0.0f && !m_field_test)
	{
		PressureAcceleration(m_pressure_acc);
	}
	if (m_velocity_friction_a != 0.0f || m_velocity_friction_b != 0.0f || m_velocity_friction_c != 0.0f)
	{
		VelocityFriction(m_velocity_friction_a, m_velocity_friction_b, m_velocity_friction_c);
	}

	// Clamping Veclocity prevents problems when too much energy is introduced into the system
	// it's not strictly necessary, and in fact leads to problems as it gives you a leading edge wave of
	// constant velocity, which cannot dissipate
//	if (this == &g_fluid2)
//	ClampVelocity(0.2f);

	float before_p = 0;
	float tot_p = 0;

	float v_tot = 0.0f;
	float v_max = 0.0f;

#if 1
	for (int x0=0;x0<m_w;x0++)
	{
		for (int y0=0;y0<m_h;y0++)
		{
			int cell = Cell(x0,y0);

			before_p += mp_p0[cell];

			Vector2 v = Vector2(mp_xv0[cell],mp_yv0[cell]);
			float v_len = v.Length();
			if (v_len > v_max)
				v_max = v_len;
			v_tot += v_len;
		}
	}
#endif


// Advection step.  
// Forward advection works well in general, but does not handle the dissipation
// of single cells of pressure (or lines of cells).  
//
// Reverse advection does not handle self-advection of velocity without diffusion
// 
// Both?  Beautiful!!!

	// Problems seems to be getting aswirly velocity field
	// and the pressure accumulating at the edges
	// and the artefacts of "ripples".  Or are they natural?
	//
	// Maybe try setting up a smoke source simulation, as that looks nice.

	// The advection scale is needed for when we change grid sizes
	// smaller grids mean large cells, so scale should be smaller
	float advection_scale = m_w / 100.f;

#if 1
	SetField(mp_ink1,1.0f);
	ForwardAdvection(mp_ink1,mp_balance,m_ink_advection * advection_scale);
	if (this == &g_fluid2)
	{
		// scaling the velocity by the advection balance actually 
		// helps reduce the compressibility of the velocity field
		// But not really in a useful way, as it's just reducing the energy at
		// points that have high velocty
//		MulFields(mp_balance,mp_yv0,mp_yv0);
//		MulFields(mp_balance,mp_xv0,mp_xv0);
	}
#endif

	// Advect the ink - ink is one fluid suspended in another, like smoke in air
	ForwardAdvection(mp_ink0, mp_ink1, m_ink_advection * advection_scale);
	swap(mp_ink0,mp_ink1);
	ReverseAdvection(mp_ink0, mp_ink1, m_ink_advection * advection_scale);
	swap(mp_ink0,mp_ink1);

	// Only advect the heat if it is applying a force
	if (m_heat_force != 0.0f)
	{
		ForwardAdvection(mp_heat0, mp_heat1, m_heat_advection * advection_scale);
		swap(mp_heat0,mp_heat1);
		ReverseAdvection(mp_heat0, mp_heat1, m_heat_advection * advection_scale);
		swap(mp_heat0,mp_heat1);
	}


// Advection order makes very significant differences
// If presure is advected first, it leads to self-maintaining waves
// and ripple artifacts
// So the "velocity first" seems preferable as it naturally dissipates the waves
// By advecting the velocity first we advect the pressure using the next frames velocity
//

	if (!m_field_test )
	{

#if 1
		// Self advect the velocity via three buffers (if reverse advecting)
		// buffers are 0,1 and 2
		// our current velocity is in 0
		// we advec t 0 to 1 using 0
		// we the  advect 1 to 2 using 0 again
		// then swap 2 into 0 so it becomes the new current velocity
		ForwardAdvection(mp_xv0, mp_xv1, m_velocity_advection * advection_scale);
		ForwardAdvection(mp_yv0, mp_yv1, m_velocity_advection * advection_scale);
	 #if 1
		// Advect from 1 into 2, then swap 0 and 2
		// We can use signed reverse advection as quantities can be negative.
		ReverseSignedAdvection(mp_xv1, mp_xv2, m_velocity_advection * advection_scale);
		ReverseSignedAdvection(mp_yv1, mp_yv2, m_velocity_advection * advection_scale);
		swap(mp_xv2,mp_xv0);
		swap(mp_yv2,mp_yv0);
	 #else
		swap(mp_xv1,mp_xv0);
		swap(mp_yv1,mp_yv0);
	 #endif
		// handle velocities at the edge, confining them to within the cells.
		// Not needed with correct edge sampling and pressure, as edge pressure will turn the vel
		// But since we have several 
		//if (m_pressure_acc == 0.0f || this == &g_fluid1)
		//{
		//	ZeroEdge(mp_xv0);
		//	ZeroEdge(mp_yv0);
		//	ZeroEdge(mp_p0);
		EdgeVelocities();
		//}
#endif
	}
	// Advect the pressure, representing the compressible fluid (like the air)

	ForwardAdvection(mp_p0, mp_p1, m_pressure_advection * advection_scale);
	swap(mp_p0,mp_p1);

	ReverseAdvection(mp_p0, mp_p1, m_pressure_advection * advection_scale);
	swap(mp_p0,mp_p1);


#if 0
	for (int x0=0;x0<m_w;x0++)
	{
		for (int y0=0;y0<m_h;y0++)
		{
			int cell = Cell(x0,y0);
			tot_p += mp_p0[cell];
		}
	}
#endif

	char buf[512];
	sprintf(buf,"dt = %2.4f, P = %4.2f, P' = %4.2f, P'-P = %2.2f,  v_tot = %.2f, v_avg = %.3f, v_max = %.3f",
		m_dt, before_p, tot_p, before_p-tot_p, v_tot, v_tot/(m_w*m_h), v_max);
	//debug_log("%s",buf);
	if (this == &g_fluid1)
		DrawString(0,0,buf,0xff0000ff);
	else
		DrawString(540,0,buf,0xff0000ff);
}


void CFluid::MouseInput()
{

// hack in some mouse stuff here

	int x = (mouse_x - m_screen_x) / (m_screen_w / m_w);
	if (x < 1) x = 1;
	if (x > m_w-1) x = m_w-2;
	int y = (mouse_y - m_screen_y) / (m_screen_h / m_h);
	if (y < 1) y = 1;
	if (y > m_h-1) y = m_h-2;



#if 0
	Vector2 center = Vector2(50,50)*(m_screen_w / m_w);
	Vector2 p1 = center;
	float radius = 100;
	Vector2 start = Vector2(8,8)*(m_screen_w / m_w);
	Vector2 end = Vector2(x,y)*(m_screen_w / m_w);
	
	char bufx[1000];
	sprintf(bufx, "(%f,%f)",end.x, end.y);
	DrawString(200,20,bufx,0xff00ff00);
	
	start = start-p1;
	end=end-p1;
	Vector2 p2;
	if (LineCircleIntersect(start, end, radius, p1, p2))
	{
		// p2 is the collision point, still in relative coordinates
		// so we need to reflect start->end about 0->p2
		// we already know that p2 is on the circle, so we have its length alreadys
		
		Vector2 p2_to_end = end-p2;
		Vector2 reflection_axis = p2/radius;
		float p2_to_end_outwards = DotProduct(p2_to_end,reflection_axis);
		// Adjust the end point
		end = end - reflection_axis * p2_to_end_outwards * 2.0f; 
		// And finally return the absolute coordinates
//		x1 = end.x;
//		y1 = end.y;
		DrawLine(p2+center,end+center,0xffffffff);
	}
#endif

#if 0
	if (dragging)
	{
		mp_p0[Cell(x,y)] += 1.0f;
		if (mp_p0[Cell(x,y)]>10.0f)
			mp_p0[Cell(x,y)]=10.0f;
	}
#endif
	if (dragging || r_dragging)
	{
		if (m_init)
		{
			Vector2 v = Vector2(x-m_last_x,y-m_last_y) * 0.05f;
#if 1
			float f_step = 0.025f; // steps along the stroke
			float r_step = 0.05f; // steps across circle
			float step_scale = f_step*r_step*r_step;
			for (float f = 0.0f; f< 1.0f; f+= f_step)
			{
				float xc = m_last_x + f *(x-m_last_x);
				float yc = m_last_y + f *(y-m_last_y);
				for (float x0 = -1.0f; x0 < 1.0f; x0+=r_step)
				{
					for (float y0 = -1.0f; y0<1.0f;y0+=r_step)
					{
						// get coords relative to the center
						float rr = sqrtf(x0*x0 + y0*y0);
						if (rr<1.0f)
						{
							// inside the circle
							Vector2 tangent = Vector2(y0,-x0);
							tangent = tangent.Normal();

							if (rr == 0.0f) rr = r_step;
							tangent = tangent*rr/10.0f;
							{
								float r_drag = 0.06f * m_w;
								float drag = 5.0f;
								if (r_dragging)
								{
									AddValue(mp_xv0,x0*r_drag+xc,y0*r_drag+yc,v.x*step_scale*drag);
									AddValue(mp_yv0,x0*r_drag+xc,y0*r_drag+yc,v.y*step_scale*drag);
								}

								if (dragging)
								{
									//AddValue(mp_p0,x0*5+xc,y0*5+yc,-0.020*(1.0f-rr)*step_scale);

									// add some velocity
									//if (this == &g_fluid2)
									if (!m_field_test)
									{
										AddValue(mp_yv0,x0+xc,y0+yc,-0.60*(1.0f-rr)*step_scale);
									}
									// and some ink

									float r_ink  = 0.001f * m_w;
									float r_heat = 0.001f * m_w;
									float r_pressure = 0.1f * m_w;
									float ink  = 0.1f * m_w;
									float heat = 0.1f * m_w;

									AddValue(mp_ink0,x0*r_ink+xc,y0*r_ink+yc,ink*(1.0f)*step_scale);
									AddValue(mp_heat0,x0*r_heat+xc,y0*r_heat+yc,heat*(1.0f)*step_scale);
									//AddValue(mp_ink0,x0*r_ink+xc,y0*r_ink+yc,ink*(1.0f-rr)*step_scale);
									//AddValue(mp_heat0,x0*r_heat+xc,y0*r_heat+yc,heat*(1.0f-rr)*step_scale);
						
									
									//AddValue(mp_p0,x0*r_pressure+xc,y0*r_pressure+yc,heat*(1.0f-rr)*step_scale);
									//AddValue(mp_ink0,xc,yc,0.050*(r-rr)*step_scale);

								}
							}
						}
					}
				}
			}
#else
			mp_xv0[Cell(x,y)] = 20.0f;
#endif


		}
		m_init = 1;
		m_last_x = x;
		m_last_y = y;
	}
	else
	{
		m_init = 0;
	}



#if 1
	if (0 && key_d)
	{
		key_d = 0;
		SVG_Start("C:\\Mick\\Docs\\GamesIndustry\\GameDeveloperArticles\\Fluid and Gas  Dynamics\\SVG_Velocity.SVG");

		float step = 60.0f * 8.0/m_w;
		for (int x = 0; x<m_w; x++)
		{
			for (int y = 0; y<m_h; y++)
			{
				float xx = step * x;
				float yy = step * y;
				if (x < m_w-1)
					SVG_Line(0x404040,1,xx,yy,xx+step,yy);
				if (y < m_h-1)
					SVG_Line(0x404040,1,xx,yy,xx,yy+step);

				SVG_Dot(0xFF0000,xx,yy,step/20);
				int cell = Cell(x,y);
				float scale = 40.0f;
				// The actual points used
				float xv = mp_xv0[cell]*step*m_velocity_advection*m_dt;
				float yv = mp_yv0[cell]*step*m_velocity_advection*m_dt;
				//SVG_Line(0x506000,2,xx,yy,xx+xv,yy+yv);
				SVG_Arrow(0x506000,1,xx,yy,xx+xv,yy+yv,0.2f,0.05f);
				


			}
		}

	
		SVG_End();
	}


#endif


}



char  printf_buffer[1024];

void    debug_log( const char* text, ...)
{
	EnterCriticalSection(&debug_CS);

// Get Text into a printable buffer	 (maybe prepend time)
    va_list args;
	va_start( args, text );
	vsprintf( printf_buffer, text, args);
	va_end( args );
// output as debug text
    OutputDebugString(printf_buffer);

	LeaveCriticalSection(&debug_CS);

}

float game_time = 0.0f;

static bool paused = false;

void MX_Logic(float time)
{
    static float last_time = 0.0f;
    
    float timestep = time - last_time;
    
    if (timestep > 0.25f) timestep = 0.25f;

    last_time = time;

    if (paused)
    {
    }
    else
    {
        //game_time += timestep;
        game_time = Timer_Seconds();
    }

}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    
		case WM_SETFOCUS:
			has_focus = 1;
			break;

		case WM_KILLFOCUS:
			has_focus = 0;
			break;


        case WM_SIZE:
    	{
//    	    resize_db(hWnd);		
//    		InitD3D(hWnd);
            g_resize = true;
            break;
    	}           


		case WM_RBUTTONUP:
		{
			r_dragging = false;
			break;
		}


		case WM_RBUTTONDOWN:
		{
			r_dragging = true;
			break;
		}

		case WM_LBUTTONUP:
		{
			dragging = false;
			break;
		}


		case WM_LBUTTONDOWN:
		{
			dragging = true;
	// DROP THROUGH
	//		break;
		}

		
		
		case WM_MOUSEMOVE:
		{
			//if (dragging)
			{
				mouse_x = LOWORD(lParam);  // X position of cursor 
				mouse_y = HIWORD(lParam);  // Y position of cursor
			}
			break;	
		}

	case WM_KEYDOWN:
		{
			int key = HIWORD(lParam) & 0x1ff;
			debug_log("%x",key);
			if (key == 0x11) key_w = 1;
			if (key == 0x1e) key_a = 1;
			if (key == 0x1f) key_s = 1;
			if (key == 0x20) key_d = 1;
			if (key == 0x39) key_space = 1;
			if (key == 0x14b) key_left = 1;
			if (key == 0x14d) key_right = 1;
			if (key == 0x148) key_up = 1;
			if (key == 0x150) key_down = 1;

		}
		break;

		case WM_KEYUP:
		{
			int key = HIWORD(lParam) & 0x1ff;
			if (key == 0x11) key_w = 0;
			if (key == 0x1e) key_a = 0;
			if (key == 0x1f) key_s = 0;
			if (key == 0x20) key_d = 0;
			if (key == 0x39) key_space = 0;
			if (key == 0x14b) key_left = 0;
			if (key == 0x14d) key_right = 0;
			if (key == 0x148) key_up = 0;
			if (key == 0x150) key_down = 0;

		}
		break;

            
    }


    return DefWindowProc( hWnd, msg, wParam, lParam );
}

void InitFluids()
{

	memset(&g_fluid1,0,sizeof(CFluid));
	memset(&g_fluid2,0,sizeof(CFluid));



	// Fluid1 is the one with the double time step

	g_fluid1.m_diffusion_iterations = 1;

	g_fluid1.m_velocity_diffusion = 0.0f; //3.0f; //3.5f; // 3.5f works nicely

	// With no pressure diffusion, waves of pressure keep moving
	// But only if pressure is advected ahead of velocity
	g_fluid1.m_pressure_diffusion = 0.0f; //6.0f; // 3.5f; // too big a diffusion scale creates innacuracies, but it's fine around here

	g_fluid1.m_heat_diffusion = 0.0f; // 3.0f;
	
	g_fluid1.m_ink_diffusion = 0.0f; //1.0f; // 3.0f;

	// friction applied to velocity.  As a simple fraction each time step
	// so 1.0f will give you no friction

	// Friction is very important to the realism of the simulation
	// changes to these coeffficinets will GREATLY affect the apperence of the simulation.
	// the high factor friction (a) wich is applied to the square of the velocity,
	// acts as a smooth limit to velocity, which causes streams to break up into turbulent flow 
	// The c term will make things stop, which then shows up our border artefacts
	// but a high enough (>0.01) c term is needed to stop things just dissipating too fast
#if 0
	// Friction is dt*(a*v^2 + b*v + c)
	g_fluid1.m_velocity_friction_a = 0.80f;    
	g_fluid1.m_velocity_friction_b = 0.03f; 
	g_fluid1.m_velocity_friction_c = 0.001f;  // 0.01f is nice for stopping things 
#else
	g_fluid1.m_velocity_friction_a = 0.0f; 
	g_fluid1.m_velocity_friction_b = 0.0f; 
	g_fluid1.m_velocity_friction_c = 0.0f; 
#endif

	g_fluid1.m_vorticity = 0.0f;

	// Pressure accelleration.  Larger values (>0.5) are more realistic (like water)
	// values that are too large lead to chaotic waves
	g_fluid1.m_pressure_acc = 2; // 2.0 worked well for smoke-like stuff

	// upwards force applied by ink
#if 0
// ink heats itself
	g_fluid1.m_ink_heat   = -0.1f;
	g_fluid1.m_heat_force = 0.0f; //-0.1f;
#else
	// heat follows ink in advection, but is a seperate scalar value
	g_fluid1.m_ink_heat   = 0.0f;
	g_fluid1.m_heat_force = -0.1f; //-0.1f;
#endif
	g_fluid1.m_heat_friction_a = 0.5f;
	g_fluid1.m_heat_friction_b = 0.2f;
	g_fluid1.m_heat_friction_c = 0.01f;
#if 0
	g_fluid1.m_velocity_advection = 140.0f;
	g_fluid1.m_pressure_advection = 10.0f;
#else
// high ink advection allows fast moving nice swirlys
	g_fluid1.m_ink_advection = 150.0f;
	// seems nice if both velocity and pressure at same value, which makes sense
	g_fluid1.m_velocity_advection = 150.0f;
	g_fluid1.m_pressure_advection = 150.0f;  // 130, lag behind, to allow vel to dissipate???
	g_fluid1.m_heat_advection = 150.0f;
#endif

	//////////////////////////////////////////////////////////////

	g_fluid1.m_field_test = 0;

	g_fluid2 = g_fluid1;

//	g_fluid1.Init(100,100);
//	g_fluid2.Init(100,100);
	g_fluid1.Init(100,100);
	g_fluid2.Init(100,100);

#if 0
	// test with a static velocity field
	g_fluid1.m_field_test = 1;
	g_fluid1.SetField(g_fluid1.mp_xv0,-0.1f);
	g_fluid1.SetField(g_fluid1.mp_yv0,-0.1f);
	g_fluid2.m_field_test = 1;
	g_fluid2.SetField(g_fluid2.mp_xv0,-0.1f);
	g_fluid2.SetField(g_fluid2.mp_yv0,-0.1f);
#endif

}

