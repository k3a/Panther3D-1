
#include "common.fxh"

texture2D diffTex		: TextureLayer0;

sampler2D diffTexture = sampler_state
{
   Texture = (diffTex);
// 	MinFilter = Linear;
// 	MagFilter = Linear;
// 	MipFilter = Linear;
};

struct VS_OUTPUT
{
   float4	Position	:	POSITION0;
   float2	uv0			:	TEXCOORD0;
   float4	Color		:	COLOR;
};

//--------------------------------------------------------------
// Pass 0 - diffuse and vertex color
//--------------------------------------------------------------

VS_OUTPUT vs_main (VS_INPUT_BSPVERTEX IN)
{
   VS_OUTPUT OUT;
   OUT.Position = mul (IN.Position, matViewProjection);
   OUT.uv0 = IN.uv0;
   OUT.Color = IN.Color;
   return OUT;
}

float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	return tex2D (diffTexture, IN.uv0) * IN.Color;	
}

//--------------------------------------------------------------
// Technique Section
// nepridavat uz ziadne techniques!
//--------------------------------------------------------------
technique bspvl_def
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_1_1 ps_main();
   }
}

technique bspvl_def_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_1_1 ps_main();
	}

	DO_BSP_DEF_LIGHTING(diffTexture)
}

////////////////////////////////////////////////////////////////////////
technique bspvl_def_sm30
{
   pass Pass0
   {
      VertexShader = compile vs_1_1 vs_main();
      PixelShader = compile ps_1_1 ps_main();
   }
}

ASSEMBLE_BSP_DEF_SHADERS(diffTexture)

technique bspvl_def_sm30_lighted
{
	pass Pass0
	{
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_1_1 ps_main();
	}

	DO_BSP_DEF_LIGHTING_ONEPASS
}
