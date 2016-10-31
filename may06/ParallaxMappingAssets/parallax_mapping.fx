string description = "Parallax Mapping";

float tile
<
	string UIName = "Tile Factor";
	string UIWidget = "slider";
	float UIMin = 1.0;
	float UIStep = 1.0;
	float UIMax = 32.0;
> = 2;

float hole_depth
<
	string UIName = "Hole depth";
	string UIWidget = "slider";
	float UIMin = 0.0f;
	float UIStep = 0.01f;
	float UIMax = 1.0f;
> = 0.20;

float3 ambient
<
	string UIName = "Ambient Light";
	string UIWidget = "color";
> = {0.2,0.2,0.2};

float3 diffuse
<
	string UIName = "Diffuse Light";
	string UIWidget = "color";
> = {1,1,1};

float3 specular
<
	string UIName = "Specular";
	string UIWidget = "color";
> = {0.75,0.75,0.75};

float shine
<
    string UIName = "Shine Exponent";
	string UIWidget = "slider";
	float UIMin = 8.0f;
	float UIStep = 8;
	float UIMax = 256.0f;
> = 128.0;

float3 lightpos : POSITION
<
	string UIName="Light Position";
> = { -150.0, 200.0, -125.0 };

float4x4 mvpMatrix : WorldViewProjection;
float4x4 worldViewMatrix : WorldView;
float4x4 modelinv : WorldInverse;
float4x4 view : View;

texture texmap : DIFFUSE
<
	string UIName = "Diffuse Map";
    string ResourceName = "Plane01DiffuseMap.tga";
    string ResourceType = "2D";
>;

texture reliefmap : NORMAL
<
	string UIName = "Relief Map";
    string ResourceName = "Plane01NormalsMap.tga";
    string ResourceType = "2D";
>;

sampler2D texmap_sampler = sampler_state
{
	Texture = <texmap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D reliefmap_sampler = sampler_state
{
	Texture = <reliefmap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

struct a2v 
{
    float4 pos		: POSITION;
    float4 color	: COLOR0;
    float3 normal	: NORMAL;
    float2 TexCoord0	: TEXCOORD0;
    float3 tangent	: TANGENT0;
    float3 binormal	: BINORMAL0;
};

struct v2f
{
    float4 Position    		: POSITION;
	float4 color	: COLOR0;
    float2 TexCoord0	: TEXCOORD0;
    float3 TexCoord1	: TEXCOORD1;
    float3 tangent	: TEXCOORD2;
    float3 binormal	: TEXCOORD3;
    float3 normal	: TEXCOORD4;
	float4 lightpos	: TEXCOORD5;
};

v2f view_space(a2v IN)
{
	v2f OUT;

	// vertex position in object space
	float4 pos=float4(IN.pos.x,IN.pos.y,IN.pos.z,1.0);

	// compute worldViewMatrix rotation only part
	float3x3 mvRot;
	mvRot[0]=worldViewMatrix[0].xyz;
	mvRot[1]=worldViewMatrix[1].xyz;
	mvRot[2]=worldViewMatrix[2].xyz;

	// vertex position in clip space
	OUT.Position=mul(pos,mvpMatrix);

	// vertex position in view space (with model transformations)
	OUT.TexCoord1=mul(pos,worldViewMatrix).xyz;

	// light position in view space
	float4 lp=float4(lightpos.x,lightpos.y,lightpos.z,1);
	OUT.lightpos=mul(lp,view);

	// tangent space vectors in view space (with model transformations)
	OUT.tangent=mul(IN.tangent,mvRot);
	OUT.binormal=mul(IN.binormal,mvRot);
	OUT.normal=mul(IN.normal,mvRot);
	
	// copy color and texture coordinates
	OUT.color=IN.color;
	OUT.TexCoord0=IN.TexCoord0.xy;

	return OUT;
}




float4 normal_map(
	v2f IN,
	uniform sampler2D texmap,
	uniform sampler2D reliefmap) : COLOR
{
	float4 normal=tex2D(reliefmap,IN.TexCoord0*tile);
	normal.xy=normal.xy*2.0-1.0; // trafsform to [-1,1] range
	normal.z=sqrt(1.0-dot(normal.xy,normal.xy)); // compute z component
	
	// transform normal to world space
       normal.xyz=normalize(normal.x*IN.tangent-normal.y*IN.binormal+normal.z*IN.normal);
	
	// color map
	float4 color=tex2D(texmap,IN.TexCoord0*tile);

	// view and light directions
	float3 v = normalize(IN.TexCoord1);
	float3 l = normalize(IN.lightpos.xyz-IN.TexCoord1);

	// compute diffuse and specular terms
	float att=saturate(dot(l,IN.normal.xyz));
	float diff=saturate(dot(l,normal.xyz));
	float spec=saturate(dot(normalize(l-v),normal.xyz));

	// compute final color
	float4 finalcolor;
	finalcolor.xyz=ambient*color.xyz+
		att*(color.xyz*diffuse.xyz*diff+specular.xyz*pow(spec,shine));
	finalcolor.w=color.w;
	
	return finalcolor;
}

float4 parallax_map(
	v2f IN,
	uniform sampler2D texmap,
	uniform sampler2D reliefmap) : COLOR
{
   	// view and light directions
	float3 v = normalize(IN.TexCoord1);                                     // v = unit vector towards the viewer
	float3 l = normalize(IN.lightpos.xyz-IN.TexCoord1);                     // l = unit vector from the light to the viewer

	float2 uv = IN.TexCoord0*tile;                                          // uv = regular texture coordinates

        // Get depth from the alpha (w) of the relief map
        // and scale
	float depth = (tex2D(reliefmap,uv).w) * hole_depth;    

        // Get the alpha from the original pixel to prevent holes at extreme angles
        float orig_alpha = tex2D(texmap,uv).w;

	// For use in Max (Right Handed) 
        // Right handed is b,t,n
	//float3x3 to_tangent_space = float3x3(IN.binormal,IN.tangent,IN.normal);

        // For use most engines, and FX Composer
        // Left handed is t,n,b
	float3x3 to_tangent_space = float3x3(IN.tangent,IN.binormal,IN.normal);     

                                             
        // transform the vector from the pixel to the viewer into tangent space
        // then simply by taking xy, we project it onto the surface of the polygon
        // which gives us a direction in which to offset the uv
        // we then scale that by the depth at the original point                               
        float2 off = depth * mul(to_tangent_space,v);        

        // get a new uv
	uv += off;

	// use the new UV to get a normal
        // note this means we read the reliefmap twice, once for the height
        // and then for the normal that matches the pixel in the texture map
	float4 normal=tex2D(reliefmap,uv);
	normal.xy=normal.xy*2.0-1.0; // trafsform to [-1,1] range
	normal.z=sqrt(1.0-dot(normal.xy,normal.xy)); // compute z component

        // The normal is in tangent space, so transform back to world space
	// transform normal to world space
	normal.xyz=normalize(normal.x*IN.tangent-normal.y*IN.binormal+normal.z*IN.normal);

	// color map
	float4 color=tex2D(texmap,uv);

	// compute diffuse and specular terms
	float att=saturate(dot(l,IN.normal.xyz));
	float diff=saturate(dot(l,normal.xyz));
	float spec=saturate(dot(normalize(l-v),normal.xyz));

	// compute final color
	float4 finalcolor;

	finalcolor.xyz=ambient*color.xyz+
		att*(color.xyz*diffuse.xyz*diff+specular.xyz*pow(spec,shine));

        // Test, just use the original color
        //finalcolor.xyz = color.xyz;

        //Test, use the computed world spacenormal
	//finalcolor.xyz = normal;

        // copy alpha from the original texture
	//finalcolor.w=color.w;
    
        // Test, color by offset, should get shades of red - yellow - green - black,     
        // (negative numbers are set to zero)
        //finalcolor.xy = ((off ) / hole_depth) * 4;
        //finalcolor.z = 0;
        
        // Test, color by view normal
        // will be blue in the middle of the sceen (0,0,1)
        // tending towards 
        //finalcolor.xyz = v.xyz;
        //finalcolor.x = 0;
        

	finalcolor.w=orig_alpha;

	return finalcolor;
}


// Specific to the concave faceted geometry of our bullet hole 
float4 parallax_bullet_map(
	v2f IN,
	uniform sampler2D texmap,
	uniform sampler2D reliefmap) : COLOR
{
   	// view and light directions
	float3 v = normalize(IN.TexCoord1);                                     // v = unit vector towards the viewer
	float3 l = normalize(IN.lightpos.xyz-IN.TexCoord1);                     // l = unit vector from the light to the viewer

	float2 uv = IN.TexCoord0*tile;                                          // uv = regular texture coordinates

        // Get the alpha from the original pixel to prevent holes at extreme angles
        float orig_alpha = tex2D(texmap,uv).w;

        // Get depth from the alpha (w) of the relief map
        // and scale
	//float depth = (tex2D(reliefmap,uv).w) * hole_depth;    
	//float depth = 1.0f * hole_depth;    

	// For use in Max (Right Handed) 
        // Right handed is b,t,n
	float3x3 to_tangent_space = float3x3(IN.binormal,IN.tangent,IN.normal);

        // For use most engines, and FX Composer
        // Left handed is t,n,b
	//float3x3 to_tangent_space = float3x3(IN.tangent,IN.normal,IN.binormal);     

                                             
        // transform the vector from the pixel to the viewer into tangent space
        // then simply by taking xy, we project it onto the surface of the polygon
        // which gives us a direction in which to offset the uv
        // we then scale that by the depth at the original point                               

//        float2 uv2 = uv + offset;
//        float depth2 = (tex2D(reliefmap,uv2).w);
       
        // now we've assumed a depth of 1, 
        // if this gives us a point on the base then we want to use it
        // otherwise, we redo the depth calculation with the "correct" depth
        
        float2 offset = hole_depth * mul(to_tangent_space,v);        
        if (tex2D(reliefmap,uv+offset).w < 0.96)
        {
	   offset *= (tex2D(reliefmap,uv).w);          
        }

        // get a new uv
	uv += offset;

	// use the new UV to get a normal
        // note this means we read the reliefmap twice, once for the height
        // and then for the normal that matches the pixel in the texture map
	float4 normal=tex2D(reliefmap,uv);
	normal.xy=normal.xy*2.0-1.0; // trafsform to [-1,1] range
	normal.z=sqrt(1.0-dot(normal.xy,normal.xy)); // compute z component

        // The normal is in tangent space, so transform back to world space
	// transform normal to world space
	normal.xyz=normalize(normal.x*IN.tangent-normal.y*IN.binormal+normal.z*IN.normal);

	// color map
	float4 color=tex2D(texmap,uv);

	// compute diffuse and specular terms
	float att=saturate(dot(l,IN.normal.xyz));
	float diff=saturate(dot(l,normal.xyz));
	float spec=saturate(dot(normalize(l-v),normal.xyz));

	// compute final color
	float4 finalcolor;

	finalcolor.xyz=ambient*color.xyz+
		att*(color.xyz*diffuse.xyz*diff+specular.xyz*pow(spec,shine));

        // Test, just use the original color
        //finalcolor.xyz = color.xyz;

        //Test, use the computed world spacenormal
	//finalcolor.xyz = normal;

        // copy alpha from the original texture
	//finalcolor.w=color.w;
    
        // Test, color by offset, should get shades of red - yellow - green - black,     
        // (negative numbers are set to zero)
        //finalcolor.xy = ((off ) / hole_depth) * 4;
        //finalcolor.z = 0;
        
        // Test, color by view normal
        // will be blue in the middle of the sceen (0,0,1)
        // tending towards 
        //finalcolor.xyz = v.xyz;
        //finalcolor.x = 0;
        

	finalcolor.w=orig_alpha;

	return finalcolor;
}


// Specific to the concave faceted geometry of our bullet hole 
float4 parallax_average_map(
	v2f IN,
	uniform sampler2D texmap,
	uniform sampler2D reliefmap) : COLOR
{
   	// view and light directions
	float3 v = normalize(IN.TexCoord1);                                     // v = unit vector towards the viewer
	float3 l = normalize(IN.lightpos.xyz-IN.TexCoord1);                     // l = unit vector from the light to the viewer

	float2 uv = IN.TexCoord0*tile;                                          // uv = regular texture coordinates

        // Get the alpha from the original pixel to prevent holes at extreme angles
        float orig_alpha = tex2D(texmap,uv).w;

        // Get depth from the alpha (w) of the relief map
        // and scale
	//float depth = (tex2D(reliefmap,uv).w) * hole_depth;    
	//float depth = 1.0f * hole_depth;    

	// For use in Max (Right Handed) 
        // Right handed is b,t,n
	float3x3 to_tangent_space = float3x3(IN.binormal,IN.tangent,IN.normal);

        // For use most engines, and FX Composer
        // Left handed is t,n,b
	//float3x3 to_tangent_space = float3x3(IN.tangent,IN.normal,IN.binormal);     

                                             
        // transform the vector from the pixel to the viewer into tangent space
        // then simply by taking xy, we project it onto the surface of the polygon
        // which gives us a direction in which to offset the uv
        // we then scale that by the depth at the original point                               

//        float2 uv2 = uv + offset;
//        float depth2 = (tex2D(reliefmap,uv2).w);
       
        // now we've assumed a depth of 1, 
        // if this gives us a point on the base then we want to use it
        // otherwise, we redo the depth calculation with the "correct" depth
        
        float2 offset = hole_depth * mul(to_tangent_space,v);        
        float depth_at_1 = tex2D(reliefmap,uv+offset).w;
        if ( depth_at_1 < 0.96f)
        {
	   offset *= (depth_at_1 + (tex2D(reliefmap,uv).w)) * 0.5;          
        }

        // get a new uv
	uv += offset;

	// use the new UV to get a normal
        // note this means we read the reliefmap twice, once for the height
        // and then for the normal that matches the pixel in the texture map
	float4 normal=tex2D(reliefmap,uv);
	normal.xy=normal.xy*2.0-1.0; // trafsform to [-1,1] range
	normal.z=sqrt(1.0-dot(normal.xy,normal.xy)); // compute z component

        // The normal is in tangent space, so transform back to world space
	// transform normal to world space
	normal.xyz=normalize(normal.x*IN.tangent-normal.y*IN.binormal+normal.z*IN.normal);

	// color map
	float4 color=tex2D(texmap,uv);

	// compute diffuse and specular terms
	float att=saturate(dot(l,IN.normal.xyz));
	float diff=saturate(dot(l,normal.xyz));
	float spec=saturate(dot(normalize(l-v),normal.xyz));

	// compute final color
	float4 finalcolor;

	finalcolor.xyz=ambient*color.xyz+
		att*(color.xyz*diffuse.xyz*diff+specular.xyz*pow(spec,shine));

        // Test, just use the original color
        //finalcolor.xyz = color.xyz;

        //Test, use the computed world spacenormal
	//finalcolor.xyz = normal;

        // copy alpha from the original texture
	//finalcolor.w=color.w;
    
        // Test, color by offset, should get shades of red - yellow - green - black,     
        // (negative numbers are set to zero)
        //finalcolor.xy = ((offset ) / hole_depth) * 4;
        //finalcolor.z = 0;
        
        // Test, color by view normal
        // will be blue in the middle of the sceen (0,0,1)
        // tending towards 
        //finalcolor.xyz = v.xyz;
        //finalcolor.x = 0;
        

	finalcolor.w=orig_alpha;

	return finalcolor;
}

// Specific to the concave faceted geometry of our bullet hole 
float4 parallax_3dsmax_map(
	v2f IN,
	uniform sampler2D texmap,
	uniform sampler2D reliefmap) : COLOR
{
   	// view and light directions
	float3 v = normalize(IN.TexCoord1);                                     // v = unit vector towards the viewer
	float3 l = normalize(IN.lightpos.xyz-IN.TexCoord1);                     // l = unit vector from the light to the viewer

	float2 uv = IN.TexCoord0*tile;                                          // uv = regular texture coordinates

        // Get the alpha from the original pixel to prevent holes at extreme angles
        float orig_alpha = tex2D(texmap,uv).w;

        // Get depth from the alpha (w) of the relief map
        // and scale
	//float depth = (tex2D(reliefmap,uv).w) * hole_depth;    
	//float depth = 1.0f * hole_depth;    

	// For use in Max (Right Handed) 
        // Right handed is b,t,n
	float3x3 to_tangent_space = float3x3(IN.binormal,IN.tangent,IN.normal);

        // For use most engines, and FX Composer
        // Left handed is t,b,n
	//float3x3 to_tangent_space = float3x3(IN.tangent,IN.binormal,IN.normal);     

                                             
        // transform the vector from the pixel to the viewer into tangent space
        // then simply by taking xy, we project it onto the surface of the polygon
        // which gives us a direction in which to offset the uv
        // we then scale that by the depth at the original point                               

//        float2 uv2 = uv + offset;
//        float depth2 = (tex2D(reliefmap,uv2).w);
       
        // now we've assumed a depth of 1, 
        // if this gives us a point on the base then we want to use it
        // otherwise, we redo the depth calculation with the "correct" depth
        
        float2 offset_direction = hole_depth * mul(to_tangent_space,v);        
        float2 offset = offset_direction;   // implied * 1.0      
        float depth_at_1 = tex2D(reliefmap,uv+offset).w;
        if ( depth_at_1 < 0.96f)
        {
	   offset = offset_direction * (depth_at_1 + (tex2D(reliefmap,uv).w)) * 0.5;          
        }
        

        // get a new uv
	uv += offset;

	// use the new UV to get a normal
        // note this means we read the reliefmap twice, once for the height
        // and then for the normal that matches the pixel in the texture map
	float4 normal=tex2D(reliefmap,uv);
	normal.xy=normal.xy*2.0-1.0; // trafsform to [-1,1] range
	normal.z=sqrt(1.0-dot(normal.xy,normal.xy)); // compute z component

        // The normal is in tangent space, so transform back to world space
	// transform normal to world space
    // fx composer
    //    normal.xyz=normalize(normal.x*IN.tangent-normal.y*IN.binormal+normal.z*IN.normal);

    // or Max
	normal.xyz=normalize(normal.x*IN.binormal-normal.y*IN.tangent+normal.z*IN.normal);

	// color map
	float4 color=tex2D(texmap,uv);

	// compute diffuse and specular terms
	float att=saturate(dot(l,IN.normal.xyz));
	float diff=saturate(dot(l,normal.xyz));
	float spec=saturate(dot(normalize(l-v),normal.xyz));

	// compute final color
	float4 finalcolor;

	finalcolor.xyz=ambient*color.xyz+
		att*(color.xyz*diffuse.xyz*diff+specular.xyz*pow(spec,shine));

        // Test, just use the original color
        //finalcolor.xyz = color.xyz;

        //Test, use the computed world spacenormal
	//finalcolor.xyz = normal;

        // copy alpha from the original texture
	//finalcolor.w=color.w;
    
        // Test, color by offset, should get shades of red - yellow - green - black,     
        // (negative numbers are set to zero)
        //finalcolor.xy = ((offset ) / hole_depth) * 4;
        //finalcolor.z = 0;
        
        // Test, color by view normal
        // will be blue in the middle of the sceen (0,0,1)
        // tending towards 
        //finalcolor.xyz = v.xyz;
        //finalcolor.x = 0;
        

	finalcolor.w=orig_alpha;

	return finalcolor;
}

// Specific to the concave faceted geometry of our bullet hole 
float4 parallax_iterate_map(
	v2f IN,
	uniform sampler2D texmap,
	uniform sampler2D reliefmap) : COLOR
{
   	// view and light directions
	float3 v = normalize(IN.TexCoord1);                                     // v = unit vector towards the viewer
	float3 l = normalize(IN.lightpos.xyz-IN.TexCoord1);                     // l = unit vector from the light to the viewer

	float2 uv = IN.TexCoord0*tile;                                          // uv = regular texture coordinates

        // Get the alpha from the original pixel to prevent holes at extreme angles
        float orig_alpha = tex2D(texmap,uv).w;

        // Get depth from the alpha (w) of the relief map
        // and scale
	//float depth = (tex2D(reliefmap,uv).w) * hole_depth;    
	//float depth = 1.0f * hole_depth;    

	// For use in Max (Right Handed) 
        // Right handed is b,t,n
	//float3x3 to_tangent_space = float3x3(IN.binormal,IN.tangent,IN.normal);

        // For use most engines, and FX Composer
        // Left handed is t,b,n
	float3x3 to_tangent_space = float3x3(IN.tangent,IN.binormal,IN.normal);     

                                             
        // transform the vector from the pixel to the viewer into tangent space
        // then simply by taking xy, we project it onto the surface of the polygon
        // which gives us a direction in which to offset the uv
        // we then scale that by the depth at the original point                               

//        float2 uv2 = uv + offset;
//        float depth2 = (tex2D(reliefmap,uv2).w);
       
        // now we've assumed a depth of 1, 
        // if this gives us a point on the base then we want to use it
        // otherwise, we redo the depth calculation with the "correct" depth

        
        float2 offset_direction = hole_depth * mul(to_tangent_space,v);        
        float2 offset = offset_direction;   // implied * 1.0      
        float depth_at_1 = tex2D(reliefmap,uv+offset).w;
        if ( depth_at_1 < 0.96f)
        {

	   offset = offset_direction * (depth_at_1 + (tex2D(reliefmap,uv).w)) * 0.5;          
       
//           depth_at_1 = tex2D(reliefmap,uv+offset).w;
//	   offset = (offset + offset_direction * (tex2D(reliefmap,uv+offset_direction*depth_at_1).w)) * 0.5;          

        }

        // get a new uv
	uv += offset;

	// use the new UV to get a normal
        // note this means we read the reliefmap twice, once for the height
        // and then for the normal that matches the pixel in the texture map
	float4 normal=tex2D(reliefmap,uv);
	normal.xy=normal.xy*2.0-1.0; // trafsform to [-1,1] range
	normal.z=sqrt(1.0-dot(normal.xy,normal.xy)); // compute z component

        // The normal is in tangent space, so transform back to world space
	// transform normal to world space
	normal.xyz=normalize(normal.x*IN.tangent-normal.y*IN.binormal+normal.z*IN.normal);

	// color map
	float4 color=tex2D(texmap,uv);

	// compute diffuse and specular terms
	float att=saturate(dot(l,IN.normal.xyz));
	float diff=saturate(dot(l,normal.xyz));
	float spec=saturate(dot(normalize(l-v),normal.xyz));

	// compute final color
	float4 finalcolor;

	finalcolor.xyz=ambient*color.xyz+
		att*(color.xyz*diffuse.xyz*diff+specular.xyz*pow(spec,shine));

        // Test, just use the original color
        //finalcolor.xyz = color.xyz;

        //Test, use the computed world spacenormal
	//finalcolor.xyz = normal;

        // copy alpha from the original texture
	//finalcolor.w=color.w;
    
        // Test, color by offset, should get shades of red - yellow - green - black,     
        // (negative numbers are set to zero)
        //finalcolor.xy = ((offset ) / hole_depth) * 4;
        //finalcolor.z = 0;
        
        // Test, color by view normal
        // will be blue in the middle of the sceen (0,0,1)
        // tending towards 
        //finalcolor.xyz = v.xyz;
        //finalcolor.x = 0;
        

	finalcolor.w=orig_alpha;
//finalcolor.w=0.0;

	return finalcolor;
}


technique parallax_3dsmax_mapping
{
    pass p0 
    {
    	CullMode = none;
		VertexShader = compile vs_1_1 view_space();
		PixelShader  = compile ps_2_0 parallax_3dsmax_map(texmap_sampler,reliefmap_sampler);
    }
}



technique parallax_mapping
{
    pass p0 
    {
    	CullMode = none;
		VertexShader = compile vs_1_1 view_space();
		PixelShader  = compile ps_2_0 parallax_map(texmap_sampler,reliefmap_sampler);
    }
}

technique parallax_bullet_mapping
{
    pass p0 
    {
    	CullMode = none;
		VertexShader = compile vs_1_1 view_space();
		PixelShader  = compile ps_2_0 parallax_bullet_map(texmap_sampler,reliefmap_sampler);
    }
}


technique parallax_iterate_mapping
{
   pass p0 
    {
 		SrcBlend = SrcAlpha;
		DestBlend = SrcAlpha;
    	CullMode = none;
		VertexShader = compile vs_1_1 view_space();
		PixelShader  = compile ps_2_0 parallax_iterate_map(texmap_sampler,reliefmap_sampler);
    }
}

technique normal_mapping
{
    pass p0 
    {
    	CullMode = none;
		VertexShader = compile vs_1_1 view_space();
		PixelShader  = compile ps_2_0 normal_map(texmap_sampler,reliefmap_sampler);
    }
}


