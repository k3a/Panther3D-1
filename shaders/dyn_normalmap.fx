
#include "common.fxh"

float4x4 matWorld : World;

float3x3 lightGridInfo	: LightGridInfo;	// ambientRGB, directionalRGB, directionXYZ

texture2D diffuseTex	: TextureLayer0;
texture2D normalTex		: TextureLayer1;

sampler2D diffTexture = sampler_state
{
	Texture = (diffuseTex);
// 	MinFilter = Linear;
// 	MagFilter = Linear;
// 	MipFilter = Linear;
};
sampler2D normalTexture = sampler_state
{
	Texture = (normalTex);
};

#define SPEC_POWER	32.0f


struct VS_OUTPUT
{
	float4 Position	:	POSITION;
	float3 wView	:	TEXCOORD0;
	float3 Normal	:	TEXCOORD1;
	float3 Tangent	:	TEXCOORD2;
	float3 Binormal	:	TEXCOORD3;
	float2 uv0		:	TEXCOORD4;
	float2 uv1		:	TEXCOORD5;
};

///////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT_DYNMODELS IN)
{
	VS_OUTPUT OUT;
	float4 worldPos = mul (IN.Position, matWorld);
	OUT.wView = vec3CameraPosition - worldPos.xyz;
	OUT.Position = mul (worldPos, matViewProjection);
	float3x3 world = (float3x3)matWorld;
	OUT.Normal = mul(IN.Normal, world);
	OUT.Tangent = mul(IN.Tangent, world);
	OUT.Binormal = mul(IN.Binormal, world);
	OUT.uv0 = IN.uv0;
	OUT.uv1 = IN.uv1;
	return OUT;
}

///////////////////////////////////////
float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	float4 color = tex2D(diffTexture, IN.uv0);
	float4 normal = tex2D(normalTexture, IN.uv1);
	normal.xyz = (2.0f * normal.xyz) - 1.0f;

	// transform normal to world space
	normal.xyz = normalize(normal.x*IN.Tangent + normal.y*IN.Binormal + normal.z*IN.Normal);
	// specular reflection
	float3 reflectVec = reflect (lightGridInfo[2], normal.xyz);
	float spec = pow(saturate(dot(reflectVec, normalize (IN.wView))), SPEC_POWER);	/* * saturate(dot(-lightGridInfo[2], IN.Normal));*/

	color.xyz = (lightGridInfo[0] * color.xyz) + \
		lightGridInfo[1] * (color.xyz * saturate (dot (-lightGridInfo[2], normal.xyz)) + spec * normal.w);

	return color;
}


//--------------------------------------------------------------
// Technique Section
//--------------------------------------------------------------
technique dyn_normalmap
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique dyn_normalmap_lighted
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }

   DO_DYNMODEL_NORMALMAP_LIGHTING (matWorld, diffTexture, normalTexture)
}

////////////////////////////////////////////////////////////////////////
technique dyn_normalmap_sm30
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

ASSEMBLE_DYNMODEL_NORMALMAP_SHADERS(diffTexture, normalTexture)

technique dyn_normalmap_sm30_lighted
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }

   DO_DYNMODEL_NORMALMAP_LIGHTING_ONEPASS(matWorld)
}
