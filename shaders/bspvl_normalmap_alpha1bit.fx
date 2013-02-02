
#include "common.fxh"

texture2D diffuseTex				: TextureLayer0;
texture2D normalTex					: TextureLayer3;

sampler2D diffTexture = sampler_state
{
   Texture = (diffuseTex);
// 	MinFilter = Linear;
// 	MagFilter = Linear;
 	MipFilter = None;
};
sampler2D normalTexture = sampler_state
{
   Texture = (normalTex);
};

#define SPEC_POWER	32.0f
#define DIFFLIGHT_CORRECTION	0.2f


struct VS_OUTPUT
{
   float4 Position	:	POSITION0;
   float3 wView		:	TEXCOORD0;
   float3 Normal	:	TEXCOORD1;
   float3 Deluxel	:	TEXCOORD2;
   float3 Tangent	:	TEXCOORD3;
   float3 Binormal	:	TEXCOORD4;   
   float2 uv0		:	TEXCOORD5;   
   float4 Color		:	COLOR;
};

///////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT_BSPVERTEX IN)
{
	VS_OUTPUT OUT;
	OUT.Position = mul (IN.Position, matViewProjection);
	OUT.wView = vec3CameraPosition - IN.Position.xyz;
	OUT.Normal = IN.Normal;
	OUT.Deluxel = IN.Deluxel;
	OUT.Tangent = IN.Tangent;
	OUT.Binormal = IN.Binormal;
	OUT.uv0 = IN.uv0;
	OUT.Color = IN.Color;
	return OUT;
}

///////////////////////////////////////
float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	float4 color = tex2D(diffTexture, IN.uv0);
	color.xyz *= IN.Color.xyz;
	float4 normal = tex2D(normalTexture, IN.uv0);
	normal.xyz = 2.0f * normal.xyz - 1.0f;
	// transform normal from tangent to world space
	normal.xyz = (normal.x * IN.Tangent) + (normal.y * IN.Binormal) + (normal.z * IN.Normal);
	normal.xyz = normalize (normal.xyz);
	// att
	color.xyz *= saturate (dot (IN.Deluxel, normal.xyz) + DIFFLIGHT_CORRECTION);	// normal.xyz, IN.Normal
	// specular reflection
	float3 V = normalize (IN.wView);
	float3 reflectVec = reflect (-IN.Deluxel, normal.xyz);
	float3 spec = IN.Color.xyz * pow(saturate(dot(reflectVec, V)), SPEC_POWER);// * saturate(dot(IN.Deluxel, IN.Normal));
	// specular map
	color.xyz += spec * normal.w;

	return color;
}


//--------------------------------------------------------------
// Technique Section
//--------------------------------------------------------------
technique bspvl_normalmap_alpha1bit
{
	pass Pass0
	{
		ALPHATESTENABLE = TRUE;
		ALPHAFUNC = GREATEREQUAL;
		ALPHAREF = ALPHAREF_VALUE;
		CULLMODE = NONE;
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}
}

technique bspvl_normalmap_alpha1bit_lighted
{
	pass Pass0
	{
		ALPHATESTENABLE = TRUE;
		ALPHAFUNC = GREATEREQUAL;
		ALPHAREF = ALPHAREF_VALUE;
		CULLMODE = NONE;
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_BSP_NORMALMAP_LIGHTING(diffTexture, normalTexture)
}

////////////////////////////////////////////////////////////////////////
technique bspvl_normalmap_alpha1bit_sm30
{
	pass Pass0
	{
		ALPHATESTENABLE = TRUE;
		ALPHAFUNC = GREATEREQUAL;
		ALPHAREF = ALPHAREF_VALUE;
		CULLMODE = NONE;
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}
}

ASSEMBLE_BSP_NORMALMAP_SHADERS(diffTexture, normalTexture)

technique bspvl_normalmap_alpha1bit_sm30_lighted
{
	pass Pass0
	{
		ALPHATESTENABLE = TRUE;
		ALPHAFUNC = GREATEREQUAL;
		ALPHAREF = ALPHAREF_VALUE;
		CULLMODE = NONE;
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_BSP_NORMALMAP_LIGHTING_ONEPASS
}
