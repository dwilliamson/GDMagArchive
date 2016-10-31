//--------------------------------------------------------------------------------------
// File: GeometryTextures.fx
//
// The effect file for the GeometryTextures sample.  
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

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
float3 g_LightDir;                  // Light's direction in world space
float4 g_LightDiffuse;              // Light's diffuse color
texture g_TerrainTexture;           // Color texture for terrain
texture g_GrassTexture;             // Color texture for grass VBGT
float   g_fTime;                    // App's time in seconds
float4x4 g_mModelViewProjection;    // World * View * Projection matrix
float3  g_TriangleV0;
float3  g_TriangleV1;
float3  g_TriangleV2;
float3  g_TriangleN0;
float3  g_TriangleN1;
float3  g_TriangleN2;
float3  g_Eye;
float   g_fNear;
float   g_fFar;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler TerrainTextureSampler = 
sampler_state
{
    Texture = <g_TerrainTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler GrassTextureSampler = 
sampler_state
{
    Texture = <g_GrassTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderTerrainVS( float3 vPos : POSITION, 
                           float3 vNormal : NORMAL,
                           float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(float4(vPos,1), g_mModelViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(vNormal); // normal (world space)

    // Calc diffuse color    
    Output.Diffuse.rgb = g_MaterialDiffuseColor * g_LightDiffuse * max(0,dot(vNormalWorldSpace, g_LightDir)) + 
                         g_MaterialAmbientColor;   
    Output.Diffuse.a = 1.0f; 
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderTerrainPS( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;

    // Lookup mesh texture and modulate it with diffuse
    Output.RGBColor = tex2D(TerrainTextureSampler, In.TextureUV) * In.Diffuse;
    
    return Output;
}

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderGrassVBGTVS( float3 vOffIn : POSITION,
						float4 vCol : COLOR0, 
                        float2 vTexCoord0 : TEXCOORD0,
                        float3 vBaryCoords: TEXCOORD1 )
{
    VS_OUTPUT Output;
    float3 vPos;
    float3 vNormal;
    float3 vNormalWorldSpace;
    float3 vOff = vOffIn;
    
    vOff.x += 0.1 * sin( g_fTime + vOff.x ) * vOff.y; 
    vOff.z += 0.1 * cos( g_fTime + vOff.z ) * vOff.y; 

    vPos = vBaryCoords.x * g_TriangleV0 +
		   vBaryCoords.y * g_TriangleV1 +
		   vBaryCoords.z * g_TriangleV2 +
		   vOff;
    
    vNormal = vBaryCoords.x * g_TriangleN0 +
		      vBaryCoords.y * g_TriangleN1 +
		      vBaryCoords.z * g_TriangleN2;

    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(float4(vPos,1), g_mModelViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(vNormal); // normal (world space)

    // Calc diffuse color    
    Output.Diffuse.rgb = vCol.rgb * g_LightDiffuse * max(0,dot(vNormalWorldSpace, g_LightDir)) + 
                         g_MaterialAmbientColor;   
    Output.Diffuse.a = 1.0 - 0.03 * Output.Position.z; 
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderGrassVBGTPS( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;

    // Lookup mesh texture and modulate it with diffuse
    Output.RGBColor = tex2D(GrassTextureSampler, In.TextureUV).ggga * In.Diffuse;
    
    Output.RGBColor.rgb += float3(0,0.2,0.0);

    return Output;
}

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderRocksVBGTVS( float3 vOffIn : POSITION,
						float4 vCol : COLOR0, 
                        float2 vTexCoord0 : TEXCOORD0,
                        float3 vBaryCoords: TEXCOORD1 )
{
    VS_OUTPUT Output;
    float3 vPos;
    float3 vNormal;
    float3 vNormalWorldSpace;
    float3 vOff = vOffIn;
    
    vPos = vBaryCoords.x * g_TriangleV0 +
		   vBaryCoords.y * g_TriangleV1 +
		   vBaryCoords.z * g_TriangleV2 +
		   vOff;
    
    vNormal = vBaryCoords.x * g_TriangleN0 +
		      vBaryCoords.y * g_TriangleN1 +
		      vBaryCoords.z * g_TriangleN2;

    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(float4(vPos,1), g_mModelViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(vNormal); // normal (world space)

    // Calc diffuse color    
    Output.Diffuse.rgb = vCol.rgb * g_LightDiffuse * max(0,dot(vNormalWorldSpace, g_LightDir)) + 
                         g_MaterialAmbientColor;   
    Output.Diffuse.a = 1.0f - ( 100.0 * ( Output.Position.w - g_fNear ) / ( g_fFar - g_fNear ) ); 
    
    // Just copy the texture coordinate through
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
// color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderRocksVBGTPS( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;
    float fade = 1.0 - In.Diffuse.a * 0.25;

    // Lookup mesh texture and modulate it with diffuse
    Output.RGBColor = float4( fade, fade, fade, 1 ) * tex2D(TerrainTextureSampler, In.TextureUV) * float4( In.Diffuse.rgb, 1 );
    
    return Output;
}

//--------------------------------------------------------------------------------------
// Renders terrain 
//--------------------------------------------------------------------------------------
technique RenderTerrain
{
    pass P0
    {          
        VertexShader = compile vs_1_1 RenderTerrainVS();
        PixelShader  = compile ps_1_1 RenderTerrainPS(); 
    }
}

//--------------------------------------------------------------------------------------
// Renders Grass VBGT 
//--------------------------------------------------------------------------------------
technique RenderGrassVBGT
{
    pass P0
    {          
        VertexShader = compile vs_1_1 RenderGrassVBGTVS();
        PixelShader  = compile ps_1_1 RenderGrassVBGTPS(); 
    }
}

//--------------------------------------------------------------------------------------
// Renders Rock VBGT 
//--------------------------------------------------------------------------------------
technique RenderRocksVBGT
{
    pass P0
    {          
        VertexShader = compile vs_1_1 RenderRocksVBGTVS();
        PixelShader  = compile ps_1_1 RenderRocksVBGTPS(); 
    }
}
