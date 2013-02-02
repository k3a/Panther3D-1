
/////////////////////////////////////////////////////////////
// standardny shader (diffuse + ambient) pre dynamicke modely
/////////////////////////////////////////////////////////////

#include "common.fxh"

float4x4 matWorldViewProj : WorldViewProjection;
float4x4 matWorld : World;

float3x3 lightGridInfo	: LightGridInfo;	// ambientRGB, directionalRGB, directionXYZ

texture2D diffTex		: TextureLayer0;

sampler2D diffTexture = sampler_state
{
   Texture = (diffTex);
//    MAGFILTER = LINEAR;
//    MINFILTER = LINEAR;
//    MIPFILTER = LINEAR;
};

struct VS_OUTPUT
{
   float4 Position : POSITION;
   float3 Normal : TEXCOORD0;
   float2 uv0 : TEXCOORD1;
};

//////////////////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT_DYNMODELS IN)
{
	VS_OUTPUT OUT;
	OUT.Position = mul (IN.Position, matWorldViewProj);
	OUT.Normal = mul (IN.Normal, (float3x3)matWorld);
	OUT.uv0 = IN.uv0;
	return OUT;
}

//////////////////////////////////////////////////
float4 ps_main (VS_OUTPUT IN) : COLOR
{
   float4 color = tex2D (diffTexture, IN.uv0);
   color.xyz *= lightGridInfo[0] + lightGridInfo[1] * saturate(dot(normalize(IN.Normal), -lightGridInfo[2]));
   return color;
}

//------------------------------------
// Technique Section
//------------------------------------
technique dyn_def
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique dyn_def_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_DYNMODEL_DEF_LIGHTING(matWorld, matWorldViewProj, diffTexture)
}


////////////////////////////////////////////////////////////////////////
technique dyn_def_sm30
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}
}

ASSEMBLE_DYNMODEL_DEF_SHADERS(diffTexture)

technique dyn_def_sm30_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}

	DO_DYNMODEL_DEF_LIGHTING_ONEPASS
}
