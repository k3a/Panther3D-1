
shared float4x4 matViewProjection	: ViewProjection;

float4x4	matWorld	: World;
float		fTime		: Time;

texture2D	Clouds_Tex	: Texture;
textureCUBE	Cubemap_Tex	: TextureCubeMap;

#define		CLOUD_TEXTURE_TILES	2.0f
#define		CLOUD_SPEED			0.0015f

sampler2D	cloudTexture = sampler_state
{
   Texture = (Clouds_Tex);
 	AddressU = Wrap;
 	AddressV = Wrap;
 	MinFilter = Linear;
 	MagFilter = Linear;
 	MipFilter = Linear;   
};
samplerCUBE	skyTexture = sampler_state
{
	Texture = (Cubemap_Tex);
 	AddressU = Clamp;
 	AddressV = Clamp;
 	MinFilter = Linear;
 	MagFilter = Linear;
 	MipFilter = None;
};

struct VS_INPUT
{
   float4 Position : POSITION0;
};
struct VS_OUTPUT
{
   float4 Position	: POSITION0;
   float3 Tex3		: TEXCOORD0;
};

struct VS_INPUT_CLOUD
{
	float4 Position	: POSITION0;
	float2 uv1		: TEXCOORD0;
};
struct VS_OUTPUT_CLOUD
{
   float4 Position	: POSITION0;
   float2 uv1		: TEXCOORD0;
   float2 alphaCoef	: TEXCOORD1;
};

////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT IN)
{
   VS_OUTPUT OUT;
   OUT.Position = mul (IN.Position, mul (matWorld, matViewProjection));
   OUT.Tex3 = IN.Position;
   return OUT;
}

float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	return texCUBE (skyTexture, IN.Tex3);
}

////////////////////////////////
VS_OUTPUT_CLOUD vs_main_cloud (VS_INPUT_CLOUD IN)
{
	VS_OUTPUT_CLOUD OUT;
	OUT.Position = mul (IN.Position, mul (matWorld, matViewProjection));
	OUT.alphaCoef = IN.uv1;
	OUT.uv1 = (IN.uv1 * CLOUD_TEXTURE_TILES) + (fTime * CLOUD_SPEED);
	return OUT;
}

float4 ps_main_cloud (VS_OUTPUT_CLOUD IN) : COLOR0
{
	float4 OutColor = float4(1,1,1,1);
	OutColor.w = tex2D (cloudTexture, IN.uv1);
	float2 uv = 2.0f * abs(0.5f - IN.alphaCoef);
	OutColor.w -= dot(uv, uv);
	return OutColor;
}

//--------------------------------------------------------------
// Techniques
//--------------------------------------------------------------
technique spec_skybox
{
   pass Pass0
   {
      CULLMODE = NONE;
      ZWRITEENABLE = FALSE;
      VertexShader = compile vs_2_0 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique spec_clouds
{
   pass Pass0
   {
      ALPHABLENDENABLE = TRUE;
      SRCBLEND = SRCALPHA;
      DESTBLEND = INVSRCALPHA;
      ZWRITEENABLE = FALSE;

      VertexShader = compile vs_2_0 vs_main_cloud();
      PixelShader = compile ps_2_0 ps_main_cloud();
   }
}
