//--------------------------------------------------------------------------------------
// File: GeometryTextures.cpp
//
// Basic starting point for new Direct3D samples
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------
// The changes to the initial file as copied from the DirectX samples are not under any copyright.
// You may use them freely for your softwarte projects. Your use of the unchanged portion of the source
// code however must obey the rules specified for direct x samples as described in the 'DirectX SDK EULA'
// that comes with this source code. 
//
// DISCLAIMER OF WARRANTY.   The software is licensed “as-is.”  You bear the risk of using it.  
// The author gives no express warranties, guarantees or conditions.  
// You may have additional consumer rights under your local laws which this agreement cannot change.  
// To the extent permitted under your local laws, the author excludes the implied warranties of merchantability, 
// fitness for a particular purpose and non-infringement.
// ------------------------------------------------------------------------------------------------------------


#include "dxstdafx.h"
#include "resource.h"
#include <list>

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface
IDirect3DTexture9*      g_pTerrainTex = NULL;   // terrain texture
IDirect3DTexture9*      g_pGrassTex = NULL;     // grass texture
CFirstPersonCamera      g_Camera;               // A first person camera
CDXUTDirectionWidget    g_DirWidget;            // directional widgt
bool                    g_bShowHelp = true;     // If true, it renders the UI control text
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg         g_SettingsDlg;          // Device settings dialog
CDXUTDialog             g_HUD;                  // dialog for standard controls
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls
D3DXHANDLE              g_HandleTechniqueTerrain;
D3DXHANDLE              g_HandleTechniqueGrass;
D3DXHANDLE              g_HandleTechniqueRocks;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp();
void    RenderText();

#define NUM_TERRAIN_TILES 100
#define TERRAIN_TILE_SIZE 10.0f

struct TerrainVertex
  {
  D3DXVECTOR3 pos;
  D3DXVECTOR3 normal;
  D3DXVECTOR2 tex;
  };

D3DVERTEXELEMENT9 g_TerrainVertexElem[] = 
{
  { 0, 0,      D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, // position
  { 0, 3 * 4,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 }, // normal
  { 0, 6 * 4,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // texcoord
  D3DDECL_END()
};

struct Triangle
 {
 D3DXVECTOR3 vts[3];
 D3DXVECTOR3 ns[3];
 };
 
typedef std::list< Triangle >  TriangleList;
typedef TriangleList::iterator TriIt;

TriangleList g_TriangleList;

IDirect3DVertexDeclaration9* g_pTerrainVertexDecl = NULL;
IDirect3DVertexBuffer9 * g_pTerrainVB = 0;
IDirect3DIndexBuffer9 *  g_pTerrainIB = 0;

#define NUM_GRASS_TUFTS 200
#define GRASS_TUFT_WIDTH 0.25f
#define GRASS_TUFT_HEIGHT 0.4f

#define NUM_ROCKS 80
#define NUM_ROCK_SEGMENTS 7

struct VBGTVertex
{
  D3DXVECTOR3   pos;
  unsigned char col[4];
  D3DXVECTOR2   tex;
  D3DXVECTOR3   bary;
};

D3DVERTEXELEMENT9 g_VBGTVertexElem[] = 
{
  { 0, 0,      D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, // local position
  { 0, 3 * 4,  D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 }, // color
  { 0, 4 * 4,  D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // texture coord
  { 0, 6 * 4,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 }, // barycentric coords
  D3DDECL_END()
};

IDirect3DVertexDeclaration9* g_pVBGTVertexDecl = NULL;
IDirect3DVertexBuffer9 * g_pVBGTvb = 0;
IDirect3DIndexBuffer9 *  g_pVBGTib = 0;
IDirect3DVertexBuffer9 * g_pVBGTvb1 = 0;
IDirect3DIndexBuffer9 *  g_pVBGTib1 = 0;

float HeightMap[ NUM_TERRAIN_TILES + 2 ][ NUM_TERRAIN_TILES + 2 ];
TerrainVertex Triangles[ NUM_TERRAIN_TILES - 1][ NUM_TERRAIN_TILES - 1 ][ 2 ][ 3 ];

inline float randFloat()
{
  return ( float(rand()) / float( RAND_MAX ) );
}

static void setupHeightMap()
  {
  for( int y = 0; y < NUM_TERRAIN_TILES + 2; ++y )
    {
    for( int x = 0; x < NUM_TERRAIN_TILES + 2; ++x )
      HeightMap[x][y]= 5.0f * randFloat();
    }
  }

float getRandomHeight( int x, int y )
  {
  return HeightMap[ x+1 ][ y+1 ];
  }

D3DXVECTOR3 getNormal( const int x, const int y )
  {
  float h   = getRandomHeight( x, y );
  float hy0 = getRandomHeight( x, y - 1 );
  float hy1 = getRandomHeight( x, y + 1 );
  float hx0 = getRandomHeight( x - 1, y );
  float hx1 = getRandomHeight( x + 1, y );
  
  D3DXVECTOR3 xp(  TERRAIN_TILE_SIZE, hx1 - h, 0 );
  D3DXVECTOR3 xm( -TERRAIN_TILE_SIZE, hx0 - h, 0 );
  D3DXVECTOR3 zp(  0, hy1 - h, TERRAIN_TILE_SIZE );
  D3DXVECTOR3 zm(  0, hy0 - h,-TERRAIN_TILE_SIZE );

  D3DXVECTOR3 n1, n2, n3, n4;
  
  D3DXVec3Cross( &n1, &xp, &zm );
  D3DXVec3Cross( &n2, &zm, &xm );
  D3DXVec3Cross( &n3, &zp, &xp );
  D3DXVec3Cross( &n4, &xm, &zp );
  
  D3DXVECTOR3 sum = -n1 - n2 - n3 - n4;
  D3DXVECTOR3 n;
  
  D3DXVec3Normalize( &n, &sum );
  
  return n;
  }

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions. These functions allow DXUT to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then DXUT won't be able to 
    // recreate your device resources.
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTCreateWindow( L"GeometryTextures" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 640, 480, IsDeviceAcceptable, ModifyDeviceSettings );

    // Pass control to DXUT for handling the message pump and 
    // dispatching render calls. DXUT will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent ); int iY = 10; 
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );

    g_SampleUI.SetCallback( OnGUIEvent ); iY = 10; 

/*
    TODO: add UI controls as needed
    g_SampleUI.AddComboBox( 19, 35, iY += 24, 125, 22 );
    g_SampleUI.GetComboBox( 19 )->AddItem( L"Text1", NULL );
    g_SampleUI.GetComboBox( 19 )->AddItem( L"Text2", NULL );
    g_SampleUI.GetComboBox( 19 )->AddItem( L"Text3", NULL );
    g_SampleUI.GetComboBox( 19 )->AddItem( L"Text4", NULL );
    g_SampleUI.AddCheckBox( 21, L"Checkbox1", 35, iY += 24, 125, 22 );
    g_SampleUI.AddCheckBox( 11, L"Checkbox2", 35, iY += 24, 125, 22 );
    g_SampleUI.AddRadioButton( 12, 1, L"Radio1G1", 35, iY += 24, 125, 22 );
    g_SampleUI.AddRadioButton( 13, 1, L"Radio2G1", 35, iY += 24, 125, 22 );
    g_SampleUI.AddRadioButton( 14, 1, L"Radio3G1", 35, iY += 24, 125, 22 );
    g_SampleUI.GetRadioButton( 14 )->SetChecked( true ); 
    g_SampleUI.AddButton( 17, L"Button1", 35, iY += 24, 125, 22 );
    g_SampleUI.AddButton( 18, L"Button2", 35, iY += 24, 125, 22 );
    g_SampleUI.AddRadioButton( 15, 2, L"Radio1G2", 35, iY += 24, 125, 22 );
    g_SampleUI.AddRadioButton( 16, 2, L"Radio2G3", 35, iY += 24, 125, 22 );
    g_SampleUI.GetRadioButton( 16 )->SetChecked( true );
    g_SampleUI.AddSlider( 20, 50, iY += 24, 100, 22 );
    g_SampleUI.GetSlider( 20 )->SetRange( 0, 100 );
    g_SampleUI.GetSlider( 20 )->SetValue( 50 );
    g_SampleUI.AddEditBox( 20, L"Test", 35, iY += 24, 125, 32 );
*/
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
         pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
    {
        pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif

    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning();
    }

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnCreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnCreateDevice( pd3dDevice ) );
    
    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                         OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                         L"Arial", &g_pFont ) );

    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
    // shader debugger. Debugging vertex shaders requires either REF or software vertex 
    // processing, and debugging pixel shaders requires REF.  The 
    // D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
    // shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile 
    // against the next higher available software target, which ensures that the 
    // unoptimized shaders do not exceed the shader model limitations.  Setting these 
    // flags will cause slower rendering since the shaders will be unoptimized and 
    // forced into software.  See the DirectX documentation for more information about 
    // using the shader debugger.
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
    #ifdef DEBUG_VS
        dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
    #endif
    #ifdef DEBUG_PS
        dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
    #endif

    // Read the D3DX effect file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"GeometryTextures.fx" ) );

    // If this fails, there should be debug output as to 
    // they the .fx file failed to compile
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, 
                                        NULL, &g_pEffect, NULL ) );
    WCHAR strTexturePath[512] = L"";
    WCHAR *wszName;
    WCHAR wszBuf[MAX_PATH];

    strTexturePath[0]=0;
    wszName = wszBuf;
    MultiByteToWideChar( CP_ACP, 0, "terrain.dds", -1, wszBuf, MAX_PATH );
    wszBuf[MAX_PATH - 1] = L'\0';
    DXUTFindDXSDKMediaFileCch( strTexturePath, 512, wszName );
    D3DXCreateTextureFromFile( pd3dDevice, strTexturePath, &g_pTerrainTex );

    strTexturePath[0]=0;
    wszName = wszBuf;
    MultiByteToWideChar( CP_ACP, 0, "grass.dds", -1, wszBuf, MAX_PATH );
    wszBuf[MAX_PATH - 1] = L'\0';
    DXUTFindDXSDKMediaFileCch( strTexturePath, 512, wszName );
    D3DXCreateTextureFromFile( pd3dDevice, strTexturePath, &g_pGrassTex );

    g_HandleTechniqueTerrain = g_pEffect->GetTechniqueByName( "RenderTerrain" );
    g_HandleTechniqueGrass = g_pEffect->GetTechniqueByName( "RenderGrassVBGT" );
    g_HandleTechniqueRocks = g_pEffect->GetTechniqueByName( "RenderRocksVBGT" );

    // Setup the camera's view parameters
    D3DXVECTOR3 vecEye(0.0f, 5.0f, 0.0f);
    D3DXVECTOR3 vecAt (0.0f, 0.0f, 5.0f);
    g_Camera.SetViewParams( &vecEye, &vecAt );
    g_Camera.SetRotateButtons();
    g_DirWidget.SetButtonMask();
    g_DirWidget.SetLightDirection( D3DXVECTOR3(0,-1,0) );

    // create vertex buffers for terrain vertices
    pd3dDevice->CreateVertexBuffer( NUM_TERRAIN_TILES * NUM_TERRAIN_TILES * sizeof( TerrainVertex ), 0, 0, D3DPOOL_MANAGED, &g_pTerrainVB, 0 );    
    pd3dDevice->CreateIndexBuffer( ( NUM_TERRAIN_TILES - 1 ) * ( NUM_TERRAIN_TILES - 1 ) * 3 * 2 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pTerrainIB, 0 );
    
    setupHeightMap();
    
    TerrainVertex* pTV;
    TerrainVertex* pOrgTV;
    int            iVi = 0;
    WORD*          pI;
    const float    off = ( TERRAIN_TILE_SIZE * TERRAIN_TILE_SIZE ) / 0.5f;
    
    g_pTerrainVB->Lock(0,0,(void**)&pTV,0);
    g_pTerrainIB->Lock(0,0,(void**)&pI,0);
    pOrgTV = pTV;
      {
      for( int y = 0; y < NUM_TERRAIN_TILES; ++y )
        {
        for( int x = 0; x < NUM_TERRAIN_TILES; ++x )
          {
          pTV->pos    = D3DXVECTOR3( x * TERRAIN_TILE_SIZE - off, getRandomHeight( x,y ), y * TERRAIN_TILE_SIZE - off );
          pTV->normal = getNormal( x, y );
          pTV->tex    = D3DXVECTOR2( (float)x, (float)y );
          ++pTV;
          
          if( x < ( NUM_TERRAIN_TILES - 1 ) && y < ( NUM_TERRAIN_TILES - 1 ) )
            {
            *pI = WORD(iVi); ++pI;
            *pI = WORD(iVi + NUM_TERRAIN_TILES + 1); ++pI;
            *pI = WORD(iVi + 1 ); ++pI;
            *pI = WORD(iVi ); ++pI;
            *pI = WORD(iVi + NUM_TERRAIN_TILES ); ++pI;
            *pI = WORD(iVi + NUM_TERRAIN_TILES + 1 ); ++pI;
            }
            
          ++iVi;
          }
        }
        iVi = 0;
        // build array for easy to code collission detection
        for( int y = 0; y < NUM_TERRAIN_TILES; ++y )
        {
          for( int x = 0; x < NUM_TERRAIN_TILES; ++x )
          {
            if( x < ( NUM_TERRAIN_TILES - 1 ) && y < ( NUM_TERRAIN_TILES - 1 ) )
            {
            int f = int( 1000 * randFloat() ) % 3;
            Triangles[ x ][ y ][ 0 ][ f ] = pOrgTV[iVi];
            Triangles[ x ][ y ][ 0 ][ ( f + 1 ) %3 ] = pOrgTV[iVi + NUM_TERRAIN_TILES + 1];
            Triangles[ x ][ y ][ 0 ][ ( f + 2 ) %3 ] = pOrgTV[iVi + 1];
            f = int( 1000 * randFloat() ) % 3;
            Triangles[ x ][ y ][ 1 ][ f ] = pOrgTV[iVi];
            Triangles[ x ][ y ][ 1 ][ ( f + 1 ) %3 ] = pOrgTV[iVi + NUM_TERRAIN_TILES ];
            Triangles[ x ][ y ][ 1 ][ ( f + 2 ) %3 ] = pOrgTV[iVi + NUM_TERRAIN_TILES + 1 ];
            }

            ++iVi;
          }
        }
      }
    g_pTerrainVB->Unlock();
    g_pTerrainIB->Unlock();

    VBGTVertex* pV;

    // create vertex buffers for VBGT
    pd3dDevice->CreateVertexBuffer( NUM_GRASS_TUFTS * 8 * sizeof( VBGTVertex ), 0, 0, D3DPOOL_MANAGED, &g_pVBGTvb, 0 );    
    pd3dDevice->CreateIndexBuffer( NUM_GRASS_TUFTS * 12 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pVBGTib, 0 );

    g_pVBGTvb->Lock(0,0,(void**)&pV,0);
    g_pVBGTib->Lock(0,0,(void**)&pI,0);
      {
      // fill vertex buffer with in_iInstanceCount number grass tufts
      for( int i = 0; i < NUM_GRASS_TUFTS; ++i )
        {
        // compute random barycentric origin of instance i
        float b0 = randFloat(); // generate random number between 0.0f and 1.0f
        float b1 = ( 1.0f - b0 ) * randFloat();
        float b2 = 1 - b0 - b1;

        float randWidth = ( 0.5f + 0.5f * randFloat() ) * GRASS_TUFT_WIDTH;
        float randHeight = ( 0.5f + 0.5f * randFloat() ) * GRASS_TUFT_HEIGHT;

        // copy vertices of grass tuft instance i
        for( unsigned int vi = 0; vi < 8; ++vi )
          {
          static D3DXVECTOR3 vts[ 8 ] =
            {
            D3DXVECTOR3( -1.0f, 0.0f,  0.0f ),
            D3DXVECTOR3(  1.0f, 0.0f,  0.0f ),
            D3DXVECTOR3(  1.0f, 1.0f,  0.0f ),
            D3DXVECTOR3( -1.0f, 1.0f,  0.0f ),
            D3DXVECTOR3(  0.0f, 0.0f, -1.0f ),
            D3DXVECTOR3(  0.0f, 0.0f,  1.0f ),
            D3DXVECTOR3(  0.0f, 1.0f,  1.0f ),
            D3DXVECTOR3(  0.0f, 1.0f, -1.0f )
            };

          static D3DXVECTOR2 txs[ 8 ] =
            {
            D3DXVECTOR2( 0.0f, 1.0f ),
            D3DXVECTOR2( 1.0f, 1.0f ),
            D3DXVECTOR2( 1.0f, 0.0f ),
            D3DXVECTOR2( 0.0f, 0.0f ),
            D3DXVECTOR2( 0.0f, 1.0f ),
            D3DXVECTOR2( 1.0f, 1.0f ),
            D3DXVECTOR2( 1.0f, 0.0f ),
            D3DXVECTOR2( 0.0f, 0.0f )
            };

          float randCol0    = ( 0.1f + 0.9f * randFloat() );
          float randCol1    = ( 0.1f + 0.9f * randFloat() );

          pV->pos.x = randWidth * vts[vi].x;
          pV->pos.y = randHeight * vts[vi].y;
          pV->pos.z = randWidth * vts[vi].z;
          pV->col[0] = unsigned char( randCol0 * 255 );
          pV->col[1] = unsigned char( randCol1 * 255 );
          pV->col[2] = 0xff;
          pV->col[3] = 0xff;
          pV->tex    = txs[vi];

          // fill in barycentric coordinates of origin 
          pV->bary = D3DXVECTOR3( b0, b1, b2 );

          ++pV;
          }

        // generate indices of this instance of the grass tuft
        for( int ii = 0; ii < 12; ++ii )
          {
          static int idx[] = { 0,1,2,
                               0,2,3,
                               4,5,6,
                               4,6,7};
          *pI = WORD( idx[ii] + i * 8 ), ++pI;
          }
        }
      }
    g_pVBGTvb->Unlock();
    g_pVBGTib->Unlock();


    // create vertex buffers for VBGT
    pd3dDevice->CreateVertexBuffer( NUM_ROCKS * NUM_ROCK_SEGMENTS * NUM_ROCK_SEGMENTS * sizeof( VBGTVertex ), 0, 0, D3DPOOL_MANAGED, &g_pVBGTvb1, 0 );    
    pd3dDevice->CreateIndexBuffer( 2 * 3 * NUM_ROCKS * ( NUM_ROCK_SEGMENTS ) * ( NUM_ROCK_SEGMENTS - 1 )* sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pVBGTib1, 0 );

    g_pVBGTvb1->Lock(0,0,(void**)&pV,0);
    g_pVBGTib1->Lock(0,0,(void**)&pI,0);
    int vi = 0;
    {
      // fill vertex buffer with in_iInstanceCount number grass tufts
      for( int i = 0; i < NUM_ROCKS; ++i )
        {
        // compute random barycentric origin of instance i
        float b0 = randFloat(); // generate random number between 0.0f and 1.0f
        float b1 = ( 1.0f - b0 ) * randFloat();
        float b2 = 1 - b0 - b1;
        
        for( int y = 0; y < NUM_ROCK_SEGMENTS; ++y )
          {
          float height = 1.0f - 2.0f * ( (float)y/(float)(NUM_ROCK_SEGMENTS-1) );
          float beta = (22.0f/7.0f) * ( (float)y/(float)(NUM_ROCK_SEGMENTS-1) );
          float r = sinf(beta);//fabsf( 0.1f + randFloat() * 0.25f * sinf( beta ) );

          // generate vertices on a sphere that are displaced randomly
          for(int x = 0; x < NUM_ROCK_SEGMENTS; ++x )
            {
            float alpha = (22.0f/7.0f) * 2.0f * ( (float)x/(float)(NUM_ROCK_SEGMENTS-1) );
            D3DXVECTOR3 pos;
            pos.x = r * cosf( alpha );
            pos.z = r * sinf( alpha );
            pos.y = height;
            D3DXVec3Normalize( &pV->pos, &pos );
            pV->pos *= 0.25f + randFloat() * 0.08f;
            pV->col[0] =
            pV->col[1] =
            pV->col[2] =
            pV->col[3] = 0xff;
            pV->tex.x  = pV->pos.x;
            pV->tex.y  = pV->pos.z + pV->pos.y;

            // fill in barycentric coordinates of origin 
            pV->bary = D3DXVECTOR3( b0, b1, b2 );

            if( x < NUM_ROCK_SEGMENTS - 1 && y < NUM_ROCK_SEGMENTS - 1)
              {
              *pI = WORD( vi + 1 ), ++pI;
              *pI = WORD( vi + NUM_ROCK_SEGMENTS ), ++pI;
              *pI = WORD( vi ), ++pI;
              *pI = WORD( vi + 1), ++pI;
              *pI = WORD( vi + NUM_ROCK_SEGMENTS + 1 ), ++pI;
              *pI = WORD( vi + NUM_ROCK_SEGMENTS ), ++pI;
              }
            else if( x < NUM_ROCK_SEGMENTS && y < NUM_ROCK_SEGMENTS - 1 )
              {
              *pI = WORD( vi - ( NUM_ROCK_SEGMENTS - 1 ) ), ++pI;
              *pI = WORD( vi + NUM_ROCK_SEGMENTS ), ++pI;
              *pI = WORD( vi ), ++pI;
              *pI = WORD( vi - ( NUM_ROCK_SEGMENTS - 1 ) ), ++pI;
              *pI = WORD( vi + NUM_ROCK_SEGMENTS - ( NUM_ROCK_SEGMENTS - 1 ) ), ++pI;
              *pI = WORD( vi + NUM_ROCK_SEGMENTS ), ++pI;
              }
            ++pV;
            ++vi;
            }
          }
        }
      }
    g_pVBGTvb1->Unlock();
    g_pVBGTib1->Unlock();

    SAFE_RELEASE( g_pTerrainVertexDecl );
    pd3dDevice->CreateVertexDeclaration( g_TerrainVertexElem, &g_pTerrainVertexDecl );

    SAFE_RELEASE( g_pVBGTVertexDecl );
    pd3dDevice->CreateVertexDeclaration( g_VBGTVertexElem, &g_pVBGTVertexDecl );

    CDXUTDirectionWidget::StaticOnCreateDevice( pd3dDevice );
    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnResetDevice() );
    V_RETURN( g_SettingsDlg.OnResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 3000.0f );
    //g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width-170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width-170, pBackBufferSurfaceDesc->Height-350 );
    g_SampleUI.SetSize( 170, 300 );

    g_DirWidget.OnResetDevice( pBackBufferSurfaceDesc );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
  // Update the camera's position based on user input 
  g_Camera.FrameMove( fElapsedTime );
  
  // get camera pos
  D3DXVECTOR3 pos = *g_Camera.GetEyePt();

  // clear list of relevant terrain triangles
  g_TriangleList.clear();

  // find all triangle near the camera
  // this is not how it should be done ...there are more efficient ways to find
  // the triangle set
  for( int y = 0; y < NUM_TERRAIN_TILES - 1; ++y )
  {
    bool bCheckCol = false;
    
    for( int t = 0; t < 2; ++t )
    {
      for( int v = 0; v < 3; ++ v )
      {
        if( fabsf( Triangles[0][y][t][v].pos.z - pos.z ) <= ( 2.0f * TERRAIN_TILE_SIZE ) )
        {
        bCheckCol = true;
        break;
        }
      }
      
    if( bCheckCol )
      break;
    }
    
    if( bCheckCol )
    {
      for( int x = 0; x < NUM_TERRAIN_TILES - 1; ++x )
      {
        for( int t = 0; t < 2; ++t )
        {
          for( int v = 0; v < 3; ++ v )
          {
            if( fabsf( Triangles[x][y][t][v].pos.x - pos.x ) <= ( 2.0f * TERRAIN_TILE_SIZE ) )
            {
            Triangle tri;

            // fill triangle struct              
            tri.vts[0] = Triangles[x][y][t][0].pos;
            tri.vts[1] = Triangles[x][y][t][1].pos;
            tri.vts[2] = Triangles[x][y][t][2].pos;
            tri.ns[0] = Triangles[x][y][t][0].normal;
            tri.ns[1] = Triangles[x][y][t][1].normal;
            tri.ns[2] = Triangles[x][y][t][2].normal;
            
            // add triangle to list of relevant triangles
            g_TriangleList.push_back( tri );
              
            break; // we don't need to check other vertices
            }
          }
        }
      }
    }
  }
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;
    D3DXMATRIXA16 mViewProjection;
    
    D3DXVECTOR3 pos = *g_Camera.GetEyePt();

    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        D3DXVECTOR4 white(1,1,1,1);
        D3DXVECTOR4 grey(0.3f,0.3f,0.3f,1);
        D3DXVECTOR3 lightdir = g_DirWidget.GetLightDirection();
        DWORD cull;
        
        pd3dDevice->GetRenderState(D3DRS_CULLMODE,&cull);
        
        // Get the projection & view matrix from the camera class
        mProj = *g_Camera.GetProjMatrix();
        mView = *g_Camera.GetViewMatrix();

        mViewProjection = mView * mProj;
        
        // Update the effect's variables.  Instead of using strings, it would 
        // be more efficient to cache a handle to the parameter by calling 
        // ID3DXEffect::GetParameterByName
        V( g_pEffect->SetMatrix( "g_mModelViewProjection", &mViewProjection ) );
        V( g_pEffect->SetFloat( "g_fTime", (float)fTime ) );
        V( g_pEffect->SetFloat( "g_fNear", (float)g_Camera.GetNearClip() ) );
        V( g_pEffect->SetFloat( "g_fFar", (float)g_Camera.GetFarClip() ) );
        V( g_pEffect->SetVector( "g_MaterialAmbientColor", (D3DXVECTOR4*)&grey ) );
        V( g_pEffect->SetVector( "g_MaterialDiffuseColor", (D3DXVECTOR4*)&white ) );
        V( g_pEffect->SetVector( "g_LightDiffuse", (D3DXVECTOR4*)&white ) );
        V( g_pEffect->SetFloatArray( "g_LightDir", (float*)&lightdir, 3 ) );
        V( g_pEffect->SetFloatArray( "g_Eye", (float*)&pos, 3 ) );

        V( g_pEffect->SetTexture( "g_TerrainTexture", g_pTerrainTex ) );
        V( g_pEffect->SetTexture( "g_GrassTexture", g_pGrassTex ) );
        V( pd3dDevice->SetVertexDeclaration( g_pTerrainVertexDecl ));
        V( g_pEffect->SetTechnique( g_HandleTechniqueTerrain ) );
        V( g_pEffect->CommitChanges() );

        //pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

        DWORD NumTris  = 2 * ( NUM_TERRAIN_TILES - 1 ) * ( NUM_TERRAIN_TILES - 1 );
        DWORD NumVerts = NUM_TERRAIN_TILES * NUM_TERRAIN_TILES;
        UINT cPasses;
        V( g_pEffect->Begin( &cPasses, 0 ) );
        for( UINT p = 0; p < cPasses; ++p )
          {
          V( g_pEffect->BeginPass( p ) );
            {
            V( pd3dDevice->SetStreamSource( 0, g_pTerrainVB, 0, sizeof(TerrainVertex)) );
            V( pd3dDevice->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | 1ul ) );

            V( pd3dDevice->SetIndices( g_pTerrainIB ) );

            // draw terrain mesh
            V( pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, NumVerts, 0, NumTris ) );
            }
         V( g_pEffect->EndPass() );
         }
        V( g_pEffect->End() );

        V( pd3dDevice->SetVertexDeclaration( g_pVBGTVertexDecl ));
        V( pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)63) );
        V( pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ); 
        V( pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL) );
        V( pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE) );

        V( pd3dDevice->SetStreamSource( 0, g_pVBGTvb, 0, sizeof(VBGTVertex)) );
        V( pd3dDevice->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | 1ul ) );
        V( pd3dDevice->SetIndices( g_pVBGTib ) );

        V( g_pEffect->SetTechnique( g_HandleTechniqueGrass ) );

        //pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

        for( TriIt it = g_TriangleList.begin(); it != g_TriangleList.end(); ++ it )
          {
          Triangle& t = (*it);
          
          V( g_pEffect->SetFloatArray( "g_TriangleV0", (float*)&t.vts[0], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleV1", (float*)&t.vts[1], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleV2", (float*)&t.vts[2], 3 ) );

          V( g_pEffect->SetFloatArray( "g_TriangleN0", (float*)&t.ns[0], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleN1", (float*)&t.ns[1], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleN2", (float*)&t.ns[2], 3 ) );

          V( pd3dDevice->SetStreamSource( 0, g_pVBGTvb, 0, sizeof(VBGTVertex)) );
          V( pd3dDevice->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | 1ul ) );
          V( pd3dDevice->SetIndices( g_pVBGTib ) );

          V( g_pEffect->CommitChanges() );

          V( g_pEffect->Begin( &cPasses, 0 ) );
          for( UINT p = 0; p < cPasses; ++p )
            {
            V( g_pEffect->BeginPass( p ) );
              {
              DWORD NumTris  = 4 * NUM_GRASS_TUFTS;
              DWORD NumVerts = 8 * NUM_GRASS_TUFTS;

              // draw grass VBGT
              V( pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, NumVerts, 0, NumTris ) );
              }
            V( g_pEffect->EndPass() );
            }
          V( g_pEffect->End() );
          }                    

        V( pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE) ); 
        V( pd3dDevice->SetRenderState(D3DRS_CULLMODE,cull) );
        V( pd3dDevice->SetStreamSource( 0, g_pVBGTvb1, 0, sizeof(VBGTVertex)) );
        V( pd3dDevice->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | 1ul ) );
        V( pd3dDevice->SetIndices( g_pVBGTib1 ) );


        V( g_pEffect->SetTechnique( g_HandleTechniqueRocks ) );

        for( TriIt it = g_TriangleList.begin(); it != g_TriangleList.end(); ++ it )
          {
          Triangle& t = (*it);
          
          V( g_pEffect->SetFloatArray( "g_TriangleV0", (float*)&t.vts[0], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleV1", (float*)&t.vts[1], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleV2", (float*)&t.vts[2], 3 ) );

          V( g_pEffect->SetFloatArray( "g_TriangleN0", (float*)&t.ns[0], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleN1", (float*)&t.ns[1], 3 ) );
          V( g_pEffect->SetFloatArray( "g_TriangleN2", (float*)&t.ns[2], 3 ) );

          // set vertices ....   
          V( g_pEffect->CommitChanges() );

          V( g_pEffect->Begin( &cPasses, 0 ) );
          for( UINT p = 0; p < cPasses; ++p )
            {
            V( g_pEffect->BeginPass( p ) );
              {
              NumTris  = 2 * NUM_ROCKS * ( NUM_ROCK_SEGMENTS ) * ( NUM_ROCK_SEGMENTS - 1 );
              NumVerts = NUM_ROCK_SEGMENTS * NUM_ROCK_SEGMENTS;

              V( pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, NumVerts, 0, NumTris ) );
              }
            V( g_pEffect->EndPass() );
            }
          V( g_pEffect->End() );
          }                    

        DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" ); // These events are to help PIX identify what the code is doing
        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_SampleUI.OnRender( fElapsedTime ) );
        DXUT_EndPerfEvent();

        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats() );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

/*
    TODO: add UI text as needed
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( L"Put whatever misc status here" );
    
    // Draw help
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetBackBufferSurfaceDesc();
    if( g_bShowHelp )
    {
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*6 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 0.75f, 0.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Controls (F1 to hide):" );

        txtHelper.SetInsertionPos( 40, pd3dsdBackBuffer->Height-15*5 );
        txtHelper.DrawTextLine( L"Quit: ESC" );
    }
    else
    {
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height-15*2 );
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }
*/
    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
    g_DirWidget.HandleMessages( hWnd, uMsg, wParam, lParam );

    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: g_bShowHelp = !g_bShowHelp; break;
            case VK_UP: 
            case VK_DOWN:
            case VK_LEFT:
            case VK_RIGHT:
            break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN: DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:        DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:     g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnLostDevice();
    g_SettingsDlg.OnLostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();
    if( g_pEffect )
        g_pEffect->OnLostDevice();
    SAFE_RELEASE( g_pTextSprite );
    CDXUTDirectionWidget::StaticOnLostDevice();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnDestroyDevice();
    g_SettingsDlg.OnDestroyDevice();
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pTerrainTex );
    SAFE_RELEASE( g_pGrassTex );
    SAFE_RELEASE( g_pFont );
    SAFE_RELEASE( g_pTerrainVertexDecl );
    SAFE_RELEASE( g_pTerrainVB );
    SAFE_RELEASE( g_pTerrainIB );
    SAFE_RELEASE( g_pVBGTVertexDecl );
    SAFE_RELEASE( g_pVBGTvb );
    SAFE_RELEASE( g_pVBGTib );
    SAFE_RELEASE( g_pVBGTvb1 );
    SAFE_RELEASE( g_pVBGTib1 );
    CDXUTDirectionWidget::StaticOnDestroyDevice();
}



