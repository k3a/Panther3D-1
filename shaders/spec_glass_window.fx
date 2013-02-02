
shared float4x4 matViewProjection	: ViewProjection;
shared float3 vec3CameraPosition	: CameraPosition;

static const float4 milk_glass_color = {0.97f, 0.97f, 1.0f, 0.2f};
#define CUBE_INTENSITY	0.4f

bool		features[2]		:	AnyValue;	// [0] - use diffuse_map, [1] - use cube_map
texture2D	diffuseTexture	:	TextureLayer0;
textureCUBE	cubeTexture		:	TextureCubeMap;

sampler2D diffuseSampler = sampler_state 
{
	texture = <diffuseTexture>;
};

samplerCUBE cubeSampler = sampler_state 
{
	texture = <cubeTexture>;
};

//////////////////////////////////////
struct VS_INPUT 
{
	float4 Position	:	POSITION0;
	float3 Normal	:	NORMAL;
	float2 uv		:	TEXCOORD0;
};

struct VS_OUTPUT 
{
	float4 Position		:	POSITION0;
	float3 Normal		:	TEXCOORD0;
	float2 uv			:	TEXCOORD1;
	float3 WorldView	:	TEXCOORD2;
};

//////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT IN)
{
   VS_OUTPUT OUT;
   OUT.Position = mul (IN.Position, matViewProjection);
   OUT.Normal = IN.Normal;
   OUT.uv = IN.uv;
   OUT.WorldView = IN.Position.xyz - vec3CameraPosition;
   return OUT;
}


//////////////////////////////////////
float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	float4 outColor;
	if (features[0] == true)
		outColor = tex2D (diffuseSampler, IN.uv);		// diffuse map with alpha
	else
		outColor = milk_glass_color;						// pure color

	if (features[1] == true)								// cube map
	{
		float3 reflVec = reflect (IN.WorldView, IN.Normal);
		outColor.xyz = lerp (outColor.xyz, texCUBE (cubeSampler, reflVec).xyz, CUBE_INTENSITY);
	}
   return outColor;
}

//////////////////////////////////////
technique glass_window
{
	pass Pass0
	{		
		VertexShader = compile vs_1_1 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}
