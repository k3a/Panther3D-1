
shared float4x4 matViewProjection : ViewProjection;
shared float3 vec3CameraPosition : CameraPosition;


#define	MAX_BILLBOARDS		120

float4	quad_light_size[MAX_BILLBOARDS]		: LightSize;
float4	quad_position_rot[MAX_BILLBOARDS]	: QuadPositionRot;
bool	bBillboard							: Billboarded;
bool	bAnimate							: Animated;
float	fTime								: Time;
float	fSquaredFadeDistance				: SquaredFadeDistance;

texture2D alphaTex : GrassTexture;

sampler2D alphaTexture = sampler_state
{
   Texture = (alphaTex);
};

//////////////////////////////////////////
struct VS_INPUT
{
   float4	Position	: POSITION0;
   float2	uv			: TEXCOORD0;
};

struct VS_OUTPUT
{
   float4	Position	: POSITION0;
   float2	uv			: TEXCOORD0;
   float4	pixelInfo	: TEXCOORD1;
};

//////////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT IN)
{
	VS_OUTPUT OUT;

	float4	newPosition;
	int		intIdx = (int)IN.Position.z;
	float3	camera_distance = vec3CameraPosition - quad_position_rot[intIdx].xyz;

	if (bAnimate && (IN.Position.y > 0))
	{
		fTime *= ((quad_position_rot[intIdx].w / 3.14159f) * 2.5f - 0.5f);
		IN.Position.x += sin (fTime) * 0.02f;
	}
	
	if (bBillboard)
	{
		float3	rightVector = normalize (float3 (-camera_distance.z, 0, camera_distance.x));	// camera_distance cross <0,1,0>
		newPosition.xz = rightVector.xz * IN.Position.x;
	}
	else
	{
		sincos (quad_position_rot[intIdx].w, newPosition.z, newPosition.x);
		newPosition.xz *= IN.Position.x;	
	}

	newPosition.y = IN.Position.y;
	newPosition.xyz *= quad_light_size[intIdx].w;
	newPosition.xyz += quad_position_rot[intIdx].xyz;
	newPosition.w = 1.0f;

	OUT.Position = mul (newPosition, matViewProjection);
	OUT.uv = IN.uv;
	OUT.pixelInfo.xyz = quad_light_size[intIdx].xyz;
	// transparency
	float	transparency = dot(camera_distance, camera_distance) / fSquaredFadeDistance;
	OUT.pixelInfo.w = pow (saturate(transparency), 8.0f);

	return OUT;
}



//////////////////////////////////////////
float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	float4 outColor = tex2D (alphaTexture, IN.uv);
	outColor.xyz *= IN.pixelInfo.xyz;	// lighting
	outColor.w -= IN.pixelInfo.w;		// fade_distance alpha
	return outColor;
}


//--------------------------------------------------------------//
// Technique Section
//--------------------------------------------------------------//
technique spec_grass_model2
{
   pass Pass_0
   {
//    ALPHABLENDENABLE = TRUE;
//    SRCBLEND = SRCALPHA;
//	  DESTBLEND = INVSRCALPHA;
//	  CULLMODE = NONE;
/*
	  // debug !!!
	  ALPHABLENDENABLE = FALSE;
	  ALPHATESTENABLE = TRUE;
	  ALPHAREF=1;
	  ZWRITEENABLE=TRUE;
	  // debug !!!
*/
      VertexShader = compile vs_2_0 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique spec_grass_model3
{
   pass Pass_0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}
