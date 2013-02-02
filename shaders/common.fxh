

/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Global shared variables
////////////////////////////////////////////////////////////////////////
***********************************************************************/

// spolu s tymito parametrami upravit globals.fx, v engine tabulky g_SemanticsInfoTable, FXSemantics a
// funkcie CP3DMaterialManager::On*()

shared float4x4 matView : View;
shared float4x4 matViewTranspose : ViewTranspose;
shared float4x4 matViewInverse : ViewInverse;
shared float4x4 matViewInverseTranspose : ViewInverseTranspose;
shared float4x4 matViewProjection : ViewProjection;
shared float4x4 matViewProjectionTranspose : ViewProjectionTranspose;
shared float4x4 matViewProjectionInverse : ViewProjectionInverse;
shared float4x4 matViewProjectionInverseTranspose : ViewProjectionInverseTranspose;
shared float4x4 matProjection : Projection;
shared float4x4 matProjectionInverse : ProjectionInverse;
shared float4x4 matProjectionInverseTranspose : ProjectionInverseTranspose;

shared float3 vec3CameraPosition		: CameraPosition;
shared float3 vec3CameraDirection	: CameraDirection;
shared float2 vec2ViewportPixelSize	: ViewportPixelSize;


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////
***********************************************************************/


#define MAX_DYN_LIGHTS_PER_FACE	4

#define ALPHAREF_VALUE	0xA0

#define SPEC_POWER_DYNLIGHT				32.0f
#define DIFFLIGHT_CORRECTION_DYNLIGHT	0.2f
#define	PARALLAX_DEPTHFACTOR			0.02f

// max pocet kosti pre jeden skin model
#define	MAX_SKIN_BONES	82		// rovnaka hodnota musi byt v engine!

#define FORCETO_2_0		// force minimum build target to 2_0

#ifdef FORCETO_2_0
	#define	vs_1_1	vs_2_0
	#define	ps_1_1	ps_2_0
	#define	ps_1_2	ps_2_0
	#define	ps_1_3	ps_2_0
	#define	ps_1_4	ps_2_0
#endif


//#define FORCE_TO_USE_FLOAT	// forcing half to float

#ifdef FORCE_TO_USE_FLOAT
	#define	half	float
	#define	half2	float2
	#define	half3	float3
	#define	half4	float4
	#define	half1x2	float1x2
	#define	half2x2	float2x2
	#define	half3x2	float3x2
	#define	half4x2	float4x2
	#define	half1x3	float1x3
	#define	half2x3	float2x3
	#define	half3x3	float3x3
	#define	half4x3	float4x3
	#define	half1x4	float1x4
	#define	half2x4	float2x4
	#define	half3x4	float3x4
	#define	half4x4	float4x4
#endif


// dynamic lighting passes definitions
//---------------------------------------------------------------------

#define ACTIVATE_DYNLIGHT_RENDERSTATES	ALPHABLENDENABLE = TRUE; DESTBLEND = ONE; SRCBLEND = ONE; ZWRITEENABLE = FALSE;

// Standard dynamic lighting for static geometry (bsp)
#define DO_BSP_DEF_LIGHTING(diffTexture)	\
	pass LightPass1 {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_1_1 vs_def_bsp_dynlight(0); \
	PixelShader = compile ps_2_0 ps_def_bsp_dynlight(0, diffTexture, samp_light_texture0);} \
	pass LightPass2 { \
	VertexShader = compile vs_1_1 vs_def_bsp_dynlight(1); \
	PixelShader = compile ps_2_0 ps_def_bsp_dynlight(1, diffTexture, samp_light_texture1);} \
	pass LightPass3 { \
	VertexShader = compile vs_1_1 vs_def_bsp_dynlight(2); \
	PixelShader = compile ps_2_0 ps_def_bsp_dynlight(2, diffTexture, samp_light_texture2);} \
	pass LightPass4 { \
	VertexShader = compile vs_1_1 vs_def_bsp_dynlight(3); \
	PixelShader = compile ps_2_0 ps_def_bsp_dynlight(3, diffTexture, samp_light_texture3);}

// Standard dynamic lighting for static geometry (bsp)
// in one pass with shader model 3.0
#define ASSEMBLE_BSP_DEF_SHADERS(diffTexture)	\
	PixelShader psDefBspDynlightArray[4] = { \
	compile ps_3_0 ps_def_bsp_dynlight_onepass(1, diffTexture), \
	compile ps_3_0 ps_def_bsp_dynlight_onepass(2, diffTexture), \
	compile ps_3_0 ps_def_bsp_dynlight_onepass(3, diffTexture), \
	compile ps_3_0 ps_def_bsp_dynlight_onepass(4, diffTexture)};

#define DO_BSP_DEF_LIGHTING_ONEPASS	\
	pass LightPass1 {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_3_0 vs_def_bsp_dynlight_onepass(); \
	PixelShader = (psDefBspDynlightArray[light_count-1]);}



// Normal map dynamic lighting for static geometry (bsp)
#define DO_BSP_NORMALMAP_LIGHTING(diffTexture, normalTexture)	\
	pass BSPNormalmapDynLightPass0 {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_1_1 vs_normalmap_bsp_dynlight(0); \
	PixelShader = compile ps_2_0 ps_normalmap_bsp_dynlight(0, diffTexture, normalTexture, samp_light_texture0);} \
	pass BSPNormalmapDynLightPass1 { \
	VertexShader = compile vs_1_1 vs_normalmap_bsp_dynlight(1); \
	PixelShader = compile ps_2_0 ps_normalmap_bsp_dynlight(1, diffTexture, normalTexture, samp_light_texture1);} \
	pass BSPNormalmapDynLightPass2 { \
	VertexShader = compile vs_1_1 vs_normalmap_bsp_dynlight(2); \
	PixelShader = compile ps_2_0 ps_normalmap_bsp_dynlight(2, diffTexture, normalTexture, samp_light_texture2);} \
	pass BSPNormalmapDynLightPass3 { \
	VertexShader = compile vs_1_1 vs_normalmap_bsp_dynlight(3); \
	PixelShader = compile ps_2_0 ps_normalmap_bsp_dynlight(3, diffTexture, normalTexture, samp_light_texture3);}

// Normal map dynamic lighting for static geometry (bsp)
// in one pass with shader model 3.0
#define ASSEMBLE_BSP_NORMALMAP_SHADERS(diffTexture, normalTexture)	\
	PixelShader psNormalmapBspDynlightArray[4] = { \
	compile ps_3_0 ps_normalmap_bsp_dynlight_onepass(1, diffTexture, normalTexture), \
	compile ps_3_0 ps_normalmap_bsp_dynlight_onepass(2, diffTexture, normalTexture), \
	compile ps_3_0 ps_normalmap_bsp_dynlight_onepass(3, diffTexture, normalTexture), \
	compile ps_3_0 ps_normalmap_bsp_dynlight_onepass(4, diffTexture, normalTexture)};

#define DO_BSP_NORMALMAP_LIGHTING_ONEPASS	\
	pass BSPNormalmapDynLightOnePass {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_3_0 vs_normalmap_bsp_dynlight_onepass(); \
	PixelShader = (psNormalmapBspDynlightArray[light_count-1]);}



// Parallax dynamic lighting for static geometry (bsp)
#define DO_BSP_PARALLAX_LIGHTING(diffTexture, normalTexture, spec_factor)	\
	pass BSPParallaxDynLightPass0 {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_1_1 vs_parallax_bsp_dynlight(0); \
	PixelShader = compile ps_2_0 ps_parallax_bsp_dynlight(0, spec_factor, diffTexture, normalTexture, samp_light_texture0);} \
	pass BSPParallaxDynLightPass1 { \
	VertexShader = compile vs_1_1 vs_parallax_bsp_dynlight(1); \
	PixelShader = compile ps_2_0 ps_parallax_bsp_dynlight(1, spec_factor, diffTexture, normalTexture, samp_light_texture1);} \
	pass BSPParallaxDynLightPass2 { \
	VertexShader = compile vs_1_1 vs_parallax_bsp_dynlight(2); \
	PixelShader = compile ps_2_0 ps_parallax_bsp_dynlight(2, spec_factor, diffTexture, normalTexture, samp_light_texture2);} \
	pass BSPParallaxDynLightPass3 { \
	VertexShader = compile vs_1_1 vs_parallax_bsp_dynlight(3); \
	PixelShader = compile ps_2_0 ps_parallax_bsp_dynlight(3, spec_factor, diffTexture, normalTexture, samp_light_texture3);}

// Parallax dynamic lighting for static geometry (bsp)
// in one pass with shader model 3.0
#define ASSEMBLE_BSP_PARALLAX_SHADERS(diffTexture, normalTexture, spec_factor)	\
	PixelShader psParallaxBspDynlightArray[4] = { \
	compile ps_3_0 ps_parallax_bsp_dynlight_onepass(1, spec_factor, diffTexture, normalTexture), \
	compile ps_3_0 ps_parallax_bsp_dynlight_onepass(2, spec_factor, diffTexture, normalTexture), \
	compile ps_3_0 ps_parallax_bsp_dynlight_onepass(3, spec_factor, diffTexture, normalTexture), \
	compile ps_3_0 ps_parallax_bsp_dynlight_onepass(4, spec_factor, diffTexture, normalTexture)};

#define DO_BSP_PARALLAX_LIGHTING_ONEPASS	\
	pass BSPParallaxDynLightOnePass {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_3_0 vs_parallax_bsp_dynlight_onepass(); \
	PixelShader = (psParallaxBspDynlightArray[light_count-1]);}



// Standard dynamic lighting for dynamic models
#define DO_DYNMODEL_DEF_LIGHTING(matWorld, matWorldViewProj, diffTexture)	\
	pass DynModelDefDynLightPass0 {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_1_1 vs_def_dynmodel_dynlight(0, matWorld, matWorldViewProj); \
	PixelShader = compile ps_2_0 ps_def_dynmodel_dynlight(0, diffTexture, samp_light_texture0);} \
	pass DynModelDefDynLightPass1 { \
	VertexShader = compile vs_1_1 vs_def_dynmodel_dynlight(1, matWorld, matWorldViewProj); \
	PixelShader = compile ps_2_0 ps_def_dynmodel_dynlight(1, diffTexture, samp_light_texture1);} \
	pass DynModelDefDynLightPass2 { \
	VertexShader = compile vs_1_1 vs_def_dynmodel_dynlight(2, matWorld, matWorldViewProj); \
	PixelShader = compile ps_2_0 ps_def_dynmodel_dynlight(2, diffTexture, samp_light_texture2);} \
	pass DynModelDefDynLightPass3 { \
	VertexShader = compile vs_1_1 vs_def_dynmodel_dynlight(3, matWorld, matWorldViewProj); \
	PixelShader = compile ps_2_0 ps_def_dynmodel_dynlight(3, diffTexture, samp_light_texture3);}

// Standard dynamic lighting for dynamic models
// in one pass with shader model 3.0
#define ASSEMBLE_DYNMODEL_DEF_SHADERS(diffTexture)	\
	PixelShader psDefDynmodelDynlightArray[4] = { \
	compile ps_3_0 ps_def_dynmodel_dynlight_onepass(1, diffTexture), \
	compile ps_3_0 ps_def_dynmodel_dynlight_onepass(2, diffTexture), \
	compile ps_3_0 ps_def_dynmodel_dynlight_onepass(3, diffTexture), \
	compile ps_3_0 ps_def_dynmodel_dynlight_onepass(4, diffTexture)};

#define DO_DYNMODEL_DEF_LIGHTING_ONEPASS	\
	pass DynModelDefDynLightOnePass {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_3_0 vs_def_dynmodel_dynlight_onepass(matWorld, matWorldViewProj); \
	PixelShader = (psDefDynmodelDynlightArray[light_count-1]);}



// Normal map dynamic lighting for dynamic models
#define DO_DYNMODEL_NORMALMAP_LIGHTING(matWorld, diffTexture, normalTexture)	\
	pass DynModelNormalmapDynLightPass0 {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_1_1 vs_normalmap_dynmodel_dynlight(0, matWorld); \
	PixelShader = compile ps_2_0 ps_normalmap_dynmodel_dynlight(0, diffTexture, normalTexture, samp_light_texture0);} \
	pass DynModelNormalmapDynLightPass1 { \
	VertexShader = compile vs_1_1 vs_normalmap_dynmodel_dynlight(1, matWorld); \
	PixelShader = compile ps_2_0 ps_normalmap_dynmodel_dynlight(1, diffTexture, normalTexture, samp_light_texture1);} \
	pass DynModelNormalmapDynLightPass2 { \
	VertexShader = compile vs_1_1 vs_normalmap_dynmodel_dynlight(2, matWorld); \
	PixelShader = compile ps_2_0 ps_normalmap_dynmodel_dynlight(2, diffTexture, normalTexture, samp_light_texture2);} \
	pass DynModelNormalmapDynLightPass3 { \
	VertexShader = compile vs_1_1 vs_normalmap_dynmodel_dynlight(3, matWorld); \
	PixelShader = compile ps_2_0 ps_normalmap_dynmodel_dynlight(3, diffTexture, normalTexture, samp_light_texture3);}

// Normal map dynamic lighting for dynamic models
// in one pass with shader model 3.0
#define ASSEMBLE_DYNMODEL_NORMALMAP_SHADERS(diffTexture, normalTexture)	\
	PixelShader psNormalmapDynmodelDynlightArray[4] = { \
	compile ps_3_0 ps_normalmap_dynmodel_dynlight_onepass(1, diffTexture, normalTexture), \
	compile ps_3_0 ps_normalmap_dynmodel_dynlight_onepass(2, diffTexture, normalTexture), \
	compile ps_3_0 ps_normalmap_dynmodel_dynlight_onepass(3, diffTexture, normalTexture), \
	compile ps_3_0 ps_normalmap_dynmodel_dynlight_onepass(4, diffTexture, normalTexture)};

#define DO_DYNMODEL_NORMALMAP_LIGHTING_ONEPASS(matWorld)	\
	pass DynModelNormalmapDynLightOnePass {ACTIVATE_DYNLIGHT_RENDERSTATES \
	VertexShader = compile vs_3_0 vs_normalmap_dynmodel_dynlight_onepass(matWorld); \
	PixelShader = (psNormalmapDynmodelDynlightArray[light_count-1]);}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////
***********************************************************************/

struct VS_INPUT_BSPVERTEX
{
	float4	Position	:	POSITION;
	float3	Normal		:	NORMAL0;
	float3	Deluxel		:	NORMAL1;
	float2	uv0			:	TEXCOORD0;
	float2	uv1			:	TEXCOORD1;
	float3	Tangent		:	TANGENT;
	float3	Binormal	:	BINORMAL;
	float4	Color		:	COLOR;
};

struct VS_INPUT_DYNMODELS
{
	float4	Position	:	POSITION;
	float3	Normal		:	NORMAL;
	float3	Tangent		:	TANGENT;
	float3	Binormal	:	BINORMAL;
	float2	uv0			:	TEXCOORD0;
	float2	uv1			:	TEXCOORD1;
	float2	uv2			:	TEXCOORD2;
	float2	uv3			:	TEXCOORD3;
 	float2	uv4			:	TEXCOORD4;
 	float2	uv5			:	TEXCOORD5;
 	float2	uv6			:	TEXCOORD6;
 	float2	uv7			:	TEXCOORD7;
};

struct VS_INPUT_DYNMODELS_SKINNED
{
	float4	Position		:	POSITION;
	float3	Normal			:	NORMAL;
	float3	Tangent			:	TANGENT;
	float3	Binormal		:	BINORMAL;
	float4  BlendWeights	:	BLENDWEIGHT;
	float4  BlendIndices	:	BLENDINDICES;
	float2	uv0				:	TEXCOORD0;
	float2	uv1				:	TEXCOORD1;
	float2	uv2				:	TEXCOORD2;
	float2	uv3				:	TEXCOORD3;
 	float2	uv4				:	TEXCOORD4;
 	float2	uv5				:	TEXCOORD5;
 	float2	uv6				:	TEXCOORD6;
 	float2	uv7				:	TEXCOORD7;
};

struct POSTPROCESS_IN_OUT
{
	half4	Position	: POSITION;
	half2	uv0			: TEXCOORD0;
};


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////
***********************************************************************/

// dynamic light variables
float4x4	light_pos_rot[MAX_DYN_LIGHTS_PER_FACE]		: DynLightTransform;
float		light_radius[MAX_DYN_LIGHTS_PER_FACE]		: DynLightRadius;
float3		light_color[MAX_DYN_LIGHTS_PER_FACE]		: DynLightColor;
bool		light_use_texture[MAX_DYN_LIGHTS_PER_FACE]	: DynLightUseTexture;
int			light_count									: DynLightCount;

textureCUBE lightcubeTexture0						: DynLightTexture0;
textureCUBE lightcubeTexture1						: DynLightTexture1;
textureCUBE lightcubeTexture2						: DynLightTexture2;
textureCUBE lightcubeTexture3						: DynLightTexture3;

samplerCUBE samp_light_texture0 = sampler_state
	{Texture = (lightcubeTexture0);};
samplerCUBE samp_light_texture1 = sampler_state
	{Texture = (lightcubeTexture1);};
samplerCUBE samp_light_texture2 = sampler_state
	{Texture = (lightcubeTexture2);};
samplerCUBE samp_light_texture3 = sampler_state
	{Texture = (lightcubeTexture3);};

samplerCUBE samp_light_textures_field[MAX_DYN_LIGHTS_PER_FACE] = {
 sampler_state
 {texture = <lightcubeTexture0>;},
 sampler_state
 {texture = <lightcubeTexture1>;},
 sampler_state
 {texture = <lightcubeTexture2>;},
 sampler_state
 {texture = <lightcubeTexture3>;} 
};


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Standard postprocess vertex shader
////////////////////////////////////////////////////////////////////////
***********************************************************************/

POSTPROCESS_IN_OUT post_vs_main (POSTPROCESS_IN_OUT IN)
{
	POSTPROCESS_IN_OUT OUT;
	OUT.Position = IN.Position;
	OUT.uv0 = IN.uv0;
	return OUT;
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Standard dynamic lighting for static geometry (bsp)
////////////////////////////////////////////////////////////////////////
***********************************************************************/

/*
Using:

pass BSPDefDynLightPass0
{
  ALPHABLENDENABLE = TRUE;
  DESTBLEND = ONE;
  SRCBLEND = ONE;
  VertexShader = compile vs_1_1 vs_def_bsp_dynlight(0);
  PixelShader = compile ps_2_0 ps_def_bsp_dynlight(0, diff_tex_sampler, samp_light_texture0);
}
... similar passes for light 1,2,3
*/

struct VS_OUTPUT_DEF_BSP_DYNLIGHT
{
   float4	Position	:	POSITION0;
   float3	Normal		:	TEXCOORD0;
   float2	uv0			:	TEXCOORD1;
   float3	LightsDist	:	TEXCOORD2;
};

//---------------------------------------------
VS_OUTPUT_DEF_BSP_DYNLIGHT vs_def_bsp_dynlight (VS_INPUT_BSPVERTEX IN, uniform int lightIdx)
{
	VS_OUTPUT_DEF_BSP_DYNLIGHT OUT;
	OUT.Position = mul (IN.Position, matViewProjection);
	OUT.Normal = IN.Normal;
	OUT.uv0 = IN.uv0;
	OUT.LightsDist = light_pos_rot[lightIdx]._41_42_43 - IN.Position.xyz;
	return OUT;
}
//---------------------------------------------
float4 ps_def_bsp_dynlight (VS_OUTPUT_DEF_BSP_DYNLIGHT IN, uniform int lightIdx, uniform sampler2D samp_diff_texture, uniform samplerCUBE samp_light_texture) : COLOR0
{
	float4 color = 0;
	float LightAtt = length(IN.LightsDist) / light_radius[lightIdx];
	LightAtt = 1.0f - (LightAtt * LightAtt);
	if (LightAtt > 0.0f)
	{
		color = tex2D(samp_diff_texture, IN.uv0);
		color.xyz *= light_color[lightIdx] * LightAtt * \
					saturate(dot(normalize(IN.LightsDist), IN.Normal));
		if (light_use_texture[lightIdx])
		{
			IN.LightsDist = mul(-IN.LightsDist, (float3x3)light_pos_rot[lightIdx]);
			color.xyz *= texCUBE(samp_light_texture, IN.LightsDist);
		}
	}
	return color;
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Standard dynamic lighting for static geometry (bsp)
// in one pass with shader model 3.0
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass BSPDefDynLightOnePass
{
  ALPHABLENDENABLE = TRUE;
  DESTBLEND = ONE;
  SRCBLEND = ONE;
  VertexShader = compile vs_3_0 vs_def_bsp_dynlight_onepass();
  PixelShader = compile ps_3_0 ps_def_bsp_dynlight_onepass(diff_tex_sampler);
}
*/

struct VS_OUTPUT_DEF_BSP_DYNLIGHT_ONE_PASS
{
   float4	Position	:	POSITION0;
   float3	Normal		:	TEXCOORD0;
   float2	uv0			:	TEXCOORD1;
   float3	LightsDist[MAX_DYN_LIGHTS_PER_FACE]	:	TEXCOORD2;
};

//---------------------------------------------
VS_OUTPUT_DEF_BSP_DYNLIGHT_ONE_PASS vs_def_bsp_dynlight_onepass (VS_INPUT_BSPVERTEX IN)
{
	VS_OUTPUT_DEF_BSP_DYNLIGHT_ONE_PASS OUT;
	OUT.Position = mul (IN.Position, matViewProjection);
	OUT.Normal = IN.Normal;
	OUT.uv0 = IN.uv0;
	for (int i=0; i<MAX_DYN_LIGHTS_PER_FACE; i++)
		OUT.LightsDist[i] = light_pos_rot[i]._41_42_43 - IN.Position.xyz;
	return OUT;
}

//---------------------------------------------
float4 ps_def_bsp_dynlight_onepass (VS_OUTPUT_DEF_BSP_DYNLIGHT_ONE_PASS IN, \
			uniform int light_num,
			uniform sampler2D samp_diff_texture) : COLOR0
{
	float3 color = 0;
	float4 diffcolor = tex2D(samp_diff_texture, IN.uv0);

	for (int i=0; i<light_num; i++)
	{
		float LightAtt = length(IN.LightsDist[i]) / light_radius[i];
		LightAtt = 1.0f - (LightAtt * LightAtt);
		if (LightAtt > 0.0f)
		{
			float3 lightcolor = diffcolor.xyz * light_color[i] * LightAtt * \
						saturate(dot(normalize(IN.LightsDist[i]), IN.Normal));
			if (light_use_texture[i])
			{
				IN.LightsDist[i] = mul(-IN.LightsDist[i], (float3x3)light_pos_rot[i]);
				lightcolor.xyz *= texCUBE(samp_light_textures_field[i], IN.LightsDist[i]);
			}
			color += lightcolor;
		}
	}
	return float4(color, diffcolor.w);
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Normal map dynamic lighting for static geometry (bsp)
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass BSPNormalmapDynLightPass0
{
	ALPHABLENDENABLE = TRUE;
	DESTBLEND = ONE;
	SRCBLEND = ONE;
	VertexShader = compile vs_1_1 vs_normalmap_bsp_dynlight(0);
	PixelShader = compile ps_2_0 ps_normalmap_bsp_dynlight(0, samp_diff_texture, samp_norm_texture, samp_light_texture0);
}
... similar passes for light 1,2,3
*/

struct VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT
{
   float4 Position	:	POSITION0;
   float3 wView		:	TEXCOORD0;
   float3 LightsDist:	TEXCOORD1;
   float3 Normal	:	TEXCOORD2;
   float3 Tangent	:	TEXCOORD3;
   float3 Binormal	:	TEXCOORD4;   
   float2 uv0		:	TEXCOORD5;   
};

//---------------------------------------------
VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT vs_normalmap_bsp_dynlight (VS_INPUT_BSPVERTEX IN, uniform int lightIdx)
{
	VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT OUT;
	OUT.Position = mul (IN.Position, matViewProjection);
	OUT.wView = vec3CameraPosition - IN.Position.xyz;
	OUT.LightsDist = light_pos_rot[lightIdx]._41_42_43 - IN.Position.xyz;
	OUT.Normal = IN.Normal;
	OUT.Tangent = IN.Tangent;
	OUT.Binormal = IN.Binormal;
	OUT.uv0 = IN.uv0;
	return OUT;
}
//---------------------------------------------
float4 ps_normalmap_bsp_dynlight (VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT IN,
				uniform int lightIdx,
				uniform sampler2D samp_diff_texture,
				uniform sampler2D samp_norm_texture,
				uniform samplerCUBE samp_light_texture) : COLOR0
{
	float4 color = 0;

	float LightAtt = length(IN.LightsDist) / light_radius[lightIdx];
	LightAtt = 1.0f - (LightAtt * LightAtt);
	if (LightAtt > 0.0f)
	{
		// normal unpacking and transforming to world space
		float4 normal = tex2D(samp_norm_texture, IN.uv0);
		normal.xyz = 2.0f * normal.xyz - 1.0f;
		normal.xyz = normalize (normal.x * IN.Tangent + normal.y * IN.Binormal + normal.z * IN.Normal);
		// attenuation
		IN.LightsDist = normalize (IN.LightsDist);
		color = tex2D(samp_diff_texture, IN.uv0);
		color.xyz *= light_color[lightIdx] * LightAtt * \
				saturate (dot (IN.LightsDist, normal.xyz) + DIFFLIGHT_CORRECTION_DYNLIGHT);
		// specular reflection
		float3 reflectVec = reflect (-IN.LightsDist, normal.xyz);
		float3 spec = light_color[lightIdx] * LightAtt * \
				pow(saturate(dot(reflectVec, normalize(IN.wView))), SPEC_POWER_DYNLIGHT) * \
				saturate(dot(IN.LightsDist, IN.Normal));
		// specular map
		color.xyz += spec * normal.w;
		// cube map modulation
		if (light_use_texture[lightIdx])
		{
			IN.LightsDist = mul(-IN.LightsDist, (float3x3)light_pos_rot[lightIdx]);
			color.xyz *= texCUBE(samp_light_texture, IN.LightsDist);
		}
	}
	return color;
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Normal map dynamic lighting for static geometry (bsp)
// in one pass with shader model 3.0
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass BSPNormalmapDynLightOnePass
{
	ALPHABLENDENABLE = TRUE;
	DESTBLEND = ONE;
	SRCBLEND = ONE;
	VertexShader = compile vs_2_0 vs_normalmap_bsp_dynlight_onepass();
	PixelShader = compile ps_3_0 ps_normalmap_bsp_dynlight_onepass(samp_diff_texture, samp_norm_texture);
}
*/


struct VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT_ONE_PASS
{
   float4 Position	:	POSITION0;
   float3 wView		:	TEXCOORD0;
   float3 Normal	:	TEXCOORD1;
   float3 Tangent	:	TEXCOORD2;
   float3 Binormal	:	TEXCOORD3;
   float2 uv0		:	TEXCOORD4;
   float3 LightsDist[MAX_DYN_LIGHTS_PER_FACE]	:	TEXCOORD5;
};

//---------------------------------------------
VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT_ONE_PASS vs_normalmap_bsp_dynlight_onepass (VS_INPUT_BSPVERTEX IN)
{
	VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT_ONE_PASS OUT;
	OUT.Position = mul (IN.Position, matViewProjection);
	OUT.wView = vec3CameraPosition - IN.Position.xyz;
	OUT.Normal = IN.Normal;
	OUT.Tangent = IN.Tangent;
	OUT.Binormal = IN.Binormal;
	OUT.uv0 = IN.uv0;
	for (int i=0; i<MAX_DYN_LIGHTS_PER_FACE; i++)
		OUT.LightsDist[i] = light_pos_rot[i]._41_42_43 - IN.Position.xyz;
	return OUT;
}
//---------------------------------------------
float4 ps_normalmap_bsp_dynlight_onepass(VS_OUTPUT_NORMALMAP_BSP_DYNLIGHT_ONE_PASS IN,
				uniform int light_num,
				uniform sampler2D samp_diff_texture,
				uniform sampler2D samp_norm_texture) : COLOR0
{
	float3 color = 0;
	float4 diffcolor = tex2D(samp_diff_texture, IN.uv0);
	float4 normal = tex2D(samp_norm_texture, IN.uv0);
	normal.xyz = 2.0f * normal.xyz - 1.0f;
	// transform normal from tangent to world space
	normal.xyz = normalize(normal.x * IN.Tangent + normal.y * IN.Binormal + normal.z * IN.Normal);
	float3 V = normalize (IN.wView);

	for (int i=0; i<light_num; i++)
	{
		float LightAtt = length(IN.LightsDist[i]) / light_radius[i];
		LightAtt = 1.0f - (LightAtt * LightAtt);
		if (LightAtt > 0.0f)
		{
			// diffuse lighting
			IN.LightsDist[i] = normalize (IN.LightsDist[i]);
			float3 lightolor = diffcolor.xyz * light_color[i] * LightAtt * \
				saturate (dot (IN.LightsDist[i], normal.xyz) + DIFFLIGHT_CORRECTION_DYNLIGHT);
			// specular reflection
			float3 reflectVec = reflect (-IN.LightsDist[i], normal.xyz);
			float3 spec = light_color[i] * LightAtt * \
					pow(saturate(dot(reflectVec, V)), SPEC_POWER_DYNLIGHT) * \
					saturate(dot(IN.LightsDist[i], IN.Normal));
			lightolor += spec * normal.w;
			// cube map modulation
			if (light_use_texture[i])
			{
				IN.LightsDist[i] = mul(-IN.LightsDist[i], (float3x3)light_pos_rot[i]);
				lightolor *= texCUBE(samp_light_textures_field[i], IN.LightsDist[i]);
			}
			color += lightolor;
		}
	}
	return float4(color, diffcolor.w);
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Parallax dynamic lighting for static geometry (bsp)
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass BSPParallaxDynLightPass0
{
	ALPHABLENDENABLE = TRUE;
	DESTBLEND = ONE;
	SRCBLEND = ONE;
	VertexShader = compile vs_1_1 vs_parallax_bsp_dynlight(0);
	PixelShader = compile ps_2_0 ps_parallax_bsp_dynlight(0, spec_factor, diffTexture, normalTexture, samp_light_texture0);
}
... similar passes for light 1,2,3
*/


struct VS_OUTPUT_PARALLAX_BSP_DYNLIGHT
{
	float4 Position		:	POSITION;
	float3 wView		:	TEXCOORD0;
	float3 LightsDist	:	TEXCOORD1;
	float3 Normal		:	TEXCOORD2;
	float3 Tangent		:	TEXCOORD3;
	float3 Binormal		:	TEXCOORD4;
	float2 uv0			:	TEXCOORD5;
};

VS_OUTPUT_PARALLAX_BSP_DYNLIGHT vs_parallax_bsp_dynlight (VS_INPUT_BSPVERTEX IN, uniform int lightIdx)
{
	VS_OUTPUT_PARALLAX_BSP_DYNLIGHT OUT;
	// vertex position in clip space
	OUT.Position = mul(IN.Position, matViewProjection);	
	OUT.wView = vec3CameraPosition - IN.Position.xyz;
	OUT.LightsDist = light_pos_rot[lightIdx]._41_42_43 - IN.Position.xyz;
	OUT.Normal = IN.Normal;
	OUT.Tangent = IN.Tangent;
	OUT.Binormal = IN.Binormal;
	OUT.uv0 = IN.uv0;
	return OUT;
}

//---------------------------------------------
float4 ps_parallax_bsp_dynlight (VS_OUTPUT_PARALLAX_BSP_DYNLIGHT IN,
				uniform int lightIdx,
				uniform float spec_factor,
				uniform sampler2D samp_diff_texture,
				uniform sampler2D samp_norm_texture,
				uniform samplerCUBE samp_light_texture) : COLOR0
{
	float4 color = 0;

	float LightAtt = length(IN.LightsDist) / light_radius[lightIdx];
	LightAtt = 1.0f - (LightAtt * LightAtt);
	if (LightAtt > 0.0f)
	{
		// parallax code
		float3 V = normalize (IN.wView);
		float3x3 matTBN = float3x3(IN.Tangent, IN.Binormal, IN.Normal);
		float height = tex2D(samp_norm_texture, IN.uv0).w * (2.0f*PARALLAX_DEPTHFACTOR) - PARALLAX_DEPTHFACTOR;
		IN.uv0 += height * mul(matTBN, -V);
		// normal unpacking and transforming to world space
		float4 normal = tex2D(samp_norm_texture, IN.uv0);
		normal.xyz = 2.0f * normal.xyz - 1.0f;
		normal.xyz = (normal.x * IN.Tangent) + (normal.y * IN.Binormal) + (normal.z * IN.Normal);
		normal.xyz = normalize (normal.xyz);
		// attenuation
		IN.LightsDist = normalize (IN.LightsDist);
		color = tex2D(samp_diff_texture, IN.uv0);
		color.xyz *= light_color[lightIdx] * LightAtt * \
			saturate (dot (IN.LightsDist, normal.xyz) + DIFFLIGHT_CORRECTION_DYNLIGHT);
		// specular reflection
		float3 reflectVec = reflect (-IN.LightsDist, normal.xyz);
		float3 spec = light_color[lightIdx] * LightAtt * \
				pow(saturate(dot(reflectVec, V)), SPEC_POWER_DYNLIGHT) * \
				saturate(dot(IN.LightsDist, IN.Normal));
		// specular map
		color.xyz += spec * spec_factor;
		// cube map modulation
		if (light_use_texture[lightIdx])
		{
			IN.LightsDist = mul(-IN.LightsDist, (float3x3)light_pos_rot[lightIdx]);
			color.xyz *= texCUBE(samp_light_texture, IN.LightsDist);
		}
	}
	return color;
}



/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Parallax dynamic lighting for static geometry (bsp)
// in one pass with shader model 3.0
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass BSPParallaxDynLightOnePass
{
	ALPHABLENDENABLE = TRUE;
	DESTBLEND = ONE;
	SRCBLEND = ONE;
	VertexShader = compile vs_3_0 vs_parallax_bsp_dynlight_onepass();
	PixelShader = compile ps_3_0 ps_parallax_bsp_dynlight_onepass(spec_factor, samp_diff_texture, samp_norm_texture);
}
*/

struct VS_OUTPUT_PARALLAX_BSP_DYNLIGHT_ONE_PASS
{
   float4 Position	:	POSITION;
   float3 wView		:	TEXCOORD0;
   float3 Normal	:	TEXCOORD1;
   float3 Tangent	:	TEXCOORD2;
   float3 Binormal	:	TEXCOORD3;
   float2 uv0		:	TEXCOORD4;
   float3 LightsDist[MAX_DYN_LIGHTS_PER_FACE]	:	TEXCOORD5;
};

//---------------------------------------------
VS_OUTPUT_PARALLAX_BSP_DYNLIGHT_ONE_PASS vs_parallax_bsp_dynlight_onepass (VS_INPUT_BSPVERTEX IN)
{
	VS_OUTPUT_PARALLAX_BSP_DYNLIGHT_ONE_PASS OUT;
	// vertex position in clip space
	OUT.Position = mul(IN.Position, matViewProjection);	
	OUT.wView = vec3CameraPosition - IN.Position.xyz;
	OUT.Normal = IN.Normal;
	OUT.Tangent = IN.Tangent;
	OUT.Binormal = IN.Binormal;
	OUT.uv0 = IN.uv0;
	for (int i=0; i<MAX_DYN_LIGHTS_PER_FACE; i++)
		OUT.LightsDist[i] = light_pos_rot[i]._41_42_43 - IN.Position.xyz;
	return OUT;
}

//---------------------------------------------
float4 ps_parallax_bsp_dynlight_onepass (VS_OUTPUT_PARALLAX_BSP_DYNLIGHT_ONE_PASS IN,
				uniform int light_num,
				uniform float spec_factor,
				uniform sampler2D samp_diff_texture,
				uniform sampler2D samp_norm_texture) : COLOR0
{
	float3 color = 0;
	// parallax code
	float3 V = normalize (IN.wView);
	float3x3 matTBN = float3x3(IN.Tangent, IN.Binormal, IN.Normal);
	float height = tex2D(samp_norm_texture, IN.uv0).w * (2.0f*PARALLAX_DEPTHFACTOR) - PARALLAX_DEPTHFACTOR;
	IN.uv0 += height * mul(matTBN, -V);
	// normal unpacking and transforming to world space
	float4 normal = tex2D(samp_norm_texture, IN.uv0);
	normal.xyz = 2.0f * normal.xyz - 1.0f;
	normal.xyz = normalize(normal.x * IN.Tangent + normal.y * IN.Binormal + normal.z * IN.Normal);

	float4 diffColor = tex2D(samp_diff_texture, IN.uv0);

	for (int i=0; i<light_num; i++)
	{
		float LightAtt = length(IN.LightsDist[i]) / light_radius[i];
		LightAtt = 1.0f - (LightAtt * LightAtt);
		if (LightAtt > 0.0f)
		{
			IN.LightsDist[i] = normalize (IN.LightsDist[i]);
			// attenuation
			float3 lightolor = diffColor.xyz * light_color[i] * LightAtt * \
				saturate (dot (IN.LightsDist[i], normal.xyz) + DIFFLIGHT_CORRECTION_DYNLIGHT);
			// specular reflection
			float3 reflectVec = reflect (-IN.LightsDist[i], normal.xyz);
			float3 spec = light_color[i] * LightAtt * \
					pow(saturate(dot(reflectVec, V)), SPEC_POWER_DYNLIGHT) * \
					saturate(dot(IN.LightsDist[i], IN.Normal));
			// specular map
			lightolor += spec * spec_factor;
			// cube map modulation
			if (light_use_texture[i])
			{
				IN.LightsDist[i] = mul(-IN.LightsDist[i], (float3x3)light_pos_rot[i]);
				lightolor *= texCUBE(samp_light_textures_field[i], IN.LightsDist[i]);
			}
			color += lightolor;
		}
	}
	return float4(color, diffColor.w);
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Standard dynamic lighting for dynamic models
////////////////////////////////////////////////////////////////////////
***********************************************************************/

/*
Using:

pass DynModelDefDynLightPass0
{
	ALPHABLENDENABLE = TRUE;
	DESTBLEND = ONE;
	SRCBLEND = ONE;
	VertexShader = compile vs_1_1 vs_def_dynmodel_dynlight(0, matWorld, matWorldViewProj);
	PixelShader = compile ps_2_0 ps_def_dynmodel_dynlight(0, diff_tex_sampler, samp_light_texture0);
}
... similar passes for light 1,2,3
*/

struct VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT
{
   float4	Position	:	POSITION0;
   float3	Normal		:	TEXCOORD0;
   float2	uv0			:	TEXCOORD1;
   float3	LightsDist	:	TEXCOORD2;
};

//---------------------------------------------
VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT vs_def_dynmodel_dynlight (VS_INPUT_DYNMODELS IN, \
						uniform int lightIdx, \
						uniform float4x4 matrixWorld, \
						uniform float4x4 matrixWorldViewProj)
{
	VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT OUT;
	OUT.Position = mul (IN.Position, matrixWorldViewProj);
	float4 worldpos = mul (IN.Position, matrixWorld);
	OUT.Normal = mul (IN.Normal, (float3x3)matrixWorld);
	OUT.uv0 = IN.uv0;
	OUT.LightsDist = light_pos_rot[lightIdx]._41_42_43 - worldpos.xyz;
	return OUT;
}
//---------------------------------------------
float4 ps_def_dynmodel_dynlight (VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT IN, \
						uniform int lightIdx, \
						uniform sampler2D samp_diff_texture, \
						uniform samplerCUBE samp_light_texture) : COLOR
{
	float4 color = 0;
	float LightAtt = length(IN.LightsDist) / light_radius[lightIdx];
	LightAtt = 1.0f - (LightAtt * LightAtt);
	if (LightAtt > 0)
	{
		color = tex2D(samp_diff_texture, IN.uv0);
		//IN.Normal = normalize(IN.Normal);		// potrebne ???
		color.xyz *= light_color[lightIdx] * LightAtt * \
					saturate(dot(normalize(IN.LightsDist), IN.Normal));
		if (light_use_texture[lightIdx])
		{
			IN.LightsDist = mul(-IN.LightsDist, (float3x3)light_pos_rot[lightIdx]);
			color.xyz *= texCUBE(samp_light_texture, IN.LightsDist);
		}
	}
	return color;
}



/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Standard dynamic lighting for dynamic models
// in one pass with shader model 3.0
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass DynmodelDefDynLightOnePass
{
  ALPHABLENDENABLE = TRUE;
  DESTBLEND = ONE;
  SRCBLEND = ONE;
  VertexShader = compile vs_3_0 vs_def_dynmodel_dynlight_onepass(matWorld, matWorldViewProj);
  PixelShader = compile ps_3_0 ps_def_dynmodel_dynlight_onepass(diff_tex_sampler);
}
*/

struct VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT_ONE_PASS
{
   float4	Position	:	POSITION0;
   float3	Normal		:	TEXCOORD0;
   float2	uv0			:	TEXCOORD1;
   float3	LightsDist[MAX_DYN_LIGHTS_PER_FACE]	:	TEXCOORD2;
};

//---------------------------------------------
VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT_ONE_PASS vs_def_dynmodel_dynlight_onepass (VS_INPUT_DYNMODELS IN, \
						uniform float4x4 matrixWorld, \
						uniform float4x4 matrixWorldViewProj)
{
	VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT_ONE_PASS OUT;
	OUT.Position = mul (IN.Position, matrixWorldViewProj);
	float4 worldpos = mul (IN.Position, matrixWorld);
	OUT.Normal = mul (IN.Normal, (float3x3)matrixWorld);
	OUT.uv0 = IN.uv0;
	for (int i=0; i<MAX_DYN_LIGHTS_PER_FACE; i++)
		OUT.LightsDist[i] = light_pos_rot[i]._41_42_43 - worldpos.xyz;
	return OUT;
}

//---------------------------------------------
float4 ps_def_dynmodel_dynlight_onepass (VS_OUTPUT_DEF_DYNMODEL_DYNLIGHT_ONE_PASS IN, \
						uniform int light_num,
						uniform sampler2D samp_diff_texture) : COLOR0
{
	float3 color = 0;
	float4 diffcolor = tex2D(samp_diff_texture, IN.uv0);

	for (int i=0; i<light_num; i++)
	{
		float LightAtt = length(IN.LightsDist[i]) / light_radius[i];
		LightAtt = 1.0f - (LightAtt * LightAtt);
		if (LightAtt > 0)
		{
			float3 lightcolor = diffcolor.xyz * light_color[i] * LightAtt * \
						saturate(dot(normalize(IN.LightsDist[i]), IN.Normal));
			if (light_use_texture[i])
			{
				IN.LightsDist[i] = mul(-IN.LightsDist[i], (float3x3)light_pos_rot[i]);
				lightcolor.xyz *= texCUBE(samp_light_textures_field[i], IN.LightsDist[i]);
			}
			color += lightcolor;
		}
	}
	return float4(color, diffcolor.w);
}



/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Normal map dynamic lighting for dynamic models
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass DynModelNormalmapDynLightPass0
{
	ALPHABLENDENABLE = TRUE;
	DESTBLEND = ONE;
	SRCBLEND = ONE;
	VertexShader = compile vs_1_1 vs_normalmap_dynmodel_dynlight(0, matWorld);
	PixelShader = compile ps_2_0 ps_normalmap_dynmodel_dynlight(0, diff_tex_sampler, samp_light_texture0);
}
... similar passes for light 1,2,3
*/

struct VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT
{
   float4 Position	:	POSITION0;
   float3 wView		:	TEXCOORD0;
   float3 LightsDist:	TEXCOORD1;
   float3 Normal	:	TEXCOORD2;
   float3 Tangent	:	TEXCOORD3;
   float3 Binormal	:	TEXCOORD4;
   float2 uv0		:	TEXCOORD5;
   float2 uv1		:	TEXCOORD6;
};

//---------------------------------------------
VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT vs_normalmap_dynmodel_dynlight (VS_INPUT_DYNMODELS IN, \
						uniform int lightIdx, \
						uniform float4x4 matrixWorld)
{
	VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT OUT;
	float4 worldpos = mul (IN.Position, matrixWorld);
	OUT.Position = mul (worldpos, matViewProjection);
	OUT.wView = vec3CameraPosition - worldpos.xyz;
	OUT.LightsDist = light_pos_rot[lightIdx]._41_42_43 - worldpos.xyz;
	float3x3 world = (float3x3)matrixWorld;
	OUT.Normal = mul (IN.Normal, world);
	OUT.Tangent = mul (IN.Tangent, world);
	OUT.Binormal = mul (IN.Binormal, world);
	OUT.uv0 = IN.uv0;
	OUT.uv1 = IN.uv1;
	return OUT;
}
//---------------------------------------------
float4 ps_normalmap_dynmodel_dynlight (VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT IN, \
				uniform int lightIdx, \
				uniform sampler2D samp_diff_texture, \
				uniform sampler2D samp_norm_texture, \
				uniform samplerCUBE samp_light_texture) : COLOR
{
	float4 color = 0;

	float LightAtt = length(IN.LightsDist) / light_radius[lightIdx];
	LightAtt = 1.0f - (LightAtt * LightAtt);
	if (LightAtt > 0.0f)
	{
		// normal unpacking and transforming to world space
		float4 normal = tex2D(samp_norm_texture, IN.uv1);
		normal.xyz = 2.0f * normal.xyz - 1.0f;
		normal.xyz = normalize (normal.x * IN.Tangent + normal.y * IN.Binormal + normal.z * IN.Normal);
		// attenuation
		IN.LightsDist = normalize (IN.LightsDist);
		color = tex2D(samp_diff_texture, IN.uv0);
		color.xyz *= light_color[lightIdx] * LightAtt * \
				saturate (dot (IN.LightsDist, normal.xyz) + DIFFLIGHT_CORRECTION_DYNLIGHT);
		// specular reflection
		float3 reflectVec = reflect (-IN.LightsDist, normal.xyz);
		float3 spec = light_color[lightIdx] * LightAtt * \
				pow(saturate(dot(reflectVec, normalize(IN.wView))), SPEC_POWER_DYNLIGHT) * \
				saturate(dot(IN.LightsDist, IN.Normal));
		// specular map
		color.xyz += spec * normal.w;
		// cube map modulation
		if (light_use_texture[lightIdx])
		{
			IN.LightsDist = mul(-IN.LightsDist, (float3x3)light_pos_rot[lightIdx]);
			color.xyz *= texCUBE(samp_light_texture, IN.LightsDist);
		}
	}
	return color;
}


/***********************************************************************
////////////////////////////////////////////////////////////////////////
// Normal map dynamic lighting for dynamic models
// in one pass with shader model 3.0
////////////////////////////////////////////////////////////////////////
***********************************************************************/
/*
Using:

pass DynmodelNormalmapDynLightOnePass
{
  ALPHABLENDENABLE = TRUE;
  DESTBLEND = ONE;
  SRCBLEND = ONE;
  VertexShader = compile vs_3_0 vs_normalmap_dynmodel_dynlight_onepass(matWorld);
  PixelShader = compile ps_3_0 ps_normalmap_dynmodel_dynlight_onepass(light_num, diff_tex_sampler, normal_tex_sampler);
}
*/

struct VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT_ONE_PASS
{
   float4 Position	:	POSITION0;
   float3 wView		:	TEXCOORD0;
   float3 Normal	:	TEXCOORD1;
   float3 Tangent	:	TEXCOORD2;
   float3 Binormal	:	TEXCOORD3;
   float2 uv0		:	TEXCOORD4;
   float2 uv1		:	TEXCOORD5;
   float3 LightsDist[MAX_DYN_LIGHTS_PER_FACE]	:	TEXCOORD6;
};

//---------------------------------------------
VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT_ONE_PASS vs_normalmap_dynmodel_dynlight_onepass ( \
						VS_INPUT_DYNMODELS IN, \
						uniform float4x4 matrixWorld)
{
	VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT_ONE_PASS OUT;
	float4 worldpos = mul (IN.Position, matrixWorld);
	OUT.Position = mul (worldpos, matViewProjection);
	OUT.wView = vec3CameraPosition - worldpos.xyz;
	for (int i=0; i<MAX_DYN_LIGHTS_PER_FACE; i++)
		OUT.LightsDist[i] = light_pos_rot[i]._41_42_43 - worldpos.xyz;
	float3x3 world = (float3x3)matrixWorld;
	OUT.Normal = mul (IN.Normal, world);
	OUT.Tangent = mul (IN.Tangent, world);
	OUT.Binormal = mul (IN.Binormal, world);
	OUT.uv0 = IN.uv0;
	OUT.uv1 = IN.uv1;
	return OUT;
}

//---------------------------------------------
float4 ps_normalmap_dynmodel_dynlight_onepass (VS_OUTPUT_NORMALMAP_DYNMODEL_DYNLIGHT_ONE_PASS IN, \
						uniform int light_num,
						uniform sampler2D samp_diff_texture, \
						uniform sampler2D samp_norm_texture) : COLOR0
{
	float3 color = 0;
	float4 diffcolor = tex2D(samp_diff_texture, IN.uv0);
	float4 normal = tex2D(samp_norm_texture, IN.uv1);
	normal.xyz = 2.0f * normal.xyz - 1.0f;
	// transform normal from tangent to world space
	normal.xyz = normalize(normal.x * IN.Tangent + normal.y * IN.Binormal + normal.z * IN.Normal);
	float3 V = normalize (IN.wView);

	for (int i=0; i<light_num; i++)
	{
		float LightAtt = length(IN.LightsDist[i]) / light_radius[i];
		LightAtt = 1.0f - (LightAtt * LightAtt);
		if (LightAtt > 0.0f)
		{
			// diffuse lighting
			IN.LightsDist[i] = normalize (IN.LightsDist[i]);
			float3 lightolor = diffcolor.xyz * light_color[i] * LightAtt * \
				saturate (dot (IN.LightsDist[i], normal.xyz) + DIFFLIGHT_CORRECTION_DYNLIGHT);
			// specular reflection
			float3 reflectVec = reflect (-IN.LightsDist[i], normal.xyz);
			float3 spec = light_color[i] * LightAtt * \
					pow(saturate(dot(reflectVec, V)), SPEC_POWER_DYNLIGHT) * \
					saturate(dot(IN.LightsDist[i], IN.Normal));
			lightolor += spec * normal.w;
			// cube map modulation
			if (light_use_texture[i])
			{
				IN.LightsDist[i] = mul(-IN.LightsDist[i], (float3x3)light_pos_rot[i]);
				lightolor *= texCUBE(samp_light_textures_field[i], IN.LightsDist[i]);
			}
			color += lightolor;
		}
	}
	return float4(color, diffcolor.w);
}
