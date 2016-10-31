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
// You should have Visual studio files blob.sln and blob.vcproj which will allow
// you to compile under visual studio 2005.
// if not, then the following info should be enough to get you compiling.
//
// The entire project consists of four source files:
//   blob.cpp (this file)
//   vector2.h  (the 2D vector library)
//   verlet.cpp/h	(the basic verlet stuff)
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
// Article specific code start around line 3040
//
// Since this is based on my ongoing article code testbed
// it's rather messy.  Just ignore the other code (particles and event tracking)
//


//#define	FIGURES  // for screenshot figures

#if 1

#define	MANUAL_OPT					// some manual optimizations
#define	PRETTY_UP					// Much nicer looking graphics and effect, a bit slower
//#define	NBODY						// all particles affect each other, reduce number of particles.
//#define	USE_WORKER_THREADS				// define either of these to enable threading
//#define USE_FORKED_THREADS
//#define SET_AFFINITY
#define PROCESS_CHUNKS
#define	MAX_THREAD	4					// any number you like, but typically 1,2,4 or 8 will work best
//#define	USE_DUMMY_PHYSICS			// compile in a full tilt thread
//#define	USE_FORK_MARKING			// fork/join a quick thread at the end of the update as a marker
//#define	CRITICAL_SECTION_RND		// Not strictly necessary, so off for speed
//#define FAST_RND

#endif

float g_friction = 1.0f;

#ifdef NBODY
const int	NUM_PARTICLES = 100;		// we have NxN interactions, so reduce number of particles.
#else
const int	NUM_PARTICLES = 1; //80000;  // 80000 is good on a 3.2ghz DC/HT.  
									 // But you can go up to 240000, which is quite impressive
#endif

// Checking optimizations:
//
// FP Model:
// Precise:				0.0214  << DEFAULT BASELINE
// Strict:				0.0253
// Fast:				0.0182  *** (15% faster)
//
// Exceptions -			No diff                          
//
// SIMD 2				0.0208  ***
//
// Opt Minimize size	0.0250
// Opt Full				unch

// Intrinsics			unch
//
// link time code gen	0.0206  ***
//
// RTTI off				unch
//
// Calling conv fastcall	unch

// All three Fast FP, SIMD2, Link time code
// =   0.0095
// with four threads:	0.0036, 0.0138


#pragma warning(disable: 4995)          // don't warn about deprecated functions
#define _CRT_SECURE_NO_DEPRECATE		// ditto

// Hooks into the simple sample
void MX_Init();
void MX_Render();
void MX_Logic(float time);
void MX_Cleanup();

void	VerletInit();
void	VerletRender();
void	VerletLogic();
void	VerletCleanup();

void    debug_log( const char* text, ...);

const int   g_viewport_width = 1024;
const int   g_viewport_height = 768;

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
#include "verlet.h"

bool	dragging=0;
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


class CParticle
{

public:
		Vector2	m_old_pos;
		Vector2	m_pos;
		Vector2	m_vel;
		DWORD	m_color;
		void	Update(float time);
		void	UpdateVerlet(float time);

// later		void	Update(float t);
};

class	CParticleManager
{
public:
		CParticleManager();
		~CParticleManager();
		void Init(int n);
		void Update(float time);
		void Render();
//private:
		CParticle	*mp_particles;
		int			m_num_particles;
};

CParticleManager	g_ParticleManager;


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


//-----------------------------------------------------------------------------
// Function-prototypes for directinput handlers
//-----------------------------------------------------------------------------
BOOL CALLBACK    EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
HRESULT InitDirectInput( HWND hDlg );
VOID    FreeDirectInput();
HRESULT UpdateInputState( HWND hDlg );


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

inline float  scale_x(float x) {return x * (float)g_window_width / (float) g_viewport_width;}
inline float  scale_y(float y) {return y * (float)g_window_height / (float) g_viewport_height;}

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
const int MAX_TRIS=500000;
int	g_nTris = 0;

CUSTOMVERTEX *g_pLineVerts;
const int MAX_LINES=500000;
int	g_nLines = 0;

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

////////////////////////////////////////////////////////////////////////////////////////
// DirectX Joystick handling starts here:

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

LPDIRECTINPUT8       g_pDI              = NULL;         
LPDIRECTINPUTDEVICE8 g_pJoystick        = NULL;     

#define JOY_BUFFERSIZE  64
DIDEVICEOBJECTDATA   g_inputbuffer[JOY_BUFFERSIZE];        // Input buffer for joystick events


DIJOYSTATE2 js;           // DInput Joystick state 


//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT InitDirectInput( HWND hDlg )
{
    HRESULT hr;

	for (int i=0;i<256;i++)
		js.rgbButtons[i] = 0;
	js.rgdwPOV[0] = -1;


    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return hr;

    // Look for a simple Joystick we can use for this sample program.
    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
                                         EnumJoysticksCallback,
                                         NULL, DIEDFL_ATTACHEDONLY ) ) )
        return hr;                                            

    // Make sure we got a Joystick
    if( NULL == g_pJoystick )
    {
      //  MessageBox( NULL, TEXT("Joystick not found."),  
      //              TEXT("DirectInput Sample"), 
      //              MB_ICONERROR | MB_OK );
      //  EndDialog( hDlg, 0 );
		// Not too worried about this for this application      
		return S_OK;
    }

    // Set the data format to "simple Joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if( FAILED( hr = g_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
        return hr;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if( FAILED( hr = g_pJoystick->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | 
                                                             DISCL_FOREGROUND ) ) )
        return hr;

    // Enumerate the Joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    if( FAILED( hr = g_pJoystick->EnumObjects( EnumObjectsCallback, 
                                                (VOID*)hDlg, DIDFT_ALL ) ) )
        return hr;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated Joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
    HRESULT hr;

    // Obtain an interface to the enumerated Joystick.
    hr = g_pDI->CreateDevice( pdidInstance->guidInstance, &g_pJoystick, NULL );

    DIDEVICEINSTANCE    device_info;

    g_pJoystick->GetDeviceInfo(&device_info);

    // If it failed, then we can't use this Joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED(hr) ) 
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first Joystick we get. You
    // could store all the enumerated Joysticks and let the user pick.
    return DIENUM_STOP;
    
    // Continue Enumeration, let's pick the best joystick
//    return DIENUM_CONTINUE; 
      
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       Joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    HWND hDlg = (HWND)pContext;

    static int nSliderCount = 0;  // Number of returned slider controls
    static int nPOVCount = 0;     // Number of returned POV controls

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwHow        = DIPH_BYID; 
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin              = -1000; 
        diprg.lMax              = +1000; 
    
        // Set the range for the axis
        if( FAILED( g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
            return DIENUM_STOP;


        DIPROPDWORD  dipdw; 
        dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        dipdw.diph.dwObj = 0; 
        dipdw.diph.dwHow = DIPH_DEVICE; 
        dipdw.dwData = JOY_BUFFERSIZE; 

        // Set the size of the data buffer
        if( FAILED( g_pJoystick->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) ) )
            return DIENUM_STOP;
         
    }
    
    return DIENUM_CONTINUE;

}



//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
VOID FreeDirectInput()
{
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pJoystick ) 
        g_pJoystick->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pJoystick );
    SAFE_RELEASE( g_pDI );
}


///////////////////////////////////////////////////////////////////////////
// ReadControllerState is an interface function between my code
// and the DirectInput framework
// takes a 256 entry byte array
// returns with the first 12 entries being the gamepad buttons
// in the order X,A,B,Y,L1,R1,L2,R2,SELECT,START,L3,R3
// and entries 128,129,130,132 being U,D,L,R
void ReadControllerState(unsigned char *buttons)
{
    // copy over the first 128 buttons
    for (int i=0;i<128;i++)
    buttons[i] = js.rgbButtons[i];
    // and create remaining buttons from the D-Pad direction
    
    // POV in directX correspond to the DPad    
    // -1 = nothing
    // anything else = angle in degrees, time 100, so, 9000 for right. 22500 for down left

    // This is a rather odd mapping, but it works    
    switch (js.rgdwPOV[0])
    {
        case -1:
            buttons[128] = 0;
            buttons[129] = 0;
            buttons[130] = 0;
            buttons[131] = 0;
            break;
        case 0:
            buttons[128] = 1;
            buttons[129] = 0;
            buttons[130] = 0;
            buttons[131] = 0;
            break;
        case 4500:
            buttons[128] = 1;
            buttons[129] = 0;
            buttons[130] = 0;
            buttons[131] = 1;
            break;
        case 9000:
            buttons[128] = 0;
            buttons[129] = 0;
            buttons[130] = 0;
            buttons[131] = 1;
            break;
        case 13500:
            buttons[128] = 0;
            buttons[129] = 1;
            buttons[130] = 0;
            buttons[131] = 1;
            break;
        case 18000:
            buttons[128] = 0;
            buttons[129] = 1;
            buttons[130] = 0;
            buttons[131] = 0;
            break;
        case 22500:
            buttons[128] = 0;
            buttons[129] = 1;
            buttons[130] = 1;
            buttons[131] = 0;
            break;
        case 27000:
            buttons[128] = 0;
            buttons[129] = 0;
            buttons[130] = 1;
            buttons[131] = 0;
            break;
        case 31500:
            buttons[128] = 1;
            buttons[129] = 0;
            buttons[130] = 1;
            buttons[131] = 0;
            break;
    }
}


// End of DirectX joystick handling code
/////////////////////////////////////////////////////////////////////////////////

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

        MX_Render();
        
        // Refill the VB, allowing us to draw whatever
		FillVB();
		
		// Draw the triangles in the vertex buffer. This is broken into a few
        // steps. We are passing the Vertices down a "stream", so first we need
        // to specify the source of that stream, which is our vertex buffer. Then
        // we need to let D3D know what vertex shader to use. Full, custom vertex
        // shaders are an advanced topic, but in most cases the vertex shader is
        // just the FVF, so that D3D knows what type of Vertices we are dealing
        // with. Finally, we call DrawPrimitive() which does the actual rendering
        // of our geometry (in this case, just one triangle).
        g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
        g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nTris/3 );

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
                      "Verlet Blob", NULL };
    RegisterClassEx( &wc );


    // Create the application's window
	HWND hWnd = CreateWindow( "Verlet Blob", "Mick West: Blob Verlet Physics Example",
                              WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );
                              
	// Get the size of the renderable rectangle
	GetClientRect(hWnd,&window_rect);
    g_window_width = window_rect.right-window_rect.left;
	g_window_height = window_rect.bottom-window_rect.top;

 //   debug_log("ClientRect Window = %dx%d",g_window_width,g_window_height);
                              


    if( FAILED( InitDirectInput( hWnd ) ) )
    {
        MessageBox( NULL, TEXT("Error Initializing DirectInput"), 
                    TEXT("Button Disambiguation Example"), MB_ICONERROR | MB_OK );
    }

    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        if( SUCCEEDED( InitVB() ) )
        {
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            MX_Init();

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
                    float   start = Timer_Seconds();
                    UpdateInputState( hWnd );
                    MX_Logic(Timer_Seconds());
                    Render();
                    //while (Timer_Seconds() > start && Timer_Seconds() < start + 4.0f * 0.016666f)
                    {
                        // Waiting for a frame to elapse, so we go to 60 fps
                    }
                        
                }
            }            
            MX_Cleanup();
        }
    }

    UnregisterClass( "ButtonDisambiguation", wc.hInstance );
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


void DrawLine(float x0,float y0,float x1,float y1, DWORD color)
{
    DrawLine2(x0,y0,x1,y1,color,color);
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

// Draw an axis aligned rectangle
void DrawRect(float x, float y, float w, float h, DWORD color)
{
	DrawQuad(x,y,x+w,y,x+w,y+h,x,y+h,color);
}

///////////////////////////////////////////////////////////////////////////////////
// World handling code
// handles rendering and collision detection 
// The world is a very simple 2D map, 80 blocks wide by 60 high

#ifdef FIGURES

unsigned char world[] = 
"11111111111111111111111111111111111111111111111111111111111111111111111111111111"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"11111111111111111111111111111111111111111111111111111111111111111111111111111111";

#else

unsigned char world[] = 
"11111111111111111111111111111111111111111111111111111111111111111111111111111111"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1111111111111111      1111111111111       11111111111111        1111111111111111"
"1111111111111111      11111111111111      11111111111111        1111111111111111"
"1111111111111111      11111111111111      11111111111111        1111111111111111"
"1111111111111111      111111111111111     11111111111111        1111111111111111"
"1111111111111111      111111111111111     11111111111111        1111111111111111"
"1                                  11     11111111111111        1111111111111111"
"1                                  11     11111111111111        1111111111111111"
"1                                  11     11111111111111        1111111111111111"
"1                                  11     11111111111111        1111111111111111"
"1                                  11     11111111111111                       1"
"1111111111111111                   11     11111111111111                       1"
"1111111111111111                   11     11111111111111                       1"
"1111111111111111                   11     11111111111111                       1"
"1                                  11     11111111111111                       1"
"1                                  11     11111111111111                       1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"11111                                                           1111111111111111"
"11111                                                           1111111111111111"
"11111                               11111                       1111111111111111"
"1                                   11111                       1111111111111111"
"1                                   11111                       1111111111111111"
"1                                   11111                       1111111111111111"
"1                                   11111                       1111111111111111"
"1                                   11111                                      1"
"1             11111111              11111                                      1"
"1             11111111              11111                                      1"
"1             11111111              11111                                      1"
"1             11111111              11111                                      1"
"1             11111111              11111                                      1"
"1                                   11111                                      3"
"1                            11111111111111111111111111                        3"
"1                            11111111111111111111111111                        3"
"1                            1111111111111111111111111111111                   3"
"1                  11111111111111111111111111111111111111111111111             3"
"1                 111111111111111111111111111111111111111111111111             3"
"1                1111111111111111111111111111111111111111111111                3"
"1                11111111111111111111111111111111111111111111                  3"
"11111111                                                                       3"
"11111111                                                                       3"
"11111111                                                                  111113"
"11111111                                                              1111111113"
"11111111                                                          11111111111113"
"11111111                            1111111                      111111111111113"
"11111111                            11111111111111111111111111111111111111111113"
"11111111111111111111111111111111111111111111111111111111111111111111111111111111";


#endif


unsigned char old_world[] = 
"11111111111111111111111111111111111111111111111111111111111111111111111111111111"
"1                                                                              1"
"1                                                                              1"
"1                               11111111111111111111                           1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                   111111111111111111         1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                222222222222221"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                            22222222222                                       1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1        22222222                                                              1"
"1        22222222                                   11111111                   1"
"1        22222222                                                              1"
"1        22222222                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"111111111111111111111                                    11111111111111111111111"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                        11111111111111111111111"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1                                                                              1"
"1111111111111111111111                      111111111111111111111111111111111111"
"1                                                                              1"
"1                                                                              3"
"1                                                                              3"
"1                                                                              3"
"1                                                                              3"
"11111111         1111111111111111111111111111111111111111111111111             3"
"1                                                                         111113"
"1                                                                         111113"
"1                                                                         111113"
"1                                                                         111113"
"1                                                                         111113"
"1          22222                    1111111                            111111113"
"1          22222                    1111111                            111111113"
"1          22222                    1111111                            111111113"
"1          22222                    1111111                            111111113"
"1          22222                    1111111                            111111113"
"11111111111111111111111111111111111111111111111111111111111111111111111111111111";

const int world_width   = 80;
const int world_height  = 60;

DWORD   get_color_from_cell(unsigned char cell)
{
    switch (cell)
    {
        case ' ':
            return 0;          // space = transparent
        case '1':
            return 0xff804040;          // 1 = lt red
        case '2':
            return 0xff408040;          // 2 = lt green
        case '3':
            return 0xff404040;          // 3 = dark grey

    default:
            return  0xffff00ff;         // default = magenta;
        
    }
}


void RenderWorld()
{
    unsigned char *p_world = world;
    float cell_width = (float)g_viewport_width/(float)(world_width);
    float cell_height = (float)g_viewport_height/(float)(world_height);
    
    for (int y=0;y<world_height;y++)
    {
        for (int x=0;x<world_width;x++)
        {
            unsigned char cell = *p_world++;
            DWORD color = get_color_from_cell(cell);
            if (color != 0)
            {
                DrawRect(x*cell_width,y*cell_height,cell_width,cell_height,color);
            }
        }
    }
}

// Just tells you if a point in world coordinates is inside a cell that is solid.
bool    WorldCollisionAt(Vector2 &pos)
{
    if (pos.x < 0.0f || pos.x >= g_viewport_width || pos.y < 0.0f || pos.y >= g_viewport_height)
    {
        return true;
    }

    float cell_width = (float)g_viewport_width/(float)(world_width);
    float cell_height = (float)g_viewport_height/(float)(world_height);
    int row = (int) (pos.y / cell_height);
    int col = (int) (pos.x / cell_width);
    return (world[row*world_width + col] != ' ');
}


////////////////////////////////////////////////////////////////////////////////////
// CWhisker - a class for a line/world collision detection
// A whisker is a lin in world space, you set the line start end end points
// and then call CheckCollision to see if there is a collision there
// if there is, then you can furthur query it to get the collision point, and the 
// vector for the unit vector of the surface we collided with
// this just uses our rather simple cell based world for now
// 
// The Whisker class is currently hard wired into the simple cell based world
// but the interface is independent of this, so if the world changes
// then we should easily be able to fix this without breaking anything.

class CWhisker
{
    public:
                    CWhisker();
                    CWhisker(Vector2 &start, Vector2 &end);
        void        Set(Vector2 &start, Vector2 &end);        
        bool        CheckCollision();
        bool        CheckCollision(Vector2 &start, Vector2 &end);        
        Vector2     GetNormal() {return m_normal;}
        Vector2     GetPoint()  {return m_point;}
        Vector2     GetStart()  {return m_start;}
        Vector2     GetEnd()  {return m_end;}
    private:
        // Input data
        Vector2     m_start, m_end;
        // Output/Calculated Data
        Vector2     m_point, m_normal;
};

CWhisker::CWhisker()
{
    m_start.x = 0.0f;
    m_start.y = 0.0f;
    m_end.x = 1.0f;
    m_start.y = 1.0f;
	//m_point = m_normal = Vector2(0,0);
}

CWhisker::CWhisker(Vector2 &start, Vector2 &end)
{
    m_start = start;
    m_end = end;
}

void CWhisker::Set(Vector2 &start, Vector2 &end)
{
    m_start = start;
    m_end = end;
}


bool CWhisker::CheckCollision()
{

    float cell_width = (float)g_viewport_width/(float)(world_width);
    float cell_height = (float)g_viewport_height/(float)(world_height);
    
    // Okay, we know the line starts in the clear
    // so we need to trace along the line, checking all the cells along the way
    // and see if we hit one of them
    // we do this seperatly for X 
    //  - find how many column edges we cross
    //  - iterate over them
    //    - find the cell from line intersection
    //    - if the cell is occupied
    //    - we have a collision
    // repeat for Y
    // return the collision closest to m_start

    bool    found_x = false;                                      
    Vector2 point_x;
    Vector2 normal_x;
    int xdir = (m_end.x > m_start.x) ? 1 : -1 ;                 // which direction is the line goin in, right or left
    int col_start = (int) (m_start.x / cell_width);             // map column to start in
    int col_end   = (int) (m_end.x / cell_width);               // and end in
    for (int col_test = col_start+xdir; col_test != col_end+xdir; col_test+=xdir)
    {
        // col_test is the column the line is now going into
        float x = col_test * cell_width;
        // if going left, then we want to be on the right side of this cell
        if (xdir < 0.0f) { x+=cell_width; }
        // we use a simple similar triangles line collision to find the y point
        float y = m_start.y + (x-m_start.x)*(m_end.y-m_start.y)/(m_end.x-m_start.x);
        int row = (int) (y / cell_height);
        unsigned char cell = world[row*world_width + col_test];
        if (cell != ' ')
        {
            found_x = true;
            point_x = Vector2(x,y);                
            normal_x = Vector2((float)-xdir, 0.0f);
            break;         
        }
    }

    // repeate exactly the same for y, just changing x/y w/h row/col senses.
    bool    found_y = false;                                      
    Vector2 point_y;
    Vector2 normal_y;
    int ydir = (m_end.y > m_start.y) ? 1 : -1 ;
    int row_start = (int) (m_start.y / cell_height);
    int row_end   = (int) (m_end.y / cell_height);
    for (int row_test = row_start+ydir; row_test != row_end+ydir; row_test+=ydir)
    {
        // row_test is the row the line is now going into
        float y = row_test * cell_height;
        // if going up, then we want to be on the bottom side of this cell
        if (ydir < 0.0f) { y+=cell_height; }
        // we use a simple similar triangles line collision to find the x point
        float x = m_start.x + (y-m_start.y)*(m_end.x-m_start.x)/(m_end.y-m_start.y);
        int col = (int) (x / cell_width);
        unsigned char cell = world[row_test*world_width + col];
        if (cell != ' ')
        {
            found_y = true;
            point_y = Vector2(x,y);                
            normal_y = Vector2(0.0f, (float)-ydir);
            break;         
        }
    }

    // Return the found collision which is closes to the start point, if any.
    if (found_x)
    {
        if (found_y && (point_y - m_start).Length2() < (point_x - m_start).Length2())  
        {
            m_point = point_y;
            m_normal = normal_y;
            return true;
        }
        else
        {
            m_point = point_x;
            m_normal = normal_x;
            return true;
        }
    }
    else
    {
        if (found_y)
        {
            m_point = point_y;
            m_normal = normal_y;
            return true;
         }
    }
    
    // not found anything - m_point and m_normal will be undefined.
    return false;       
}

bool CWhisker::CheckCollision(Vector2 &start, Vector2 &end)
{
    Set(start,end);
    return CheckCollision(); 
}

///////////////////////////////////////////////////////////////////////////////

class   CPadButton
{
    public:
        CPadButton();
        ~CPadButton();
        // The UpdateState function is called by the manager to update the state
        // based on hardware specific implementation
        // passing in the button 
        void     UpdateState(bool pressed, float current_time);
    
    
// The simplest implementation of a button would just return if it is pressed right now        
        bool    Pressed() {return m_pressed;}
// More involved would tell you when it was pressed         
        float   PressedTime() {return m_pressed_time;}
        float   ReleasedTime() {return m_released_time;}
// Even more, detecting a "Trigger", and being able to clear it after using it        
        bool    Triggered() {return m_triggered;};
        void    ClearTrigger() {m_triggered = false;}
        bool    Released() {return m_released;}
        void    ClearReleased() {m_released = false;} 
//private:
        bool    m_pressed;
        bool    m_triggered;
        bool    m_released;
        float   m_pressed_time;
        float   m_released_time;
        char    *mp_name;
};

CPadButton::CPadButton()
{
//    g_pWatchManager->SetWatch(&m_pressed);    
    m_pressed = false;
    m_triggered = false;
    m_released = false;
}

CPadButton::~CPadButton()
{
//    delete  mp_watch;
}

void     CPadButton::UpdateState(bool pressed, float current_time)
{

    if (pressed)
    {
        if (!m_pressed)
        {
            debug_log("%4.3f: + Pressed %s",current_time, mp_name);
            m_triggered = true;
            m_pressed_time = current_time;
        }
    }
    else
    {
        if (m_pressed)
        {
            debug_log("%4.3f: - Released %s",current_time, mp_name);
            m_released = true;
            m_released_time = current_time;
        }
    }
    m_pressed = pressed;
}

class   CGamepad
{
    public:
        CGamepad();
        // We are just going to store an array of buttons
        // so we give each one a name so we can refer to it.
        enum    EButton
        {
            BTN_A,
            BTN_B,
            BTN_X,
            BTN_Y,
            BTN_U,
            BTN_D,
            BTN_L,
            BTN_R,
            BTN_L1,
            BTN_R1,
            BTN_L2,
            BTN_R2,
            BTN_START,
            BTN_SELECT,
            BTN_L3,
            BTN_R3,
            
            
            NUM_BUTTONS
        };
        
        CPadButton  button[NUM_BUTTONS];
    
        void        Update( float time);

    private:
    
        
};

char * p_names[] =  {"A","B","X","Y","Up","Down","Left","Right","L1","R1","L2","R2","START","SELECT","L3","R3" };

CGamepad::CGamepad()
{
    for (int i=0;i<NUM_BUTTONS;i++)
    {
        button[i].mp_name = p_names[i];
		button[i].m_pressed = false;
    }
}


void    CGamepad::Update(float time)
{

    unsigned char buttons[256];
    ReadControllerState(buttons);      // Hook into the framework to get the controller state                
                   
    // For each button, update with the relevent state from the DirectInput joystick
    // (the ?true:false is to avoid the compiler's "performance warning" by making the conversion explicit
    
    button[BTN_X].UpdateState((buttons[0]?true:false || key_space), time);
    button[BTN_Y].UpdateState((buttons[3]?true:false) || key_a, time);
    button[BTN_A].UpdateState((buttons[1]?true:false) || key_s, time);
    button[BTN_B].UpdateState((buttons[2]?true:false) || key_d, time);
    button[BTN_L1].UpdateState((buttons[4]?true:false) , time);
    button[BTN_R1].UpdateState(buttons[5]?true:false, time);
    button[BTN_L2].UpdateState(buttons[6]?true:false, time);
    button[BTN_R2].UpdateState(buttons[7]?true:false, time);
    button[BTN_SELECT].UpdateState(buttons[8]?true:false, time);
    button[BTN_START].UpdateState(buttons[9]?true:false, time);
    button[BTN_L3].UpdateState(buttons[10]?true:false, time);
    button[BTN_R3].UpdateState(buttons[11]?true:false, time);
    button[BTN_U].UpdateState((buttons[128]?true:false) || key_up, time);
    button[BTN_D].UpdateState((buttons[129]?true:false) || key_down, time);
    button[BTN_L].UpdateState((buttons[130]?true:false) || key_left, time);
    button[BTN_R].UpdateState((buttons[131]?true:false) || key_right, time);
}

CGamepad                    * g_pGamepad;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// CWatch - Records the state of a particular variable
// this is a debugging class, and would not be used in production code
class   CWatch
{
    friend class    CWatchManager;
                 
                 CWatch();
    void        SetWatch(float *p);
    void        SetWatch(int *p);
    void        SetWatch(char *p);
    void        SetWatch(unsigned char *p);
    void        SetWatch(bool *p);
    void        Update(float time);
    void        Render(float pos_x, float pos_y, float width, float height, float border, float period, float left_area, float time, float time_off );
    
    private:
        enum    EWatchType {
            WATCH_CHAR,
            WATCH_INT,
            WATCH_FLOAT,
            WATCH_BOOL,
        };    
    struct SWatchValue{
        float time;
        union {
            int             i;
            float           f;   
        };
    };  
    
    void init(void*p, EWatchType type);
            
    int get_1_0_at_index(int index);

    const static int   max_watch = 1024;
    
    SWatchValue     m_values[max_watch];       //  MEMOPT!!!   1024*8  8K hmm
    int             m_value_index;
    int             m_num_values;
    void          * mp_watch;       // The address to watch
    EWatchType      m_type;

    DWORD           m_color;
    const char *    mp_desc;
    CWatch  *       mp_next;        // next one in the list

};

CWatch::CWatch()
{
    mp_next = NULL;
}

void CWatch::init(void*p, EWatchType type)
{
    m_value_index   = 0;
    m_num_values    = 0;
    mp_watch        = p;
    m_type          = type;
}

void        CWatch::SetWatch(float *p)
{
    init((void*)p, WATCH_FLOAT);
}

void        CWatch::SetWatch(int *p)
{
    init((void*)p, WATCH_INT);
}

void        CWatch::SetWatch(char *p)
{
    init((void*)p, WATCH_CHAR);
}

void        CWatch::SetWatch(unsigned char *p)
{
    init((void*)p, WATCH_CHAR);
}

void        CWatch::SetWatch(bool *p)
{
    init((void*)p, WATCH_BOOL);
}

void        CWatch::Update(float time)
{
// in theory we could just store it when it changes
// but for now it's simpler just to store all the entries.
    m_values[m_value_index].time = time;
    switch (m_type)
    {
        case WATCH_CHAR:
            m_values[m_value_index].i = *(char*)mp_watch;
            break;
        case WATCH_INT:
            m_values[m_value_index].i = *(int*)mp_watch;
            break;
        case WATCH_FLOAT:
            m_values[m_value_index].f = *(float*)mp_watch;
            break;
        case WATCH_BOOL:
            m_values[m_value_index].i = *(bool*)mp_watch;
            break;
    }
    // increment the buffer and wrap around
    m_value_index++;                // m_value index will point to the entry AFTER the latest valid entry
    if (m_value_index == max_watch) m_value_index = 0;
    if (m_num_values<max_watch) m_num_values++;       // counts up to maximum.
}

// get the value at a particular index and convert it to 1 or 0
int CWatch::get_1_0_at_index(int index)
{
    switch (m_type)
    {
        case WATCH_INT:
        case WATCH_CHAR:
        case WATCH_BOOL:
            return m_values[index].i == 0 ? 0 : 1;
        case WATCH_FLOAT:
            return m_values[index].f == 0.0f ? 0 : 1;
    }
    return 0;
}


void CWatch::Render(float pos_x, float pos_y, float width, float height, float border, float period, float left_area, float time, float time_off )
{

    float   x = width;
    int index = m_value_index;
    int num = m_num_values;
    
    index--;
    if (index <0) index = max_watch-1;

    float   prev_time = time;
    int     prev_value = get_1_0_at_index(index);

    DrawRect(pos_x-border,pos_y-border, width+border*2 + left_area ,height+border*2,0xffffffff);
    DrawString(pos_x, pos_y, mp_desc, m_color);
    
    while (x > 0.01f && num > 2)
    {
        int index_value = get_1_0_at_index(index);
        float   index_time = m_values[index].time + time_off;                
        x = width - width*(time-prev_time)/period;
        float xp = width - width*(time-index_time)/period;
        if (x<0) x = 0;
        if (xp<0) xp = 0;
        DrawLine(left_area+pos_x+x ,pos_y+height*(1-index_value) , left_area+pos_x+xp , pos_y+height*(1-index_value),m_color);              
        DrawLine(left_area+pos_x+x,pos_y+height*(1-prev_value)     , left_area+pos_x+x    , pos_y+height*(1-index_value),m_color);              
        num--;
        index--;
        if (index <0) index = max_watch-1;
        prev_time = index_time;
        prev_value = index_value;
    }
    

}

// End of CWatch
/////////////////////////////////////////////////////////////////////////////

class   CWatchManager
{
public:    
    CWatchManager();
    void        Update(float time);
    void        Render(float time, bool paused);
    void        SetWatch(float *p, DWORD color, const char*c); 
    void        SetWatch(int *p, DWORD color, const char*c); 
    void        SetWatch(char *p, DWORD color, const char*c);
    void        SetWatch(unsigned char *p, DWORD color, const char*c);
    void        SetWatch(bool *p, DWORD color, const char*c);

    void        RecordEvent(int event, float time, char *desc, DWORD color);

    void        ToggleEventDisplay() {m_display_events = !m_display_events;}
                                    
private:
    CWatch *    add_watch(DWORD color, const char *desc);    
    CWatch *    mp_head;

    struct SWatchEvent{
        int         event;
        float       time;
        char        desc[64];                   //  MEMOPT!!!
        DWORD       color;
        float y;
    };  
    
    const static int   max_watch_event = 1024;
    SWatchEvent     m_events[max_watch_event];        // 80 K
    int             m_event_index;
    int             m_num_events;
    float           m_last_event_time;
    float           m_last_event_y;

    float           m_tick_time;
    float           m_last_tick_time;

    bool            m_display_events;
    
};


CWatchManager           * g_pWatchManager;


CWatchManager::CWatchManager()
{
    mp_head = NULL;
    m_event_index = 0;
    m_num_events = 0;
    m_last_event_time =0;
    m_last_event_y =0.0f;
    m_display_events = true;
}

void        CWatchManager::RecordEvent(int event, float time, char *desc, DWORD color)
{

//    if (event >= 0) debug_log("%4.3f: Event %s",time, desc);

    float   event_y = 12.0f;

    // For the "tick" event, record the length of the tick, for later dispay
    if (event == -1)
    {
        m_tick_time = time - m_last_tick_time;
        m_last_tick_time = time;
    }
    else
    // for regular events
    {
        // alternate the height of the event text if it is close to the previous event    
        if (time - m_last_event_time < 0.50)
        {   
            event_y = 12.0f - m_last_event_y;
        }
        m_last_event_time = time;
        m_last_event_y = event_y;
    }
        
    m_events[m_event_index].event = event;
    m_events[m_event_index].time = time;
    m_events[m_event_index].y = event_y;
    m_events[m_event_index].color = color;
    strncpy(m_events[m_event_index].desc,desc,63);
    m_events[m_event_index].desc[63]='\0';           //  MEMOPT!!! (if you change above...... const it or something!!
    // increment the buffer and wrap around
    m_event_index++;                // m_event index will point to the entry AFTER the latest valid entry
    if (m_event_index == max_watch_event) m_event_index = 0;
    if (m_num_events<max_watch_event) m_num_events++;       // counts up to maximum.

}




void CWatchManager::Render(float time, bool paused)
{
    static float   period = 5.0f;           // period to draw in seconds
    static float   time_off = 0.0f;         // time offset to start drawing from
    
    
    float   height = 10;
    float   pos_x = 0;
    float   start_y = 0;
    float   pos_y = start_y;
    float   border = 3;
    const float left_area = 40;
    float   width = 1024-left_area;

    const   float   max_tick_period = 2.5f;


    if (paused)
    {
        // Zoom in and out, scaling around the center of the window
        if (g_pGamepad->button[CGamepad::BTN_A].Pressed())
        {
            time_off -= period * (1.01f - 1.0f) * 0.5f;
            period *= 1.01f;
        }
        if (g_pGamepad->button[CGamepad::BTN_Y].Pressed())
        {
            time_off -= period * (1.0f - 1.01f) * 0.5f;
            period *= 1.0f/1.01f;
        }
        if (g_pGamepad->button[CGamepad::BTN_L].Pressed())
            time_off += period / 100.0f;
        if (g_pGamepad->button[CGamepad::BTN_R].Pressed())
            time_off -= period / 100.0f;
        
    }
    else
    {
        time_off = 0;
     //   period = 5.0f;
    }

    if (!mp_head)
        return;

// Count how many there are, so we can put the events at the bottom
    int watches = 0;
    CWatch *p_watch = mp_head;
    while (p_watch)
    {
        watches++;
        p_watch=p_watch->mp_next;
    }

    pos_y = start_y + watches * ( height+border*3);

///////////////////////////////
// Now render the events

    float x = width;
    int index = m_event_index;
    int num = m_num_events;

    DrawRect(pos_x-border,pos_y-border, width+border*2 + left_area ,height*2+border*5,0xffffffff);
    if (m_display_events)
    {
        DrawString(pos_x, pos_y, "Events", 0xff000000);
    }
    char c[128];
    if  (period < max_tick_period)
    {
        sprintf(c,"%3.1fms",m_tick_time*1000);
    }
    else
    {
        sprintf(c,"%3.0fms",m_tick_time*10000);
    }
    DrawString(pos_x, pos_y+height+border, c, 0xff000000);
    float last_index_time = 0.0f;
    while (x > 0.01f && num > 0)
    {
        index--;
        if (index <0) index = max_watch_event-1;
        float   abs_index_time = m_events[index].time;
        float   index_time = m_events[index].time+time_off;                
        x = width - width*(time-index_time)/period;
        if (x>0)
        {
            if (m_events[index].event == -1)
            {
                // Draw the dicks as little grey lines at the bottom
                if (period < max_tick_period)
                {
                    DrawLine(left_area+pos_x+x,start_y - border, left_area+pos_x+x , pos_y + height * 3 , 0xfff0f0f0);              
                    DrawLine2(left_area+pos_x+x,pos_y + height , left_area+pos_x+x , pos_y + height * 3 , 0xfff0f0f0, 0xff808080);              
                }
                else if (last_index_time != 0.0f)
                {
                    //  draw the ticks that fall as close to 1/10th as possible
                    //  get prev, round it down to 1/10th, and see if this one is below it
                    // if so, draw it here as a blue line
                    float rounded = (float)((int)(last_index_time*10))/10.0f;
                    if (abs_index_time < rounded)
                    {
                        DrawLine2(left_area+pos_x+x,pos_y-border  , left_area+pos_x+x    , pos_y + height * 3 , 0xffffffff, 0xffe0f0f0);              
                        DrawLine2(left_area+pos_x+x+1,pos_y-border, left_area+pos_x+x+1 , pos_y + height * 3  , 0xffffffff, 0xffe0f0f0);              
                    }
                    
                }
                last_index_time = abs_index_time;
            }
            else
            {
                if (m_display_events)
                {
                    DrawLine(left_area+pos_x+x,start_y , left_area+pos_x+x , pos_y + m_events[index].y,m_events[index].color);              
                    DrawString(left_area+pos_x+x,pos_y + m_events[index].y, m_events[index].desc, m_events[index].color);
                }
            }
        }
        num--;
    }
    
    pos_y=start_y;

    p_watch = mp_head;
    while (p_watch)
    {
        p_watch->Render(pos_x, pos_y, width, height, border, period, left_area, time, time_off);
        pos_y += height+3.0f*border;
        p_watch = p_watch->mp_next;
        // Draw a line between individual watches 
        DrawRect(pos_x-border,pos_y-border*2, width+border*2 + left_area ,border,0xffe0e0e0);
    }
}

void CWatchManager::Update(float time)
{
    CWatch *p_watch = mp_head;
    while (p_watch)
    {
        p_watch->Update(time);
        p_watch = p_watch->mp_next;
    }
}


CWatch  *    CWatchManager::add_watch(DWORD color, const char *desc)
{
    CWatch * p_new_watch = new CWatch();
    p_new_watch->m_color = color;
    p_new_watch->mp_desc = desc;
    if (!mp_head)
    {
        mp_head = p_new_watch;
    }
    else
    {
        CWatch * p_tail = mp_head;
        while (p_tail->mp_next) p_tail = p_tail->mp_next;
        p_tail->mp_next = p_new_watch;
    }
    return p_new_watch;
}
                              
void        CWatchManager::SetWatch(float *p, DWORD color , const char*c)  { add_watch(color,c)->SetWatch(p); } 
void        CWatchManager::SetWatch(int *p, DWORD color, const char*c)     { add_watch(color,c)->SetWatch(p); } 
void        CWatchManager::SetWatch(char *p, DWORD color, const char*c)    { add_watch(color,c)->SetWatch(p); }
void        CWatchManager::SetWatch(unsigned char *p, DWORD color, const char*c)  { add_watch(color,c)->SetWatch(p); }
void        CWatchManager::SetWatch(bool *p, DWORD color, const char*c)  { add_watch(color,c)->SetWatch(p); }




/////////////////////////////////////////////////////////////////////////////////
// The main MX application

CRITICAL_SECTION	debug_CS;


/*

// problem happend before around 00260

all with no threads
	10000			.0051
	12000			.0062
	14000			.0071
	15000			.0078
	16000			.0083
	17000			.0088
	18000			.0095
	20000			.0104
	40000			.0207
	48000			.0246
	50000			.0255
	50250			.0257
	50400			.0260
	50500			.0269, 0.0266 .0273  <<<< ODD
	50550			.0258
	50600			.0262
	51000			.0264
	51500			.0265
	52000			.0270
	52500			.0273
	53000			.0271
	54000			.0279
	80000			.0414

*/

void MX_Init()
{
	InitializeCriticalSection(&debug_CS);
#ifdef		CRITICAL_SECTION_RND
	InitializeCriticalSection(&rnd_CS);
#endif
	g_pWatchManager = new CWatchManager();
    g_pGamepad = new CGamepad();
	g_ParticleManager.Init(NUM_PARTICLES);


	VerletInit();


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

	VerletLogic();


    if (paused)
    {
        g_pGamepad->Update(game_time);

        if (g_pGamepad->button[CGamepad::BTN_R1].Triggered())
        {
            g_pGamepad->button[CGamepad::BTN_R1].ClearTrigger();
            g_pWatchManager->ToggleEventDisplay();
        }
    }
    else
    {
        //game_time += timestep;
        game_time = Timer_Seconds();
        g_pGamepad->Update(game_time);
        g_pWatchManager->Update(game_time);
        //  adding the tick event last, as they are drawn in reverse order and we don't want ot overwrite actual events
        g_pWatchManager->RecordEvent(-1, game_time, "Tick", 0xffc00000);  // Special event to record clock ticks, displayed along the bottom
		g_ParticleManager.Update(timestep);
    }
    
    if (g_pGamepad->button[CGamepad::BTN_START].Triggered())
    {
        g_pGamepad->button[CGamepad::BTN_START].ClearTrigger();
        paused = !paused;
        if (paused)
            Timer_Pause();
        else
            Timer_Resume();
    }

}

void MX_Render()
{
    RenderWorld();
 //   g_pWatchManager->Render(game_time, paused);
//	g_ParticleManager.Render();
	VerletRender();
}

void MX_Cleanup()
{
    delete      g_pWatchManager;
    delete      g_pGamepad;
	VerletCleanup();
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
    	
        case WM_SIZE:
    	{
//    	    resize_db(hWnd);		
//    		InitD3D(hWnd);
            g_resize = true;
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

    if (g_pWatchManager)  // Might not have been initialized yet
    {
        char win_event[1024]; 
        sprintf(win_event, "%x (%d)", msg, wParam);
    
//        g_pWatchManager->RecordEvent(5, Timer_Seconds(), win_event, 0xffc00000);  // Special event to record clock ticks, displayed along the bottom
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}



//-----------------------------------------------------------------------------
// Name: UpdateInputState()
// Desc: Get the input device's state and display it.
//-----------------------------------------------------------------------------
HRESULT UpdateInputState( HWND hDlg )
{
    HRESULT     hr;
    TCHAR       strText[512] = {0}; // Device state text

    if( NULL == g_pJoystick ) 
        return S_OK;

    // Poll the device to read the current state
    hr = g_pJoystick->Poll(); 
    if( FAILED(hr) )  
    {
        // DInput is telling us that the input stream has been
        // interrupted. We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done. We
        // just re-acquire and try again.
        hr = g_pJoystick->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = g_pJoystick->Acquire();

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return S_OK; 
    }

    // Get the input's device state
    if( FAILED( hr = g_pJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &js ) ) )
        return hr; // The device should have been acquired during the Poll()

    // Get the buffered input events

    DWORD     pdwInOut = JOY_BUFFERSIZE;
    g_pJoystick->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), g_inputbuffer, &pdwInOut, 0);         
    DWORD system_ticks = GetTickCount();
    float   game_time = Timer_Seconds();
    float system_time = system_ticks/1000.0f;
    if (pdwInOut != 0)
    {

        //debug_log("Have %d buffered from joy", pdwInOut);

        // Go through them and note them as events so we can see timing

        for (unsigned int i=0;i<pdwInOut;i++)
        {
            float timestamp = (float)g_inputbuffer[i].dwTimeStamp/1000.0f;
          //  debug_log("[%d] = 0x%x at Time = %f, system_time = %f, (%f), gametime = %f",(int)g_inputbuffer[i].dwOfs, (int)g_inputbuffer[i].dwData, timestamp, system_time, timestamp-system_time, Timer_Seconds());

            g_pWatchManager->RecordEvent(6, game_time + (timestamp - system_time), g_inputbuffer[i].dwData?"On":"Off", 0xffc00000);  
        }
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// Simple particle system for testing paralleization schemes

float force_x,force_y;
int force_dir;

void CParticle::Update(float time)
{

	UpdateVerlet(time);
	return;

#ifndef PRETTY_UP
	
	float a = 1.1f;
#if 0
	// some extra CPU intensive processing
	for (int x=0;x<100;x++)
		a *= 0.999f;
	return;
#endif


	m_old_pos = m_pos;
	Vector2 pos2 = m_pos + m_vel * time;
	
	// acc toward mouse pos
	if (1 || dragging)
	{

		float x = force_x; 
		float y = force_y; 
		if (dragging)
		{
			x = (float)mouse_x;
			y = (float)mouse_y;
		}

		Vector2 to = m_pos - Vector2(x,y);
		float force = 1000.0f/to.Length2();
		if (!dragging)
		{
			m_vel += to * force;
			// friction
			m_vel -= m_vel * 0.03f;
		}
		else
		{
			m_vel -= to * force * 5.0f;
			// friction
			m_vel -= m_vel * 0.002f;
		}

	}
	else
	{
		// bit of randomness
		m_vel += Vector2(rnd(10000)/1000.0f-5.0f,rnd(10000)/1000.0f-5.0f);
		// bit of gravity
		m_vel += Vector2(0.0f,10.0f);
		// friction
		m_vel -= m_vel * 0.02f;
	}

	m_color = 0xff800000 + (int)m_vel.Length() + (int)a;

	CWhisker whisker;
	if (whisker.CheckCollision(m_pos, pos2))
	{
		// Move to the collision point, and raise slightly off the surface
		m_pos = whisker.GetPoint() + 0.01f * whisker.GetNormal();
		// reflect the velocity in the plane of the surface
		m_vel = m_vel - 2.0f * DotProduct((m_vel),whisker.GetNormal()) * whisker.GetNormal();
		//m_vel = -m_vel;
	}
	else
	{
		m_pos = pos2;
	}
#else




	float a = 1.1f;
#if 0
	// some extra CPU intensive processing
	for (int x=0;x<100;x++)
		a *= 0.999f;
	return;
#endif


	m_old_pos = m_pos;
	Vector2 pos2 = m_pos + m_vel * time;
	
#if 1

	// acc toward mouse pos
	if (dragging)
	{

		float x = force_x; 
		float y = force_y; 
		if (dragging)
		{
			x = (float)mouse_x;
			y = (float)mouse_y;
		}

		Vector2 to = m_pos - Vector2(x,y);
		float len2 = to.Length2();
		if (len2> 0.1f)
		{
			//float force = 1000.0f/len2;
			float force = 0.01f;
			//force = 1.0f;
			m_vel -= to.Normal() * force * 50.0f;
			// friction
			m_vel -= m_vel * 0.02f;
		}


	}
#if 0
	else
	{
		// bit of randomness
		m_vel += Vector2(rnd(10000)/1000.0f-5.0f,rnd(10000)/1000.0f-5.0f);
		// bit of gravity
		m_vel += Vector2(0.0f,10.0f);
		// friction
		m_vel -= m_vel * 0.02f;
	}
#endif

#endif

#ifdef	NBODY

		CParticleManager *ppm = &g_ParticleManager;
		int n = ppm->m_num_particles;
		CParticle *pp = ppm->mp_particles; 
		Vector2 my_pos = m_pos;
		Vector2 all_vel = Vector2(0.0f,0.0f);
		for (int i=0;i<n;i++)
		{
			all_vel += pp[i].m_vel;
			if (&pp[i] != this)
			{


				Vector2 to = pp[i].m_pos - my_pos;
				float len2 = to.Length2();

#define	FLUID_MIN	(20.0f)

				if (len2 < FLUID_MIN * FLUID_MIN)
				{
					pp[i].m_pos = my_pos + to.Normal() * FLUID_MIN;
				}

				float len = sqrtf(len2);
				if (len > 0.001f)
				{
					// Attractive force propotional to distance
					m_vel += len * to.Normal() * 0.0001f;
					//m_vel +=  to.Normal() / len * 10.0f;



					// repulsive force inversely proportional to distance squared
					m_vel -= to.Normal() * 1.0f / len2 * 100.0f;


					//m_vel += to.Normal() * 0.1;
				}
			}
		}
		
		
		//m_vel -= m_vel * 0.002f;
		
		// inverse square friction

		if (m_vel.Length2() >0.001)
		{
	//		m_vel -= m_vel.Normal() * m_vel.Length2() * 0.0000002f;
		}

		// inverse square friction that is not parallel and in same direction as overall velocity of other points
		all_vel /= (float)n;		// get average velocity of the group
/*
		// get direction of the group
		Vector2 all_dir = all_vel.Normal();

		// get component of velocity in this direction

		float my_group_speed = DotProduct(m_vel,all_dir);
		if (my_group_speed < 0)
		{
			// if going backwards against the group, then will be full friction
			my_group_speed = 0;
		}

		// subtract my velocity in the direction of the group
		Vector2 my_perp_vel = m_vel - my_group_speed * all_dir;

		// apply friction to the remainder
		my_perp_vel -= my_perp_vel * 0.1;

		// add this back to velocity in the group direction
		m_vel = my_perp_vel + my_group_speed * all_dir;
*/



#endif



	//m_color = 0xffff0000 + (int)m_vel.Length() + ((int)(255.0f*(m_vel.Normal().x))<<8);
	//m_color = 0xffff0000 + ((int)(255.0f*(m_vel.Normal().y))) + ((int)(255.0f*(m_vel.Normal().x))<<8);
	//m_color = 0xffff0000 + ((int)(127.0f*(m_vel.Normal().y+1.0f))) + ((int)(127.0f*(m_vel.Normal().x+1.0f))<<8);
	m_color = 0xff000000 + ((int)(127.0f*(m_vel.Normal().y+1.0f))) + ((int)(127.0f*(m_vel.Normal().x+1.0f))<<8) + ((int)(127.0f*(m_vel.Normal().y * m_vel.Normal().x+1.0f))<<16) ;

	CWhisker whisker;
	if (whisker.CheckCollision(m_pos, pos2))
	{
		// Move to the collision point, and raise slightly off the surface
		m_pos = whisker.GetPoint() + 0.01f * whisker.GetNormal();
		// reflect the velocity in the plane of the surface
		m_vel = m_vel - 2.0f * DotProduct((m_vel),whisker.GetNormal()) * whisker.GetNormal();
		//m_vel = -m_vel;
	}
	else
	{
		m_pos = pos2;
	}

#endif

}

void CParticle::UpdateVerlet(float time)
{


	// verlet integration just uses the position and the old position

	
	//	Vector2 pos2 = m_pos + m_vel * time;
	

	Vector2	acc = Vector2(0,0);

	// acc toward mouse pos
	if (dragging)
	{

		float x = (float)mouse_x;
		float y = (float)mouse_y;

		Vector2 to = m_pos - Vector2(x,y);
		float force = 10.0f/to.Length2();
		acc = -to.Normal() * force;

	}


	// x(t0 + dt) = 2x(t0) − x(t0 − dt) + adt^2 
	Vector2 last_pos = m_pos;
	
	// verlet integration
	m_pos = 2 * m_pos - m_old_pos + acc;
	
	
	// Satisfy collision constraints
	CWhisker whisker;
	if (whisker.CheckCollision(last_pos, m_pos))
	{
		// Move to the collision point, and raise slightly off the surface
		m_pos = whisker.GetPoint() + 0.01f * whisker.GetNormal();
	}

	m_old_pos = last_pos;



	m_color = 0xff800000 + (int)m_vel.Length();

	/*
	CWhisker whisker;
	if (whisker.CheckCollision(m_pos, pos2))
	{
		// Move to the collision point, and raise slightly off the surface
		m_pos = whisker.GetPoint() + 0.01f * whisker.GetNormal();
		// reflect the velocity in the plane of the surface
		m_vel = m_vel - 2.0f * DotProduct((m_vel),whisker.GetNormal()) * whisker.GetNormal();
		//m_vel = -m_vel;
	}
	else
	{
		m_pos = pos2;
	}
*/

	//m_color = 0xffff0000 + (int)m_vel.Length() + ((int)(255.0f*(m_vel.Normal().x))<<8);
	//m_color = 0xffff0000 + ((int)(255.0f*(m_vel.Normal().y))) + ((int)(255.0f*(m_vel.Normal().x))<<8);
	//m_color = 0xffff0000 + ((int)(127.0f*(m_vel.Normal().y+1.0f))) + ((int)(127.0f*(m_vel.Normal().x+1.0f))<<8);
	m_color = 0xff000000 + ((int)(127.0f*(m_vel.Normal().y+1.0f))) + ((int)(127.0f*(m_vel.Normal().x+1.0f))<<8) + ((int)(127.0f*(m_vel.Normal().y * m_vel.Normal().x+1.0f))<<16) ;
/*
	CWhisker whisker;
	if (whisker.CheckCollision(m_pos, pos2))
	{
		// Move to the collision point, and raise slightly off the surface
		m_pos = whisker.GetPoint() + 0.01f * whisker.GetNormal();
		// reflect the velocity in the plane of the surface
		m_vel = m_vel - 2.0f * DotProduct((m_vel),whisker.GetNormal()) * whisker.GetNormal();
		//m_vel = -m_vel;
	}
	else
	{
		m_pos = pos2;
	}
*/


	m_color = 0xffff00ff;


}





CParticleManager::CParticleManager()
{
	mp_particles = NULL;
	m_num_particles = 0;
}

CParticleManager::~CParticleManager()
{
	if (mp_particles)
		delete [] mp_particles;
}

// Release, forked threads, no physics
// 0		0.0214 0.0331
// 1		0.0226 0.0343
// 2		0.0132 0.0250
// 3		0.0102 0.0218
// 4		0.0096 0.0209

// Release Mode, Worker threads no affinity, no physics
// 0		0.0214 0.0331
// 1		0.0214 0.0331
// 2		0.0117 0.0234
// 3		0.0091 0.0207
// 4		0.0071 0.0189

// Overclocked MB to 3.6 Ghz
// 4		0.0063 0.0177 << YAY!!!

// Release Mode, worker, Dual Core, HT Disabled
// 0		0.0214 0.0333
// 1		0.0211 0.0319
// 2		0.0108 0.0226  <<< best
// 3		0.0136 0.0254
// 4		0.0111 0.0228

// Single Core, HT enabled
// 0		0.0219 0.0340
// 1		0.0215 0.0336
// 2		0.0141 0.0260
// 3		0.0157 0.0276
// 4		0.0144 0.0263

// Single Core, HT disabled  (all times are essentially the same)
// 0		0.0219 0.0341
// 1		0.0223 0.0343
// 2		0.0221 0.0341
// 3		0.0220 0.0339
// 4		0.0221 0.0339

// Release Mode, no affinity, yes physics
// 0		0.0238  0.0266 0.0400
// 1		0.0251  247, 383   (252, 393) (254, 381) <<< slower!! (oscillates)
// 2		158, 301
// 3		119,251
// 4		110,243

// Release Optimized
// Optimizations used:
// Whole program Optimization: Link time code generation
// Floating point Model:  Fast
// 0		0.0182, 0.0282
// 1		0.0121,	0.0221
// 2		0.0069, 0.0168
// 3		0.0056, 0.0156
// 4		0.0046, 0.0144
//
// As above with Enhanced instruction set: streaming Simd2
//
// 0		0.0095, 0.0195
// 1		0.0095, 0.0195
// 2		0.0056, 0.0156
// 3		0.0044, 0.0146
// 4		0.0036, 0.0138
//
// And with inline any suitable
// 1		0.0080, 0.0164
// 2		0.0046, 0.0133
// 3		0.0036,	0.0122
// 4		0.0030, 0.0116
//
// Overclocked MB to 3.6 Ghz
// 4		0.0026, 0.0109

////////////////////////////////////////////////////////////////////////

// Release Mode, set affinity, no physics
// 0		0.0214
// 1		0.0217
// 2		0.0110
// 3		0.0093
// 4		0.0072

// Single core 1.5Ghz machine, worker threads, no physics
// 0		0.0790 0.1019

// 1		0.0454 0.0673
// 2		0.0426 0.0654
// 3		0.0429 0.0663 (highly variable)
// 4		0.0422 0.0646


// Single core, optimized version
// 0		0.0176, 0.0365
// 1		0.0176, 0.0366
// 4		0.0188, 0.0373

// Single Core, optimized, but no SIMD2
// 0		0.1022, 0.1207


// with physics

// 0		0.1565, 0.1997
// 1		0.0832, 0.1118
// 2		0.0664, 0.0929
// 3		0.0554	0.0925
// 4		0.0546	0.0891

// 1.5 Ghz Celeron laptop, worker, no phys
//
// 0		0.0428, 0.0727
// 1		0.0345, 0.0648


///////////////////////////////////////////////////////////////////////
// DEBUG mode
// New system - Setting affinity to 1<<t
// 0		0.0417  100
// 1		0.0417	100
// 2		0.0214  51.3%	Fastest 2 thread, as 
// 3		0.0243  56.1%	<<< slower with 3, task cannot be divided evenly between 3 procs with affinity set
// 4		0.0187	44.8%

// 5		0.0240
// 6		0.0209
// 7		0.0218
// 8		0.0188
// 16		0.0199

// New System - not setting affinity, batched processing
// 0		0.0417  (consistenet)
// 1		0.0417  (consistent)
// 2		0.0248	59.4%	,		0.0239, 0.0272, 0.0250, 0.0243  0.0247, 0.250, 0.240
// 3		0.0225  53.9%	0.0225 
// 4		0.0184  44.1%	(consistent) (fastest because other task are now evenly spread)

// New System, not set affinity, interleaved processing
// 1		0.0417
// 2		0.0256	61.3%
// 3		0.0227	54.4%
// 4		0.0191	45.8%





HANDLE	thStartThreading[MAX_THREAD];
HANDLE	thStopThreading[MAX_THREAD];
HANDLE	thThread[MAX_THREAD];

float update_time;

DWORD WINAPI thParticleUpdate(LPVOID p)
{
	int thread = *(int*)p;
#ifndef	USE_FORKED_THREADS
	while (1)
	{
		WaitForSingleObject(thStartThreading[thread],INFINITE);
#endif
		float time = update_time;
		int n = g_ParticleManager.m_num_particles;
		int step = n / MAX_THREAD;
		int start = step*thread;
		int end = step*(thread+1);
		if (thread == MAX_THREAD-1)
			end = n;

#ifdef MANUAL_OPT
		CParticle *p = &g_ParticleManager.mp_particles[start];
		for (int i=start;i<end;i++)
		{
			p->Update(time);
			p++;
		}
#else
// 
 #ifdef	PROCESS_CHUNKS	// Process in chunks
		for (int i=start;i<end;i++)
 #else	// Process interleaved
		for (int i=thread;i<n;i+=MAX_THREAD)
 #endif
		{
			g_ParticleManager.mp_particles[i].Update(time);
		}

#endif



#ifndef	USE_FORKED_THREADS
		SetEvent(thStopThreading[thread]);
	}
#endif
	return 0;
}


float dummy_a = 1.1f;;
DWORD WINAPI DummyThread(LPVOID p)
{
	while (dummy_a != 0)
		dummy_a *= 1.00001f;
	return 0;
}

DWORD WINAPI NullThread(LPVOID p)
{
	return 0;
}


int	ThreadIndex[MAX_THREAD];

Vector2	drift;
Vector2	driftv;
Vector2	drifta;


void CParticleManager::Init(int n)
{
	drift = Vector2(g_viewport_width/2.0f, g_viewport_height/2.0f);
	driftv = Vector2(0,0);
	drifta = Vector2(0,0);
	// Create the Particles
	if (mp_particles)
		delete [] mp_particles;
	mp_particles = new CParticle[n];
	m_num_particles = n;
	for (int i = 0;i<n;i++)
	{
		mp_particles[i].m_pos = Vector2(g_viewport_width/2.0f, g_viewport_height/2.0f) + Vector2((float) rnd(200)-400, (float)rnd(200)-400);
		mp_particles[i].m_old_pos = mp_particles[i].m_pos - Vector2(0.03f,0.03f);
		mp_particles[i].m_color = 0xff00ff00; // default magenta for un-updated particles
		//mp_particles[i].m_vel = Vector2((float)(rnd(10000)-5000),(float)(rnd(10000)-5000));
		mp_particles[i].m_vel = Vector2(0.001f,0.001f);
		mp_particles[i].m_vel = mp_particles[i].m_vel.Normal();
		mp_particles[i].m_vel = mp_particles[i].m_vel * (20.0f + rnd(10000)/1000.0f);
	}

#ifdef	USE_WORKER_THREADS
		DWORD	threadId;
	// Create the threads and events
	for (int t=0;t<MAX_THREAD;t++)
	{
		ThreadIndex[t] = t;
		thStartThreading[t] = CreateEvent(NULL, FALSE, FALSE, NULL);
		thStopThreading[t]  = CreateEvent(NULL, FALSE, FALSE, NULL);
		thThread[t] = CreateThread(NULL,0,thParticleUpdate,(LPVOID)&ThreadIndex[t],0,&threadId);
		SetThreadPriority(thThread[t],0);		// 0 = Normal (default)
#ifdef	SET_AFFINITY
		SetThreadAffinityMask(thThread[t],1<<t);
#endif
	}
#endif

#ifdef	USE_DUMMY_PHYSICS
		int	xx;
		DWORD	threadId2;
		HANDLE dt = CreateThread(NULL,0,DummyThread,(LPVOID)&xx,0,&threadId2);
	//	SetThreadPriority(dt,-2);  // below normal

#endif
}



// Call the particle update function, either directly
// or by starting the thread(s) and then waiting for them to finish
void CParticleManager::Update(float time)
{
	#define	SAMPLES 200
	static float avg[SAMPLES];
	static int avgn = 0;
	static float favg[SAMPLES];
	static int favgn = 0;
	static int samples = -50;

	
	if (rnd(10) == 0)
	{
	//	force_x = (float)rnd(g_viewport_width);
	//	force_y = (float)rnd(g_viewport_height);
	//	force_dir = !rnd(10);
	}
	

	force_x = drift.x;
	force_y = drift.y;

	drift += driftv;
	if (drift.x<0.0f && driftv.x<0.0f) driftv.x = -driftv.x; 
	if (drift.y<0.0f && driftv.y<0.0f) driftv.y = -driftv.y; 
	if (drift.x>g_viewport_width && driftv.x>0.0f) driftv.x = -driftv.x; 
	if (drift.y>g_viewport_height && driftv.y>0.0f) driftv.y = -driftv.y; 

	// Occasionally change acceleration
	if (rnd(5) == 0)
	{
		drifta = Vector2((float)(rnd(200)-100)/200.0f,(float)(rnd(200)-100)/200.0f) * 10.0f;
	}
	driftv += drifta;
	driftv *= 0.9f;

	// Occasionally stop
	if (rnd(20) == 0)
	{
		drifta = Vector2(0,0);
		driftv = Vector2(0,0);
	}


	float	duration = Timer_Seconds();
	update_time = time;
	update_time = 0.02f;

#ifndef	USE_WORKER_THREADS
	#ifndef	USE_FORKED_THREADS
		// Don't use worker threads just iterate over the particle list (in the main thread)
		int n = m_num_particles;
		for (int i=0;i<n;i++)
		{
			mp_particles[i].Update(update_time);
		}
	#else
		// Use forked threads, create and start them
		DWORD	threadId;
		for (int t=0;t<MAX_THREAD;t++)
		{
			ThreadIndex[t] = t;
			thThread[t] = CreateThread(NULL,0,thParticleUpdate,(LPVOID)&ThreadIndex[t],0,&threadId);
		}
		// Wait for them to terminate
		WaitForMultipleObjects(MAX_THREAD,&thThread[0],true, INFINITE);
		// and delete the handles
		for (int t=0;t<MAX_THREAD;t++)
		{
			CloseHandle(thThread[t]);
		}
	#endif
#else
// Multithreaded - flag the worker threads to start running
	for (int t=0;t<MAX_THREAD;t++)
		SetEvent(thStartThreading[t]);
// Then wait for them all to finish
	WaitForMultipleObjects(MAX_THREAD,&thStopThreading[0],true, INFINITE);
#endif
	duration -= Timer_Seconds();

#ifdef	USE_FORK_MARKING
	int	xx;
	DWORD	threadId;
	HANDLE nt = CreateThread(NULL,0,NullThread,(LPVOID)&xx,0,&threadId);
	WaitForSingleObject(nt,INFINITE);
	CloseHandle(nt);
#endif

	static float	frame_last_time = 0.0f;
	float	frame_length = Timer_Seconds() - frame_last_time;
	frame_last_time = Timer_Seconds();
	float a=0;
	float fa=0;
	if (samples<SAMPLES) samples++;
	if (samples >0)
	{
		if (avgn < SAMPLES) 
		{
			avg[avgn]=duration;
			favg[avgn]=frame_length;
			avgn++;
		}

		for (int i=0;i<avgn;i++)
		{
			a+=avg[i];
			fa+=favg[i];
		}
		a /= (float) avgn;
		fa /= (float) avgn;
	}
	char buf[512];
	//sprintf(buf, "avg = %.6f duration = %.6f   frame avg = %.6f  len = %.6f samples = %d",a,duration,fa,frame_length, samples);
	//debug_log("%s",buf);
	//DrawString(0,0,buf,0xffffffff);
	sprintf(buf, "Arrow keys = move, Space = Jump, left mouse = drag, S = Slippy, A = not slippy");
	DrawString(16,16,buf,0xff202020);
}

void CParticleManager::Render()
{
	for (int i=0;i<m_num_particles;i++)
	{
		Vector2 back = mp_particles[i].m_old_pos;
#ifdef	PRETTY_UP
		if ((mp_particles[i].m_pos-back).Length() < 2.5f)
			back += (mp_particles[i].m_pos-back).Normal()*2.5f;
#endif
		DrawLine(back, mp_particles[i].m_pos, mp_particles[i].m_color);		
	}
}

////////////////////////////////////////////////////////////////////
// VERLET BLOB SYSTEM STARTS HERE
// VERLET BLOB SYSTEM STARTS HERE
// VERLET BLOB SYSTEM STARTS HERE
// VERLET BLOB SYSTEM STARTS HERE
// VERLET BLOB SYSTEM STARTS HERE

CVerletSystem	gVerletSystem;

// local collision constraint
// Note this exists outside of verlet.cpp/h, it's derieved from the abstract base constraint

class CVerletWorldCollision : public CVerletConstraint
{
	virtual void	Satisfy(CVerletPoint* p_verlet);	
	virtual Vector2	GetForce(CVerletPoint* p_verlet) {return Vector2(0,0);};	
	virtual	void	debug_render(CVerletPoint* p_verlet) {};
};

void	CVerletWorldCollision::Satisfy(CVerletPoint* p_verlet)
{

	// Satisfy collision constraints
	CWhisker whisker;
	if (whisker.CheckCollision(p_verlet->GetLastPos(), p_verlet->GetPos()))
	{
		// Collision point, for later emergency usage
		Vector2 collision = whisker.GetPoint();
		// The attempted movment of the point
		Vector2	movement = p_verlet->GetPos() - p_verlet->GetLastPos();
		// normal of the surface that collided with
		Vector2 normal = whisker.GetNormal();
		// magnitude of the component of motion perpendicular to the surface
		float perp = DotProduct(movement, normal);
		// project vector onto the surface
		Vector2 parallel = movement - normal*perp;

		DrawLine(p_verlet->GetLastPos(), p_verlet->GetLastPos() + parallel * 20, 0xffffffff);

		if (!whisker.CheckCollision(p_verlet->GetLastPos(), p_verlet->GetLastPos() + parallel))
		{
			p_verlet->SetPos(p_verlet->GetLastPos() + parallel * g_friction);
			//p_verlet->SetPos(p_verlet->GetLastPos() + parallel);
		}
		else
		{
			// worse case - Move to the collision point, and raise slightly off the surface
			p_verlet->SetPos(collision + 0.01f * normal);
		}
	}
}

class CVerletWorldLineCollision : public CVerletConstraint
{
public:
	CVerletWorldLineCollision(CVerletPoint* p_other) {mp_other = p_other;}
	virtual void	Satisfy(CVerletPoint* p_verlet);	
	virtual Vector2	GetForce(CVerletPoint* p_verlet) {return Vector2(0,0);};	
	virtual	void	debug_render(CVerletPoint* p_verlet) {};
private:
	CVerletPoint*	mp_other;
};

void	CVerletWorldLineCollision::Satisfy(CVerletPoint* p_verlet)
{
	// Ensure the line does not intersect geometry
	CWhisker whisker;
	if (whisker.CheckCollision(p_verlet->GetPos(), mp_other->GetPos()))
	{
#if 0
		Vector2 a = p_verlet->GetPos();
		Vector2 b = mp_other->GetPos();
		Vector2 out = p_verlet->GetLastPos() - p_verlet->GetPos();
		float add = 0.5f;
		for (int i=0;i<5;i++)
		{
			// This is probably not working too weel as a way of getting lines to swing
			// around corners
			// but it's better that just stopping things

			// NEED TO TEST THIS TO SEE WHAT'S GOING ON!!!

			// go half way there
			a += add * out;
			b += add * out;
			add *= 0.5f;
			// if there is no collision, then we need to go -ve, else +ve
			if (whisker.CheckCollision(a,b))
			{
				if (add<0.0f) add = -add;
			}
			else
			{
				if (add>0.0f) add = -add;
			}
		}
		

		if (add > 0)
		{
		// worst case - just set both verlet back to the last position
		p_verlet->SetPos(p_verlet->GetLastPos());
		mp_other->SetPos(mp_other->GetLastPos());
		}
		else
		{
			p_verlet->SetPos(a);
			mp_other->SetPos(b);
		}
#else

		// Both verlets points have independ motion
		// which together has caused this line/world collision
		// since point collisions have already been resolved
		// it is unlikely that the individual point penetrate the world
		// so attemptin to move the original line segment parallel to itself
		// will likely be safe
		// and will allow the segments to slide over corners

		Vector2 a = p_verlet->GetLastPos();
		Vector2 b = mp_other->GetLastPos();
		Vector2 av = p_verlet->GetPos() - a;
		Vector2 bv = mp_other->GetPos() - b;
		Vector2 p = (a-b).Normal(); // normal vector from a to b
		// Project a and b onto p
		Vector2 ap = p * DotProduct(av,p);
		Vector2 bp = p * DotProduct(bv,p);
		Vector2 a2 = a + ap;
		Vector2 b2 = b + bp;
		// Might also need to check a to a2 and b to b2
		if (!whisker.CheckCollision(a2,b2) && !whisker.CheckCollision(a,a2) && !whisker.CheckCollision(b,b2))
		{
			p_verlet->SetPos(a2);
			mp_other->SetPos(b2);
		}
		else
		{
			p_verlet->SetPos(p_verlet->GetLastPos());
			mp_other->SetPos(mp_other->GetLastPos());
		}

#endif


	}
}


#define PI 3.141592654f
#define MAX_BLOB_SEGMENTS 1000
// create a spherical blob with a given number of segments
// This is the simplest blob, which is intended to reach equilibrium
/// with all the springs at zero tension
void CreateVerletBlob( int segments, float x, float y, float min, float mid, float max, float force)
{
//	ASSERT(segments < MAX_BLOB_SEGMENTS)
	float angle_step = 2.0f * PI / (float) segments;
	float segment_length = 2.0f * mid * sinf(angle_step/2.0f);
	CVerletPoint* p_v[MAX_BLOB_SEGMENTS];
	// Create midpoint
	CVerletPoint* p_mid = gVerletSystem.CreatePoint(Vector2(x,y));

	// Give midpoint a mass of all the other points put together
	p_mid->SetMass((float)segments);

	// create outer point (which lie on a circle)
	for (int i=0;i<segments;i++)
	{
		float angle = i * angle_step;
		float bx = x + mid * cosf(angle);
		float by = y + mid * sinf(angle);
		p_v[i] = gVerletSystem.CreatePoint(Vector2(bx,by));
	}

	// link up with constraints
	for (int i=0;i<segments;i++)
	{
		int next = (i+1)%segments;
		// to next point
//		RigidConstraint(p_v[i],p_v[next],segment_length);
		SemiRigidConstraint(p_v[i],p_v[next],segment_length*0.9f,segment_length,segment_length*1.1f,force);
		// to center point
		SemiRigidConstraint(p_v[i],p_mid,min,mid,max,force);
		// with world point collision
		p_v[i]->AddCollisionConstraint(new CVerletWorldCollision);	
		// and world line collision
		p_v[i]->AddCollision2Constraint(new CVerletWorldLineCollision(p_v[next]));	
	}



}


// Same, but extra braces on ever other point
void CreateBracedBlob( int segments, float x, float y, float min, float mid, float max, float force)
{
//	ASSERT(segments < MAX_BLOB_SEGMENTS)
	float angle_step = 2.0f * PI / (float) segments;
	float segment_length = 2.0f * mid * sinf(angle_step/2.0f);
	float segment2_length = 2.0f * mid * sinf(angle_step*2.0f/2.0f);
	float segment3_length = 2.0f * mid * sinf(angle_step*3.0f/2.0f);
	CVerletPoint* p_v[MAX_BLOB_SEGMENTS];
	// Create midpoint
	CVerletPoint* p_mid = gVerletSystem.CreatePoint(Vector2(x,y));

	// Give midpoint a mass of all the other points put together
	p_mid->SetMass((float)segments);

	// create outer point (which lie on a circle)
	for (int i=0;i<segments;i++)
	{
		float angle = i * angle_step;
		float bx = x + mid * cosf(angle);
		float by = y + mid * sinf(angle);
		p_v[i] = gVerletSystem.CreatePoint(Vector2(bx,by));
	}

	// link up with constraints
	for (int i=0;i<segments;i++)
	{
		int next = (i+1)%segments;
		int next2 = (i+3)%segments;
		// to next point
//		RigidConstraint(p_v[i],p_v[next],segment_length);
		SemiRigidConstraint(p_v[i],p_v[next],segment_length*0.1f,segment_length,segment_length*2.1f,force);
		SemiRigidConstraint(p_v[i],p_v[next2],segment3_length*0.1f,segment3_length,segment3_length*2.1f,force);
		// to center point
		SemiRigidConstraint(p_v[i],p_mid,min,mid*2,max*2,force);
		// with world point collision
		p_v[i]->AddCollisionConstraint(new CVerletWorldCollision);	
		// and world line collision
		p_v[i]->AddCollision2Constraint(new CVerletWorldLineCollision(p_v[next]));	
	}



}




// a blob under pressure
void CreatePressureBlob( int segments, float x, float y, float min, float mid, float max, float force)
{
//	ASSERT(segments < MAX_BLOB_SEGMENTS)
	float angle_step = 2.0f * PI / (float) segments;
	float segment_length = 2.0f * mid * sinf(angle_step/2.0f);
	CVerletPoint* p_v[MAX_BLOB_SEGMENTS];
	// Create midpoint
	CVerletPoint* p_mid = gVerletSystem.CreatePoint(Vector2(x,y));

	// Give midpoint a mass of all the other points put together
	p_mid->SetMass((float)segments);

	// create outer point (which lie on a circle)
	for (int i=0;i<segments;i++)
	{
		float angle = i * angle_step;
		float bx = x + mid * cosf(angle);
		float by = y + mid * sinf(angle);
		p_v[i] = gVerletSystem.CreatePoint(Vector2(bx,by));
	}

	// link up with constraints
	for (int i=0;i<segments;i++)
	{
		int next = (i+1)%segments;
		// to next point
//		RigidConstraint(p_v[i],p_v[next],segment_length);
		SemiRigidConstraint(p_v[i],p_v[next],segment_length*0.9f,segment_length,segment_length*2.0f,force*10);
		// to center point
		// Patch *2 for internal pressure
		SemiRigidConstraint(p_v[i],p_mid,min,mid*1.5,max*2,force);
		// with world point collision
		p_v[i]->AddCollisionConstraint(new CVerletWorldCollision);	
		// and world line collision
		p_v[i]->AddCollision2Constraint(new CVerletWorldLineCollision(p_v[next]));	
	}
}

// create a blob that has a thick structural skin
void CreateSkinnedBlob( int segments, float x, float y, float inner, float outer, float force, float inner_force)
{
//	ASSERT(segments < MAX_BLOB_SEGMENTS)
	float angle_step = 2.0f * PI / (float) segments;
	float outer_segment_length = 2.0f * outer * sinf(angle_step/2.0f);
	float inner_segment_length = 2.0f * inner * sinf(angle_step/2.0f);
	float ring_gap = outer-inner;
	CVerletPoint* p_v[MAX_BLOB_SEGMENTS];
	// Create midpoint
	CVerletPoint* p_mid = gVerletSystem.CreatePoint(Vector2(x,y));
	// Give midpoint a mass of all the other points put together
	//p_mid->SetMass((float)segments);

	// create outer point (which lie on a circle)
	for (int i=0;i<segments;i++)
	{
		float angle = i * angle_step;
		float bx = x + inner * cosf(angle);
		float by = y + inner * sinf(angle);
		float cx = x + outer * cosf(angle);
		float cy = y + outer * sinf(angle);
		p_v[i*2] = gVerletSystem.CreatePoint(Vector2(cx,cy)); // i*2 is outer
		p_v[i*2+1] = gVerletSystem.CreatePoint(Vector2(bx,by)); // i*2+1 is inner
	}

	// link up with constraints
	for (int i=0;i<segments;i++)
	{
		int next = (i+1)%segments;
		// to next point
//		RigidConstraint(p_v[i],p_v[next],segment_length);
		// outer ring
		//RigidConstraint(p_v[i*2],p_v[next*2],outer_segment_length);
		SemiRigidConstraint(p_v[i*2],p_v[next*2],outer_segment_length*0.9f,outer_segment_length,outer_segment_length*1.1f,0);
		/// inner ring
		SemiRigidConstraint(p_v[i*2+1],p_v[next*2+1],inner_segment_length*0.9f,inner_segment_length,inner_segment_length*1.1f,force);
		// joinn rings with structural springs
		SemiRigidConstraint(p_v[i*2],p_v[i*2+1],ring_gap*0.9f,ring_gap,ring_gap*1.1f,force);
		// and a cross-brace
		SemiRigidConstraint(p_v[i*2],p_v[next*2+1],ring_gap*0.9f,ring_gap,ring_gap*1.1f,force);
		
		// inner ring to center point
		// setting the mid point of the spring to be greater than
		// the inner radius gives teh structure some internal pressure
		// and a more blob-like shape
		//SemiRigidConstraint(p_v[i*2+1],p_mid,inner*.1,inner*1.1,inner*2.1,force);
		//SemiRigidConstraint(p_v[i*2+1],p_mid,inner*.1,inner*1.5,inner*2.1,force);
		SemiRigidConstraint(p_v[i*2+1],p_mid,inner*.2,inner*1.5,inner*2.1,inner_force);
		// with world point collision
		p_v[i*2]->AddCollisionConstraint(new CVerletWorldCollision);	
		// and world line collision
		p_v[i*2]->AddCollision2Constraint(new CVerletWorldLineCollision(p_v[next*2]));	
		
		//p_v[i*2+1]->AddCollisionConstraint(new CVerletWorldCollision);	
		// and world line collision
		//p_v[i*2+1]->AddCollision2Constraint(new CVerletWorldLineCollision(p_v[next]));	
	}



}



void	VerletInit()
{
	
	// A single line with two points, for testing

//	CVerletPoint* p1 = gVerletSystem.CreatePoint(Vector2(300,300));
//	CVerletPoint* p2 = gVerletSystem.CreatePoint(Vector2(400,300));
//	SemiRigidConstraint(p1,p2,80,100,120,30);


//	CreateVerletBlob( 10, 300,200, 48, 50, 52, 10);  
//	CreateVerletBlob( 5, 400,200, 10, 50, 150, 30);  
//	CreateVerletBlob( 3, 500,200, 50, 50, 50, 30);  
//	CreateVerletBlob( 4, 600,200, 50, 50, 50, 30);  
//	CreateVerletBlob( 4, 400,200, 40, 50, 60, 30);  
	//CreateVerletBlob( 40, 500,200, 10, 50, 60, 100);  
	//CreatePressureBlob( 40, 400,200, 10, 50, 60, 100);  
	//CreateSkinnedBlob( 4, 400,200, 50, 55, 10);  // inner, outer, force 
//	CreateVerletBlob( 20, 500,200, 10, 50, 60, 50);  
//	CreateVerletBlob( 20, 600,200, 10, 50, 60, 1);  


#ifdef FIGURES

//	CreateVerletBlob( 20, 500,400, 250, 300, 360, 10);  
	CreateBracedBlob( 20, 500,400, 150, 180, 220, 10);  
//	CreatePressureBlob( 40, 400,200, 100, 150, 160, 10);  


//CreateSkinnedBlob( 40, 500,400, 250, 300, 2,200);  // inner, outer, force 

//	CreateSkinnedBlob( 80, 500,300, 180, 220, 10,2);  // inner, outer, force 

#else

	CreateVerletBlob( 40, 400,100, 20, 50, 80, 20);  
	CreateBracedBlob( 40, 400,100, 20, 30, 35, 20);  

//                   segs,   x,  y, r1, r2, f1, f2
	CreateSkinnedBlob( 40, 500,100, 50, 60, 10,100);  // inner, outer, force 
	CreateSkinnedBlob( 40, 780,100, 50, 65, 10,10);  // inner, outer, force 
	CreateSkinnedBlob( 40, 780,100, 50, 65, 10,20);  // inner, outer, force 
	CreateSkinnedBlob( 80, 300,100, 50, 60, 10,20);  // inner, outer, force 
//	CreateSkinnedBlob( 80, 150,80, 50, 60, 10,10);  // inner, outer, force 
//	CreateSkinnedBlob( 80, 300,100, 50, 60, 10,2);  // inner, outer, force 
//	CreateSkinnedBlob( 80, 300,100, 40, 60, 40,20);  // inner, outer, force 

#endif

}


void	VerletRender()
{
	vector<CVerletPoint*>::iterator i;
	for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
	{
//		DrawLine((*i)->GetPos(), (*i)->GetLastPos() + Vector2(20,20), 0xff0000ff);

		(*i)->debug_render();

	}

}
void	VerletLogic()
{
	Vector2 jump = Vector2(0,0);

	static CVerletPoint	*p_drag = NULL;
	static bool	dragging_point = false;

	
	if (g_pGamepad->button[CGamepad::BTN_Y].Pressed())
		g_friction = 0.1f;  // Sticky
	else if (g_pGamepad->button[CGamepad::BTN_A].Pressed())
		g_friction = 1.0f;  // Slidy
	else
		g_friction = 0.85f;  // Normal

	vector<CVerletPoint*>::iterator i;

	Vector2 mouse = Vector2((float)mouse_x, (float) mouse_y);
	DrawLine(mouse-Vector2(0,10),mouse+Vector2(0,10),0xff300030);
	DrawLine(mouse-Vector2(10,0),mouse+Vector2(10,0),0xff300030);

	if (dragging)
	{

		if (dragging_point)
		{
			//p_drag->SetPos(mouse);
			//p_drag->Set(mouse);
		}
		else
		{

			// Find if there is a point close enough to grab onto
			float closest = 1000.0f;
			for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
			{
				CVerletPoint& v = *(*i);
				float len2 = (mouse-v.GetPos()).Length2(); 
				if (len2<closest)
				{
					dragging_point = true;
					closest = len2;
					p_drag = *i;
				}
			}
		}
	}
	else
	{
		dragging_point = false;
	}


	//debug_log("         VERLET LOGIC");

	Vector2	jump_force = Vector2(0,-20000);
	if (g_pGamepad->button[CGamepad::BTN_X].Triggered())
	{
		jump = jump_force;
		debug_log("         JUMP");
		g_pGamepad->button[CGamepad::BTN_X].ClearTrigger();
	}


// I'm doing multiple iterations of the physics with a small time step
// to avoid inversions of the spring mass structure
// some anti-inversion constraints may well obviate this need
	for (int tstep = 0;tstep<6;tstep++)
	{

		for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
		{
			CVerletPoint& v = *(*i);
	// gravity SET TO ZERO IF YOU DON"T WANT IT, DON'T COMMENT OUT THIS LINE
			v.SetForce(Vector2(0.0f,0));
#if 1 //ndef FIGURES
			v.SetForce(Vector2(0.0f,80.0f) * v.GetMass());
#endif
/*
			if (dragging)
			{
				Vector2 to = v.GetPos() - Vector2((float)mouse_x,(float)mouse_y);
				//float force = 1000.0f/to.Length2();
				float force = 160;		// MOUSE DRAG FORCE
				Vector2 acc = -to.Normal() * force * v.GetMass();
				//debug_log("%.3f,%.3f",acc.x,acc.y);
				v.AddForce(acc);
			}
*/

			if (dragging_point && &v==p_drag)
			{
				//p_drag->SetPos(mouse);
				//p_drag->Set(mouse);
				DrawLine(v.GetPos(),mouse,0xffff00ff);
				Vector2 to = v.GetPos() - Vector2((float)mouse_x,(float)mouse_y);
				//float force = 1000.0f/to.Length2();
				float force = 50000;		// MOUSE DRAG FORCE
				Vector2 acc = -to.Normal() * force * v.GetMass();
				//debug_log("%.3f,%.3f",acc.x,acc.y);
				v.AddForce(acc);
			}


			v.AddForce(jump*v.GetMass());

			Vector2	lr_force = Vector2(60,0);
			Vector2	ud_force = Vector2(0,60);


			if (g_pGamepad->button[CGamepad::BTN_L].Pressed())
				v.AddForce(-lr_force);
			if (g_pGamepad->button[CGamepad::BTN_R].Pressed())
				v.AddForce(lr_force);
			if (g_pGamepad->button[CGamepad::BTN_U].Pressed())
				v.AddForce(-ud_force);
			if (g_pGamepad->button[CGamepad::BTN_D].Pressed())
				v.AddForce(ud_force);




		//	debug_log("         GATHER START");
			v.GatherForces();
		//	debug_log("         GATHER END");



			Vector2 f = v.GetForce();
		//	debug_log("%d: %.3f,%.3f",i,f.x,f.y);
		//	DrawLine(v.GetPos(),v.GetPos()+f*20,0xff00ff00);  // Green = force

		}
#if 1 //ndef FIGURES

		for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
		{
			CVerletPoint& v = *(*i);
			v.Integrate(0.0016666f);
		}


		for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
		{
			CVerletPoint& v = *(*i);
			v.SatisfyConstraints();
		}

		for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
		{
			CVerletPoint& v = *(*i);
			v.SatisfyCollisionConstraints();
		}

		for (i = gVerletSystem.System().begin(); i != gVerletSystem.System().end(); i++)
		{
			CVerletPoint& v = *(*i);
			v.SatisfyCollision2Constraints();
		}
#endif

	}

}
void	VerletCleanup()
{

}




