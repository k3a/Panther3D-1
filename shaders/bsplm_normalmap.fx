
#include "common.fxh"

texture2D diffTex		: TextureLayer0;
texture2D lightTex		: TextureLayer1;
texture2D deluxelTex	: TextureLayer2;
texture2D normalTex		: TextureLayer3;


#define SPEC_POWER	32.0f
#define DIFFLIGHT_CORRECTION	0.3f


sampler2D diffTexture = sampler_state
{
	Texture = (diffTex);
// 	MinFilter = Linear;
// 	MagFilter = Linear;
// 	MipFilter = Linear;
};
sampler2D lightTexture = sampler_state
{
	Texture = (lightTex);
};
sampler2D deluxelTexture = sampler_state
{
	Texture = (deluxelTex);
};
sampler2D normalTexture = sampler_state
{
	Texture = (normalTex);
};


struct VERTEX_OUTPUT
{
	float4 Position	: POSITION;
	float3 wView	: TEXCOORD0;
	float2 uv0		: TEXCOORD1;
	float2 uv1		: TEXCOORD2;
	float3 Normal	: TEXCOORD3;
	float3 Tangent	: TEXCOORD4;
	float3 Binormal	: TEXCOORD5;
};

////////////////////////////////////////
VERTEX_OUTPUT vs_main (VS_INPUT_BSPVERTEX IN)
{
	VERTEX_OUTPUT OUT;
	// vertex position in clip space
	OUT.Position = mul(IN.Position, matViewProjection);	
	OUT.wView = vec3CameraPosition - IN.Position.xyz;
	OUT.uv0 = IN.uv0;
	OUT.uv1 = IN.uv1;
	OUT.Normal = IN.Normal;
	OUT.Tangent = IN.Tangent;
	OUT.Binormal = IN.Binormal;
	return OUT;
}

///////////////////////////////////////
float4 ps_main (VERTEX_OUTPUT IN) : COLOR0
{
	float4 color = tex2D(diffTexture, IN.uv0);
	float3 light = tex2D(lightTexture, IN.uv1);
	float3 deluxel = tex2D(deluxelTexture, IN.uv1);
	deluxel = 2.0f * deluxel - 1.0f;
	float4 normal = tex2D(normalTexture, IN.uv0);
	// unpack and transform normal from tangent to world space
	normal.xyz = 2.0f * normal.xyz - 1.0f;
	normal.xyz = (normal.x * IN.Tangent) + (normal.y * IN.Binormal) + (normal.z * IN.Normal);
	normal.xyz = normalize (normal.xyz);
	// att
	color.xyz *= light * saturate (dot (deluxel, normal.xyz) + DIFFLIGHT_CORRECTION);
	// specular reflection
	float3 V = normalize (IN.wView);
	float3 reflectVec = reflect (-deluxel, normal.xyz);
	float3 spec = light * pow(saturate(dot(reflectVec, V)), SPEC_POWER);
	// specular map
	color.xyz += spec * normal.w;

	return color;
}

//--------------------------------------------------------------
// Technique Section
//--------------------------------------------------------------
technique bsplm_normalmap
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique bsplm_normalmap_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_BSP_NORMALMAP_LIGHTING(diffTexture, normalTexture)
}

////////////////////////////////////////////////////////////////////////
technique bsplm_normalmap_sm30
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

ASSEMBLE_BSP_NORMALMAP_SHADERS(diffTexture, normalTexture)

technique bsplm_normalmap_sm30_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_BSP_NORMALMAP_LIGHTING_ONEPASS
}
