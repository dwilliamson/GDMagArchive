//-----------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------
#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
//#pragma warning( default : 4996 ) 

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; // Buffer to hold vertices
LPDIRECT3DVERTEXBUFFER9 g_pVB2       = NULL; // Buffer to hold vertices
LPDIRECT3DTEXTURE9      g_pTexture   = NULL; // Our texture
ID3DXFont*              g_pFont      = NULL;
D3DCAPS9 g_Caps;


void    DrawString(float x, float y, const char *p_text, DWORD color = 0xff000000);

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

#ifdef		CRITICAL_SECTION_RND
CRITICAL_SECTION	rnd_CS;
#endif

////
// since rand is weak, I implement my own random numbers

int rnd_a = 12345678;
int rnd_b = 12393455;
int rnd_c = 45432838;	   

void randomize(unsigned int a = 12345678)
{
	rnd_a = a;
	rnd_b = 12393455;
	rnd_c = 45432838;	   
}

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


int	key_left =0;
int key_right = 0;
int key_up = 0;
int key_down = 0;
int key_w=0;
int key_a=0;
int key_s=0;
int key_d=0;
int key_r=0;
int key_f=0;
int key_space=0;
int key_esc=0;

int mouse_x, mouse_y;
bool dragging = false;


//////////////////////////////////////////////////////////////////////////
// Simple Vector3 lib, derived from D3DXVECTOR3

#if 0
typedef D3DXVECTOR3 Vector3;  // Just so it reads like my old code (Vector2)
#else
class	Vector3	: public D3DXVECTOR3
{
public:

	Vector3()
	{
	}

	Vector3(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	Vector3( const D3DXVECTOR3 &v )
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	float			Vector3::Length2	( void ) const
	{
		return (( x * x ) + 
				( y * y ) +
				( z * z ));
	}

	float	Length ( void ) const
	{
		return sqrtf ( Length2 ());
	}

   	Vector3&	Normalize( )
	{
		float len = Length();
		if ( len > 0.0f ) 
		{
			len = 1.0f / len;
			x *= len;
			y *= len;
			z *= len;
		}
		else
		{
		}
		return *this;
	}

};

float	DotProduct ( const Vector3& v1, const Vector3& v2 ) 
{
	
	
	return ( v1.x * v2.x ) + ( v1.y * v2.y ) + ( v1.z * v2.z );
}


Vector3	CrossProduct ( const Vector3& v1, const Vector3& v2 ) 
{	
	return Vector3 (( v1.y * v2.z ) - ( v1.z * v2.y ),
				   ( v1.z * v2.x ) - ( v1.x * v2.z ),
				   ( v1.x * v2.y ) - ( v1.y * v2.x ));
}

#endif

// A very simple 3d camera

class CCamera
{
public:
	Vector3 m_pos;
	Vector3 m_fwd;
	Vector3 m_left;
	Vector3 m_up;

			CCamera();
	void	Update();
	void	LookAt(Vector3 &at);
	void	Move(Vector3 off);
	void	MoveRel(Vector3 off);
	void	Turn(float a);
	void	Tilt(float a);
};

CCamera::CCamera()
{
	m_pos = Vector3( 2.0f, 2.0f,-15.0f);
	m_up = Vector3( 0.0f, 1.0f, 0.0f );

	LookAt( Vector3(5.0f, 1.0f, 2.0f) );
}

void	CCamera::LookAt(Vector3 &at)
{
	if (at == m_pos)
	{
		// can't look at a point we are already at, so just leave it alone.
		return;
	}
	// To look at a point, we just set the fwd vector to point towards it
	m_fwd = (at - m_pos);
	// and normalize it (possible problem here if at == m_pos)
	m_fwd.Normalize();
}

void CCamera::Move(Vector3 off)
{	
	m_pos += off;
}

// a very simple rotation about up vector
void CCamera::Turn(float a)
{
	D3DXMATRIX rot;
	// rotate fwd vector about up by an arbitary angle
	D3DXMatrixRotationAxis(&rot, &m_up, a);
	D3DXVec3TransformCoord(&m_fwd,&m_fwd,&rot);
}

// a very simple rotation about left vector
void CCamera::Tilt(float a)
{
	Vector3 left = CrossProduct(m_up,m_fwd);
	D3DXMATRIX rot;
	// rotate fwd vector about left by an arbitary angle
	D3DXMatrixRotationAxis(&rot, &left, a);
	D3DXVec3TransformCoord(&m_fwd,&m_fwd,&rot);
}


void CCamera::MoveRel(Vector3 off)
{
	Vector3 left = CrossProduct(m_up,m_fwd);
	m_pos += off.x*left + off.y * m_up + off.z*m_fwd;

}

void CCamera::Update()
{

	if (key_a)
	{
		MoveRel(Vector3(-0.3,0,0));
	}
	if (key_d)
	{
		MoveRel(Vector3(0.3,0,0));
	}
	if (key_r)
	{
		MoveRel(Vector3(0,.1,0));
	}
	if (key_f)
	{
		MoveRel(Vector3(0,-.1,0));
	}
	if (key_w)
	{
		MoveRel(Vector3(0,0,.4));
	}
	if (key_s)
	{
		MoveRel(Vector3(0,0,-0.4));
	}

	if (key_left)
	{
		Turn(-.03);
	}

	if (key_right)
	{
		Turn(+.03);
	}

	if (key_up)
	{
		Tilt(-.03);
	}

	if (key_down)
	{
		Tilt(+.03);
	}

	char buf[1024];
	sprintf(buf, "m_fwd = (%.3f,%.3f,%.3f)",m_fwd.x,m_fwd.y,m_fwd.z);
	DrawString(16,48,buf,0xff2020f0);


    Vector3 vEyePt = m_pos;
    Vector3 vLookatPt = m_pos + m_fwd;
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &m_up );
//    D3DXMatrixRotationY( &matView, Timer_Seconds() );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
}

CCamera  g_camera;

// A structure for our custom vertex type. We added texture coordinates
struct CUSTOMVERTEX
{
    D3DXVECTOR3 position; // The position
	D3DXVECTOR3 normal;   // The surface normal for the vertex
    D3DCOLOR    color;    // The color
    FLOAT       tu, tv;   // The texture coordinates
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// Custom vertex for 3D lines
struct CUSTOMLINEVERTEX
{
    D3DXVECTOR3 position; // The position
    D3DCOLOR    color;    // The color
};


#define D3DFVF_CUSTOMLINEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)



//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice, try for hardware vert processing first
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
#if 1
		if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
										  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										  &d3dpp, &g_pd3dDevice ) ) )
#else
		if( FAILED( g_pD3D->CreateDevice( 1, D3DDEVTYPE_HAL, hWnd,
										  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										  &d3dpp, &g_pd3dDevice ) ) )
#endif
		{
			return E_FAIL;
		}
	}

    // Turn off culling
     g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Turn on the zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	// get a font
    if( FAILED( D3DXCreateFont( g_pd3dDevice, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         (LPCSTR)"Arial", &g_pFont ) ) )
    {
        return E_FAIL;
    }


	// Get the device capabilities,  mostly for max primitive count
	g_pd3dDevice->GetDeviceCaps(&g_Caps);

    return S_OK;
}


// some simple string rendering
const int max_strings = 200;
const int max_string_length = 255;
struct SDrawText {
    float x,y;
    char text[max_string_length+1];
    DWORD color;
};

int num_draw_strings=0;

SDrawText   texts_to_draw[max_strings];
void    DrawString(float x, float y, const char *p_text, DWORD color)
{
    if (num_draw_strings == max_strings)
        return;
    texts_to_draw[num_draw_strings].x = x;    
    texts_to_draw[num_draw_strings].y = y;    
    texts_to_draw[num_draw_strings].color = color;
    strncpy(texts_to_draw[num_draw_strings].text,p_text,max_string_length);    
    texts_to_draw[num_draw_strings].text[max_string_length]='\0'; // NULL terminator for iff p_text is >255 chars
    num_draw_strings++;
    
}



#define MAX_VERTS 1000000
#define MAX_LINE_VERTS 100000
CUSTOMVERTEX* g_pVertices;
CUSTOMLINEVERTEX* g_pLineVertices;
int			  g_nextVert;
int			  g_nextLineVert;


void PG_Vertex(Vector3 position, unsigned int color = 0xffffffff, float tu = 0.0f, float tv = 0.0f)
{

	if (g_nextVert >= MAX_VERTS)
		return;

	g_pVertices[g_nextVert].position = position;
	g_pVertices[g_nextVert].color = color;
	g_pVertices[g_nextVert].tu = tu;
	g_pVertices[g_nextVert].tv = tv;

	g_nextVert++;
}

void PG_LineVertex(Vector3 position, unsigned int color = 0xffffffff)
{

	if (g_nextLineVert >= MAX_VERTS)
		return;

	g_pLineVertices[g_nextLineVert].position = position;
	g_pLineVertices[g_nextLineVert].color = color;

	g_nextLineVert++;
}

void PG_Line(Vector3 p0, Vector3 p1, unsigned int color = 0xffffffff)
{
	PG_LineVertex(p0,color);
	PG_LineVertex(p1,color);
}

void PG_VertexN(Vector3 position, Vector3 normal, unsigned int color = 0xffffffff, float tu = 0.0f, float tv = 0.0f)
{

	if (g_nextVert >= MAX_VERTS)
		return;

	g_pVertices[g_nextVert].position = position;
	g_pVertices[g_nextVert].normal = normal;
	g_pVertices[g_nextVert].color = color;
	g_pVertices[g_nextVert].tu = tu;
	g_pVertices[g_nextVert].tv = tv;

	g_nextVert++;
}

Vector3 NormalFromTri(Vector3 p0, Vector3 p1, Vector3 p2)
{
	Vector3 side1 = (p1-p0);
	Vector3 side2 = (p2-p0);
	Vector3 normal = CrossProduct(side1,side2).Normalize();
	return normal;
}

void PG_Triangle(Vector3 p0, Vector3 p1, Vector3 p2, unsigned int color = 0xffffffff)
{
// A triangle alone has no info about the normals of it verts
// so set them all to the normal of the triangle surface, which is the cross product of two sides
	Vector3 normal = NormalFromTri(p0,p1,p2);
	
#if 0
	Vector3 mid = (p0+p1+p2)/3.0f;
	PG_Line(mid,mid+normal*.1);
#endif

	PG_VertexN(p0,normal,color);
	PG_VertexN(p1,normal,color);  
	PG_VertexN(p2,normal,color);
}

// traingle with supplied vertex normals
void PG_TriangleN(Vector3 p0, Vector3 n0, Vector3 p1, Vector3 n1, Vector3 p2, Vector3 n2, unsigned int color = 0xffffffff)
{

#if 0
	PG_Line(p0,p0+n0,color);
	PG_Line(p1,p1+n1,color);  
	PG_Line(p2,p2+n2,color);
#endif

	PG_VertexN(p0,n0,color);
	PG_VertexN(p1,n1,color);  
	PG_VertexN(p2,n2,color);
}

void PG_TriangleNUV(Vector3 p0, Vector3 n0, float u0, float v0, Vector3 p1, Vector3 n1, float u1, float v1, Vector3 p2, Vector3 n2, float u2, float v2, unsigned int color = 0xffffffff)
{

	PG_VertexN(p0,n0,color,u0,v0);
	PG_VertexN(p1,n1,color,u1,v1);  
	PG_VertexN(p2,n2,color,u2,v2);
}


void PG_Quad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, unsigned int color = 0xffffffff)
{
	PG_Triangle(p0,p1,p2,color); 
	PG_Triangle(p0,p2,p3,color);
}

void PG_QuadN(Vector3 p0, Vector3 n0, Vector3 p1, Vector3 n1,Vector3 p2, Vector3 n2,Vector3 p3,Vector3 n3, unsigned int color = 0xffffffff)
{
	PG_TriangleN(p0,n0,p1,n1,p2,n2,color); // good on top
	PG_TriangleN(p0,n0,p2,n2,p3,n3,color);
}

void PG_QuadNUV(Vector3 p0, Vector3 n0, float u0, float v0, Vector3 p1, Vector3 n1, float u1, float v1, Vector3 p2, Vector3 n2, float u2, float v2, Vector3 p3,Vector3 n3, float u3, float v3 , unsigned int color = 0xffffffff)
{
	PG_TriangleNUV(p0,n0,u0,v0,p1,n1,u1,v1,p2,n2,u2,v2,color); // good on top
	PG_TriangleNUV(p0,n0,u0,v0,p2,n2,u2,v2,p3,n3,u3,v3,color);
}


// Truncated pyramid defined by two points and two widths, sides will be axis aligned
void PG_TruncatedPyramid(Vector3 p0, Vector3 p1, float base, float top)
{
	// Set up the base points and the top points
	// initially relative to the origin, we'll rotate them then move them
	Vector3 p[8];		// 4 base points 4 top points
	// we need to get the vector p0->p1 which is the major axis
	Vector3 major_axis = (p1-p0);  // well, that was easy

	// get the normal along this axis
	Vector3 n;
	D3DXVec3Normalize(&n,&major_axis);

	// Get perpendicular basis vectors
	Vector3 v[4];
	Vector3 left = Vector3(n.z,-n.x,-n.y);  // get arbitary perpendicular vector
	Vector3 up = CrossProduct(n,left);		// Get one perpendicular to both
	left = CrossProduct(n,up);				// and make it orthogonal

	// Get a square of points forming a plane perpendicular to the axis
	v[0] = +left+up;
	v[1] = +left-up;
	v[2] = -left-up;
	v[3] = -left+up;

	// Add the points to the top and bottom points
	for (int i=0;i<4;i++)
	{
		p[0+i] = p0 + v[0+i] * base;
		p[4+i] = p1 + v[i] * top;
	}
	// Now we make the six faces from the eight corners
	PG_Quad(p[3],p[2],p[1],p[0]);
	PG_Quad(p[0],p[4],p[7],p[3]);
	PG_Quad(p[3],p[7],p[6],p[2]);
	PG_Quad(p[2],p[6],p[5],p[1]);
	PG_Quad(p[1],p[5],p[4],p[0]);
	PG_Quad(p[4],p[5],p[6],p[7]);  // note the reverse order, as it's upside down
}

// Truncated cone defined by two points and two widths, and a number of segments
void PG_TruncatedCone(Vector3 p0, Vector3 p1, float base, float top, int segments)
{
	// we need to get the vector p0->p1 which is the major axis
	Vector3 major_axis = (p1-p0);  // well, that was easy

	// get the normal along this axis
	Vector3 n = major_axis;
	n.Normalize();

	// Get perpendicular basis vector
	Vector3 v[1024];
	Vector3 left = Vector3(n.z,-n.x,-n.y);  // get arbitary perpendicular vector
	Vector3 up = CrossProduct(n,left);		// Get one perpendicular to both

	// Get a rotation matrix about the axis
	D3DXMATRIX rot;
	D3DXMatrixRotationAxis(&rot, &n, D3DX_PI*2.0f/(float)segments);

	// Get an array of points forming an N-Gon plane perpendicular to the axis
	// by rotating the "up" vector by the number of segments needed.
	for (int i=0;i<segments;i++)
	{
		v[i] = up;
		D3DXVec3TransformCoord(&up,&up,&rot);
	}

	for (int seg=0;seg<segments;seg++)
	{
		PG_Triangle(p0,p0+v[(seg+1)%segments]*base,p0+v[seg]*base);
		PG_Triangle(p1,p1+v[seg]*top,p1+v[(seg+1)%segments]*top);

		// We need to calculate the three normals
		// then average them to get the edge normals

		Vector3 n_mid = NormalFromTri(p1+v[(seg+1)%segments]*top,
			p1+v[seg]*top,
			p0+v[seg]*base);

		Vector3 n_left = NormalFromTri(p1+v[(seg+1-1)%segments]*top,
			p1+v[(seg-1+segments)%segments]*top,
			p0+v[(seg-1+segments)%segments]*base);

		Vector3 n_right = NormalFromTri(p1+v[(seg+1+1)%segments]*top,
			p1+v[(seg+1)%segments]*top,
			p0+v[(seg+1)%segments]*base);


		Vector3 n_right_edge = ((Vector3)(n_mid+n_right)).Normalize();
		Vector3 n_left_edge = ((Vector3)(n_mid+n_left)).Normalize();

#if 0
		// quads with smooth normals
		PG_QuadN(
			p1+v[(seg+1)%segments]*top,n_right_edge,
			p1+v[seg]*top,n_left_edge,
			p0+v[seg]*base,n_left_edge,
			p0+v[(seg+1)%segments]*base,n_right_edge
			);
#else
		// quads with smooth normals and face mapped to default texture
		float r = (float)seg/(float)segments;
		float r1 = r + 1.0f/(float)segments;
		//r1=1.0f;
		PG_QuadNUV(
			p1+v[(seg+1)%segments]*top,n_right_edge ,r1,0.0f,  // top left corner
			p1+v[seg]*top,n_left_edge               ,r,0.0f, // top right corner, messed up
			p0+v[seg]*base,n_left_edge              ,r,1.0f, 
			p0+v[(seg+1)%segments]*base,n_right_edge,r1,1.0f
			//,0x00804020
			);

	//	PG_Line(p1+v[(seg+1)%segments]*top,p1+v[(seg+1)%segments]*top+n_mid,0xffffffff);
#endif
	}

}

void	PG_CrapTree(Vector3 a, Vector3 b, float w, int recurse, int segments = 8)
{
	if (recurse <=0) 
		return;

	float shrink = 0.7f;

	PG_TruncatedCone(a,b,w,w*shrink,segments);

	//PG_Line(a + Vector3(1,0,0),b+Vector3(1,0,0),0xff0000ff);

	Vector3 up = (b-a);
	Vector3 left = Vector3(-up.z,-up.x,up.y);

	// Primary branch deviates less
	PG_CrapTree(b,b+up/(1.0f + rndf(0.2f)) + left/(10.0f+rnd(1.0f)),w*shrink,recurse-1, segments);
	// Secondary branch deviates more and is shorter
	PG_CrapTree(b,b+up/(1.0f + rndf(2.6f)) - left/(2.0f+rnd(2.0f)),w*shrink,recurse-1, segments);
}

// Tilt a vector by an angle in a random direction.
inline Vector3 TiltRandom(Vector3 up, float angle)
{
	// get a left vector, which is perpendicular to the up vector
	Vector3 left = Vector3(-up.z,-up.x,up.y);
	Vector3 result;

	// rotation matrix, used twice
	D3DXMATRIX rot;

	// rotate left vector about up by an arbitary angle
	D3DXMatrixRotationAxis(&rot, &up, rnd(D3DX_PI*2.0f));
	D3DXVec3TransformCoord(&left,&left,&rot);

	// rotate up vector about this random vector by the desired angle
	D3DXMatrixRotationAxis(&rot, &left, angle);
	D3DXVec3TransformCoord(&result,&up,&rot);
	return result;

}

// Recursivly create branch geometry
void	PG_AngleTreeBranch(Vector3 a, Vector3 b, float w, float ratio, float shrink, int recurse, float LOD, int segments = 8, int seed = 12345678)
{

	randomize(seed);

	PG_TruncatedCone(a,b,w,w*shrink,segments);

	if (recurse <=1) 
	{
		return;
	}
	
	Vector3 up = (b-a);
	Vector3 up1 = TiltRandom(up,rndf(0.0f,(float)mouse_x/200.0f)) * ratio ;
	//Vector3 up2 = TiltRandom(up,rndf(0.2f,2.1f / recurse)) * ratio * rndf(0.8f,1.0f);
	Vector3 up2 = TiltRandom(up,rndf(0.0f,(float)mouse_y/200.0f / recurse)) * ratio ;


	unsigned int seed1 = rnd();
	unsigned int seed2 = rnd();
	unsigned int seed3 = rnd();

	// Move point b back a bit along a-b , so branches starting from it 
	// will be embedded in the end of the larger, lower, segment
	b = a + 0.95f*(b-a);

	// Primary branch deviates less
	PG_AngleTreeBranch(b,b+up1,w*shrink, ratio, shrink, recurse-1, LOD, segments, seed1);

	if (recurse > LOD)
	{
		// Secondary branch deviates more and is shorter
		if ((recurse-LOD) <= 1.0f)
		{	
			up2 *= (recurse-LOD);
		}

		PG_AngleTreeBranch(b,b+up2,w*shrink, ratio, shrink, recurse-1, LOD, segments, seed2);
	}
	// always return with the same randomizer state
	randomize(seed3);

}


// some intial calculation, then call the recursive PG_AngleTreeBranch function
void	PG_AngleTree(Vector3 a, Vector3 b, float w, float ratio, float shrink, int recurse, float LOD, int segments = 8, int seed = 12345678)
{

		unsigned int exit_seed = rnd();

		// adjust height
		float sum = 0.0f;
		float relative = 1.0f;
		for (int seg = 0; seg<recurse; seg++)
		{
			sum += relative;
			relative *= ratio;
		}

		// Scale point b so tree will end up at point b
		// regardless of the recursion depth
		b = a + (b-a) / sum;

		// Scale the "shrink" factor similarly
		shrink = expf( logf(shrink) / float(recurse));

		PG_AngleTreeBranch( a,  b,  w,  ratio, shrink,  recurse, LOD, segments, seed);
		randomize(exit_seed);
}

// Generate a tesselated XY plane
// fairly useless, but used here as a baseline metric for geometry generation
void PG_TessPlaneXY(float x0, float y0, float x1, float y1, int tessx, int tessy)
{
	float stepx = (x1-x0) / (float) tessx;
	float stepy = (y1-y0) / (float) tessy;
	float x = x0;


	int quads = 0;
	for (int xx = 0; xx<tessx; xx++)
	{
		float y=y0;
		for (int yy = 0; yy < tessy; yy++)
		{
			quads++;
			PG_Quad(
				Vector3(x,y+stepy,0),
				Vector3(x+stepx,y+stepy,0), 
				Vector3(x+stepx,y,0),
				Vector3(x,y,0), 
				0xff00ff00
				);
			PG_Line(Vector3(x,y,0),Vector3(x+stepx,y+stepy,0));
			y+= stepy;
		}
		x+= stepx;
	}


	char buf[1024];
	sprintf(buf, "quads = %d, stepx = %f, stepy = %f, tessx = %d, tessy=%d",quads,stepx,stepy,tessx,tessy);
	DrawString(16,48,buf,0xff2020f0);


}


//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Create the Textures and vertex buffers
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    // Use D3DX to create a texture from a file based image
    if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "browntex.bmp", &g_pTexture ) ) )
    {
        // If texture is not in current folder, try parent folder
        if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "..\\browntex.bmp", &g_pTexture ) ) )
        {
            MessageBox(NULL, "browntex.bmp", "Proctree.exe", MB_OK);
            return E_FAIL;
        }
    }

    // Create the riangle vertex buffer.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( MAX_VERTS*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    {
        return E_FAIL;
    }

    // Create the Line vertex buffer.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( MAX_LINE_VERTS*sizeof(CUSTOMLINEVERTEX),
                                                  0, D3DFVF_CUSTOMLINEVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB2, NULL ) ) )
    {
        return E_FAIL;
    }
    return S_OK;
}


// Generate geometry
// called once per frame, but obviously could just be called once.
HRESULT GenerateGeometry()
{

	randomize();

#if 1
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&g_pVertices, 0 ) ) )
        return E_FAIL;
    if( FAILED( g_pVB2->Lock( 0, 0, (void**)&g_pLineVertices, 0 ) ) )
	{
		g_pVB->Unlock();
        return E_FAIL;
	}
#endif
	g_nextVert = 0;
	g_nextLineVert = 0;


		//PG_Triangle(Vector3(0,0,0), Vector3(10,0,10),Vector3(10,10,5), 0xffff00ff);
//		PG_Quad(Vector3(0,0,0), Vector3(0,0,1),Vector3(0,1,1),Vector3(0,1,0), 0xffffffff);

//		PG_TruncatedPyramid(Vector3(0,0,0), Vector3(0,1,0), 0.2, 0.1);
//		PG_TruncatedCone(Vector3(0,0,0), Vector3(0,1,0), 0.2, 0.1,30);
		

//		PG_TruncatedPyramid(Vector3(1,0,1), Vector3(1.5,0,1.5), 0.2, 0.3);

//		PG_TruncatedPyramid(Vector3(1,0,1), Vector3(2,0,1), 0.2, 0.2);
//		PG_TruncatedPyramid(Vector3(1,0,1), Vector3(1,1,1), 0.2, 0.2);


	// Draw a base grid

	// 
	float w = 24.0f;
	float step = 1.0f;
	for (float x=0;x<w;x+=step)
		for (float z=0;z<w;z+=step)
		{
	//		PG_Line(Vector3(x,0,0),Vector3(x,0,w-step),0x808080);
	//		PG_Line(Vector3(0,0,z),Vector3(w-step,0,z),0x808080);
		}

#if 1
// render a forect of trees
		// height of the whole tree
//		float h = 5.0f + 0.2f*sinf(Timer_Seconds());
		float ratio = 0.9f;

		for (int x=0;x<4;x++)
		{
			for (int z=0;z<40;z++)
			{
				float xx = 2.0f + x*2.5f + rndf(-1.5f,+1.5f);
				float zz = 2.0f + z*2.5f + rndf(-1.5f,+1.5f);

				float h = rndf(3.0f,6.0f);

				float branch_ratio = rndf(.8f, 0.99f);

				float trunk_width = rndf(0.03f,h*0.15f/6.0f);

				int recurse = 8;

				float LOD = 6.235435435;
				int segments = 3;

				Vector3 from_cam = ((Vector3(xx,0.0f,zz) - g_camera.m_pos));
				float cam_dist = from_cam.Length();


				LOD = sqrtf(cam_dist);
				if (LOD > recurse-1)
					LOD = recurse-1;

				if (x ==0 && z ==0)
				{
					char buf[1024];
					sprintf(buf, "cam_dist = %f, LOD = %f", cam_dist, LOD);
					DrawString(320,68,buf,0xff2020f0);
				}
				PG_AngleTree(Vector3(xx,0.0,zz), Vector3(xx,0.0+h,zz),   trunk_width, branch_ratio, 0.1f, 
					recurse, LOD, segments, (x+1)*(z+1));
				
			}
		}
#else
// render a single tree
				float xx = 6.0f;
				float h =11.0f;
				//float ratio = 0.99f;
				float ratio = 0.99f; //(float)mouse_y/800.0f;
				randomize(2);
				PG_AngleTree(Vector3(xx,0,xx), Vector3(xx,0+h,xx), 0.1f, ratio, 0.1f, 11, (float)mouse_y/100, 8, 12347);
#endif

//	PG_TessPlaneXY(0,0,4,4,10,10);


//		PG_CrapTree(Vector3(1,-1,0), Vector3(1,-.75,0), 0.01f, 3, 15);

#if 1
       g_pVB2->Unlock();
       g_pVB->Unlock();
#endif

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: SetupLights()
// Desc: Sets up the Lights and materials for the scene.
// Called every frame
//-----------------------------------------------------------------------------
VOID SetupLights()
{
	
    D3DMATERIAL9 mtrl;
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial( &mtrl );


    // Set up a white, directional light, with an oscillating direction.
    D3DXVECTOR3 vecDir;
    D3DLIGHT9 light;
    ZeroMemory( &light, sizeof(D3DLIGHT9) );
    light.Type       = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r  = 1.0f;
    light.Diffuse.g  = 1.0f;
    light.Diffuse.b  = 1.0f;
#if 1
    vecDir = D3DXVECTOR3(cosf(Timer_Seconds()),
                         1.0f,
                         sinf(Timer_Seconds()) );
#else
	    vecDir = D3DXVECTOR3(cosf(0/350.0f),
                         1.0f,
                         sinf(0/350.0f) );
#endif
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
    light.Range       = 1000.0f;
    g_pd3dDevice->SetLight( 0, &light );
    g_pd3dDevice->LightEnable( 0, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Finally, turn on some ambient light.
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00f02020 );
}



//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pTexture != NULL )
        g_pTexture->Release();

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
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view (camera), and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    // Set up world matrix
    D3DXMATRIXA16 matWorld;
    D3DXMatrixIdentity( &matWorld );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

#if 0
    D3DXVECTOR3 vEyePt( 2.0f, 2.0f,-15.0f );
    D3DXVECTOR3 vLookatPt( 5.0f, 1.0f, 2.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    D3DXMatrixRotationY( &matView, Timer_Seconds() );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
#else
	g_camera.Update();
#endif

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer and the zbuffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         //D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
                         D3DCOLOR_XRGB(255,255,255), 1.0f, 0 );

	static float fps = 0.0f;
    float   start = Timer_Seconds();
	char buf[100];

	GenerateGeometry();

	float gen_time = Timer_Seconds()-start;

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Setup the world, view, and projection matrices
        SetupMatrices();

		// Setup the Lights and materials
//        SetupLights();

    // Set up a material. The material here just has the diffuse and ambient
    // colors set to ????? (White). Note that only one material can be used at a time.

#if 1
    D3DMATERIAL9 mtrl;
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial( &mtrl );
#endif	
    // Set up a white, directional light, with an oscillating direction.
    // Note that many Lights may be active at a time (but each one slows down
    // the rendering of our scene). However, here we are just using one. Also,
    // we need to set the D3DRS_LIGHTING renderstate to enable lighting
    D3DXVECTOR3 vecDir;
    D3DLIGHT9 light;
    ZeroMemory( &light, sizeof(D3DLIGHT9) );
    light.Type       = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r  = 1.0f;    
    light.Diffuse.g  = 1.0f;
    light.Diffuse.b  = 1.0f;
    vecDir = D3DXVECTOR3(cosf(Timer_Seconds()),
                         1.0f,
                         sinf(Timer_Seconds()) );
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
    light.Range       = 1000.0f;
    g_pd3dDevice->SetLight( 0, &light );
    g_pd3dDevice->LightEnable( 0, TRUE );


    // Render the line list vertex buffer contents
    // lights off for the lines
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );  // works with material
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetStreamSource( 0, g_pVB2, 0, sizeof(CUSTOMLINEVERTEX) );
    g_pd3dDevice->SetFVF( D3DFVF_CUSTOMLINEVERTEX );
    g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, g_nextLineVert/2 );

	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xff804020 );  // works with material
	//g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );  // works with material
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		
#if 0
    // Setup our texture. Using Textures introduces the texture stage states,
    // which govern how Textures get blended together (in the case of multiple
    // Textures) and lighting information. In this case, we are modulating
    // (blending) our texture with the diffuse color of the vertices.
    g_pd3dDevice->SetTexture( 0, g_pTexture );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    //g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
#endif

#if 0
        // Render the triangle list vertex buffer contents
        g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
        g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nextVert/3 );
#else
	// render in batches to allow for more polys on weak machines
		static int batch_size = 20000;  // number of triangles to send at once

		batch_size = g_Caps.MaxPrimitiveCount/3;

		int tris_left = g_nextVert/3;
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
#endif

        // End the scene
        g_pd3dDevice->EndScene();
    }






        RECT rc;
        for (int i=0;i<num_draw_strings;i++)
        {
            SetRect( &rc, (int)texts_to_draw[i].x, (int)texts_to_draw[i].y, 0, 0 );        
            g_pFont->DrawText(NULL, (LPCSTR)texts_to_draw[i].text, -1, &rc, DT_NOCLIP, texts_to_draw[i].color);
        }
        num_draw_strings = 0;


    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );


	float end = Timer_Seconds();
	fps = 1.0f / (end - start);

	float pps = (g_nextVert/3/gen_time);

	static float ppsx[60];
	static int ppsi = 0;
	ppsx[ppsi] = pps;
	ppsi++;
	if (ppsi == 60)
		ppsi=0;
	float ppsa = 0;
	for (int i=0;i<60;i++)
		ppsa += ppsx[i];
	ppsa /= 60.0f;
		sprintf(buf, "FPS = %4.2f, gen time = %0.4f, polys = %d, pps = %8.0f, (%8.0f k/s) g_nextVert=%d",fps,gen_time, g_nextVert/3, ppsa, ppsa/1024*sizeof(CUSTOMVERTEX),g_nextVert);
	DrawString(16,32,buf,0xff2020f0);


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
			char buf[1024];
			sprintf(buf, "Key = %2x",key); 
			DrawString(64,70,buf,0xff2020f0);

			if (key == 0x11) key_w = 1;
			if (key == 0x1e) key_a = 1;
			if (key == 0x1f) key_s = 1;
			if (key == 0x20) key_d = 1;
			if (key == 0x39) key_space = 1;
			if (key == 0x14b) key_left = 1;
			if (key == 0x14d) key_right = 1;
			if (key == 0x148) key_up = 1;
			if (key == 0x150) key_down = 1;
			if (key == 0x1)  
			{
				key_esc = 1;
	            Cleanup();
				PostQuitMessage( 0 );
				return 0;
			}
			if (key == 0x13) key_r = 1;
			if (key == 0x21) key_f = 1;


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
			if (key == 0x1)  key_esc = 0;
			if (key == 0x13) key_r = 0;
			if (key == 0x21) key_f = 0;

		}
		break;

    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "Mick West Procedural Trees", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
	HWND hWnd = CreateWindow( "Mick West Procedural Trees", "Mick West: Real Time Procedural Geometry as an Alternative to Spooling",
                              //WS_OVERLAPPEDWINDOW, 0, 0, 1200, 768,
                              WS_OVERLAPPEDWINDOW, 0, 0, 1600, 1200,
                              NULL, NULL, wc.hInstance, NULL );

	Timer_Init();

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the scene geometry
        if( SUCCEEDED( InitGeometry() ) )
        {
			// We do the geometry generation oce here currently
			// but also do it in real time in every frame
			// just a note in case you want to change this.
			if( SUCCEEDED( GenerateGeometry() ) )
			{
				// Show the window
				ShowWindow( hWnd, SW_SHOWDEFAULT );
				UpdateWindow( hWnd );

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof(msg) );
				while( msg.message!=WM_QUIT )
				{
					if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
					{
						TranslateMessage( &msg );
						DispatchMessage( &msg );
					}
					else
						Render();
				}
			}
        }
    }

    UnregisterClass( "Mick West Procedural Trees", wc.hInstance );
    return 0;
}



