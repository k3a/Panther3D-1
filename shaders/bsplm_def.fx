
#include "common.fxh"

texture2D diffuseTexture			: TextureLayer0;
texture2D lightmapTexture			: TextureLayer1;

sampler2D diffTexture = sampler_state
{
   Texture = (diffuseTexture);
// 	MinFilter = Linear;
// 	MagFilter = Linear;
// 	MipFilter = Linear;
};
sampler2D LMTexture = sampler_state
{
   Texture = (lightmapTexture);
};


struct VS_OUTPUT
{
   float4	Position	:	POSITION0;
   float2	uv0			:	TEXCOORD0;
   float2	uv1			:	TEXCOORD1;
};


//--------------------------------------------------------------
// Pass 0 - diffuse and lightmap
//--------------------------------------------------------------

VS_OUTPUT vs_main (VS_INPUT_BSPVERTEX IN)
{
   VS_OUTPUT OUT;
   OUT.Position = mul (IN.Position, matViewProjection);
   OUT.uv0 = IN.uv0;
   OUT.uv1 = IN.uv1;
   return OUT;
}

float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	return tex2D (diffTexture, IN.uv0) * tex2D (LMTexture, IN.uv1);
}

//--------------------------------------------------------------
// Technique Section
// nepridavat uz ziadne techniques!
//--------------------------------------------------------------
technique bsplm_def
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_1_1 ps_main();
   }
}

technique bsplm_def_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_1_1 ps_main();
	}

	DO_BSP_DEF_LIGHTING(diffTexture)
}

////////////////////////////////////////////////////////////////////////
technique bsplm_def_sm30
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_1_1 ps_main();
   }
}

ASSEMBLE_BSP_DEF_SHADERS(diffTexture)

technique bsplm_def_sm30_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_1_1 ps_main();
	}

	DO_BSP_DEF_LIGHTING_ONEPASS
}
