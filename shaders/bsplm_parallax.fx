
#include "common.fxh"

texture2D diffTex		: TextureLayer0;
texture2D lightTex		: TextureLayer1;
texture2D deluxelTex	: TextureLayer2;
texture2D normalTex		: TextureLayer3;

float spec_factor		: CustomValue1 = 1.0f;

#define	SPEC_POWER				32.0f
#define DIFFLIGHT_CORRECTION	0.2f


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

////////////////////////////////////////
float4 ps_main (VERTEX_OUTPUT IN) : COLOR
{
	// parallax code
	float3 V = normalize(IN.wView);
	float3x3 matTBN = float3x3(IN.Tangent, IN.Binormal, IN.Normal);
	float height = (1.0f - tex2D(normalTexture, IN.uv0).w) * (2.0f*PARALLAX_DEPTHFACTOR) - PARALLAX_DEPTHFACTOR;
	IN.uv0 += height * mul(matTBN, -V);

	float3 light = tex2D(lightTexture, IN.uv1);
	float3 deluxel = tex2D(deluxelTexture, IN.uv1);
	deluxel = 2.0f * deluxel - 1.0f;
	float4 normal = tex2D(normalTexture, IN.uv0);
	normal.xyz = 2.0f * normal.xyz - 1.0f;
	// transform normal from Tangent to world space
	normal.xyz = (normal.x * IN.Tangent) + (normal.y * IN.Binormal) + (normal.z * IN.Normal);
	normal.xyz = normalize (normal.xyz);
	// diffuse attenuation
	float3 color = tex2D(diffTexture, IN.uv0) * \
		light * saturate (dot (deluxel, normal.xyz) + DIFFLIGHT_CORRECTION);
	// specular reflection
	float3 reflectVec = reflect (-deluxel, normal.xyz);
	float3 spec = light * pow(saturate(dot(reflectVec, V)), SPEC_POWER);
	// specular map
	color += spec * spec_factor;

	return float4(color, 1.0f);
}

//--------------------------------------------------------------
// Technique Section
//--------------------------------------------------------------
technique bsplm_parallax
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique bsplm_parallax_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_BSP_PARALLAX_LIGHTING(diffTexture, normalTexture, spec_factor)
}

////////////////////////////////////////////////////////////////////////
technique bsplm_parallax_sm30
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

ASSEMBLE_BSP_PARALLAX_SHADERS(diffTexture, normalTexture, spec_factor)

technique bsplm_parallax_sm30_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_BSP_PARALLAX_LIGHTING_ONEPASS
}
