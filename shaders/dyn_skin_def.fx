
/////////////////////////////////////////////////////////////
// default skin shader
/////////////////////////////////////////////////////////////

#include "common.fxh"

/*
input constant float registers:
vs1.1 - min 96
vs2.0 - min 256
vs3.0 - min 256
*/

float3x3 lightGridInfo						: LightGridInfo;	// ambientRGB, directionalRGB, directionXYZ
int CurNumBones								: NumBones;
float4x3 mWorldMatrixArray[MAX_SKIN_BONES]	: SkinMatrixArray;

texture2D diffTex	: TextureLayer0;

sampler2D diffTexture = sampler_state
{
	Texture = <diffTex>;
// 	MinFilter = Linear;
// 	MagFilter = Linear;
// 	MipFilter = Linear;
};

///////////////////////////////////////////////////////
struct VS_OUTPUT
{
	float4  Position	: POSITION;
	float3  Normal		: TEXCOORD0;
	float2  uv0			: TEXCOORD1;
};

///////////////////////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT_DYNMODELS_SKINNED IN, uniform int NumBones)
{
	VS_OUTPUT   OUT;
	float		fWeight;
	int			Idx;
	float3      pos = 0.0f;
	float3      Normal = 0.0f;
	float       LastWeight = 1.0f;

	// cast the vectors to arrays for use in the for loop below
	float BlendWeightsArray[4] = (float[4])IN.BlendWeights;
	int   IndexArray[4]        = (int[4])IN.BlendIndices;
//	int   IndexArray[4]        = (int[4])D3DCOLORtoUBYTE4(IN.BlendIndices);

	for (int iBone = 0; iBone < NumBones; iBone++)
	{
		fWeight = BlendWeightsArray[iBone];
		LastWeight -= fWeight;
		Idx = IndexArray[iBone];

		pos += mul(IN.Position, mWorldMatrixArray[Idx]) * fWeight;
		Normal += mul(IN.Normal, (float3x3) mWorldMatrixArray[Idx]) * fWeight;
	}

	Idx = IndexArray[NumBones];

	// add in the final influence
	pos += mul(IN.Position, mWorldMatrixArray[Idx]) * LastWeight;
	Normal += mul(IN.Normal, (float3x3) mWorldMatrixArray[Idx]) * LastWeight;


	OUT.Position = mul(float4(pos.xyz, 1.0f), matViewProjection);
	OUT.Normal = normalize (Normal);
	OUT.uv0  = IN.uv0;

	return OUT;
}

///////////////////////////////////////////////////////
float4 ps_main (VS_OUTPUT IN): COLOR
{
	float4 color = tex2D(diffTexture, IN.uv0);
	color.xyz = color.xyz * (lightGridInfo[0] + \
			lightGridInfo[1] * saturate(dot(-lightGridInfo[2], IN.Normal)));
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
technique dyn_skin_def
{
	Pass Pass0
	{
		VertexShader = (vsArray[CurNumBones]);
		PixelShader = compile ps_2_0 ps_main();
	}
}

technique dyn_skin_def_sm30
{
	Pass Pass0
	{
		VertexShader = (vsArraySM3[CurNumBones]);
		PixelShader = compile ps_2_0 ps_main();
	}
}
