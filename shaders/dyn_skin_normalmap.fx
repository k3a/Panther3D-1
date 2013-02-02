
/////////////////////////////////////////////////////////////
// skin normalmap shader
/////////////////////////////////////////////////////////////

#include "common.fxh"

/*
input constant float registers:
vs1.1 - min 96
vs2.0 - min 256
vs3.0 - min 256
*/

#define SPEC_POWER		32.0f


float3x3 lightGridInfo						: LightGridInfo;	// ambientRGB, directionalRGB, directionXYZ
int CurNumBones								: NumBones;
float4x3 mWorldMatrixArray[MAX_SKIN_BONES]	: SkinMatrixArray;

texture2D diffTex	: TextureLayer0;
texture2D normalTex	: TextureLayer1;

sampler2D diffTexture = sampler_state
{
	Texture = <diffTex>;
// 	MinFilter = Linear;
// 	MagFilter = Linear;
// 	MipFilter = Linear;
};

sampler2D normalTexture = sampler_state
{
	Texture = <normalTex>;
};

///////////////////////////////////////////////////////
struct VS_OUTPUT
{
	float4  Position	: POSITION;
	float3  Normal		: TEXCOORD0;
	float3  Tangent		: TEXCOORD1;
	float2  uv0			: TEXCOORD2;
	float2  uv1			: TEXCOORD3;
	float3  wView		: TEXCOORD4;
};

///////////////////////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT_DYNMODELS_SKINNED IN, uniform int NumBones)
{
	VS_OUTPUT   OUT;
	float		fWeight;
	int			Idx;
	float3      pos = 0.0f;
	float3      Normal = 0.0f;
	float3      Tangent = 0.0f;
	float       LastWeight = 1.0f;

	// cast the vectors to arrays for use in the for loop below
	float BlendWeightsArray[4] = (float[4])IN.BlendWeights;
	int   IndexArray[4]        = (int[4])IN.BlendIndices;

	for (int iBone = 0; iBone < NumBones; iBone++)
	{
		fWeight = BlendWeightsArray[iBone];
		LastWeight -= fWeight;
		Idx = IndexArray[iBone];

		pos += mul(IN.Position, mWorldMatrixArray[Idx]) * fWeight;
		Normal += mul(IN.Normal, (float3x3) mWorldMatrixArray[Idx]) * fWeight;
		Tangent += mul(IN.Tangent, (float3x3) mWorldMatrixArray[Idx]) * fWeight;
	}
	Idx = IndexArray[NumBones];

	// add in the final influence
	pos += mul(IN.Position, mWorldMatrixArray[Idx]) * LastWeight;
	Normal += mul(IN.Normal, (float3x3) mWorldMatrixArray[Idx]) * LastWeight;
	Tangent += mul(IN.Tangent, (float3x3) mWorldMatrixArray[Idx]) * LastWeight;

	OUT.Position = mul(float4(pos.xyz, 1.0f), matViewProjection);
	OUT.wView = vec3CameraPosition - pos;

	OUT.Normal = normalize (Normal);
	OUT.Tangent = normalize (Tangent);
	OUT.uv0  = IN.uv0;
	OUT.uv1  = IN.uv1;

	return OUT;
}

///////////////////////////////////////////////////////
float4 ps_main (VS_OUTPUT IN): COLOR
{
	float4 color = tex2D(diffTexture, IN.uv0);
	float4 normal = tex2D(normalTexture, IN.uv1);
	normal.xyz = 2.0 * normal.xyz - 1.0;
	// transform normal to world space
	float3 binormal = cross (IN.Normal, IN.Tangent);
	normal.xyz = normalize(normal.x*IN.Tangent + normal.y*binormal + normal.z*IN.Normal);

	// specular reflection
	float3 reflectVec = reflect (lightGridInfo[2], normal.xyz);
	float spec = pow(saturate(dot(reflectVec, normalize (IN.wView))), SPEC_POWER);	/* * saturate(dot(-lightGridInfo[2], IN.Normal));*/

	// compute final color
	color.xyz = (lightGridInfo[0] * color.xyz) + \
		lightGridInfo[1] * (color.xyz * saturate(dot(-lightGridInfo[2], normal.xyz)) + spec /** normal.w*/);

	return color;
}


///////////////////////////////////////////////////////
VertexShader vsArray[4] = { compile vs_2_0 vs_main(0), 
                            compile vs_2_0 vs_main(1),
                            compile vs_2_0 vs_main(2),
                            compile vs_2_0 vs_main(3)};

VertexShader vsArraySM3[4] = { compile vs_3_0 vs_main(0), 
                            compile vs_3_0 vs_main(1),
                            compile vs_3_0 vs_main(2),
                            compile vs_3_0 vs_main(3)};

//////////////////////////////////////
// Techniques
//////////////////////////////////////
technique dyn_skin_normalmap
{
	Pass Pass0
	{
		VertexShader = (vsArray[CurNumBones]);
		PixelShader = compile ps_2_0 ps_main();
	}
}

technique dyn_skin_normalmap_sm30
{
	Pass Pass0
	{
		VertexShader = (vsArraySM3[CurNumBones]);
		PixelShader = compile ps_2_0 ps_main();
	}
}
