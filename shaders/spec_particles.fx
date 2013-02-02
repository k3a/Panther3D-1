shared float4x4 matViewProjection : ViewProjection;
shared float3 vec3CameraPosition : CameraPosition;


float4	quad_position_size[128]	: Array1;

texture2D Tex : Texture;

sampler2D sTex = sampler_state
{
   Texture = (Tex);
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
   float2	uv		: TEXCOORD0;
 //  float4	pixelInfo	: TEXCOORD1;
};

//////////////////////////////////////////
VS_OUTPUT vs_main (VS_INPUT IN)
{
	VS_OUTPUT OUT;

	float4	newPosition;
	int	intIdx = (int)IN.Position.z;
	float3	camera_distance = vec3CameraPosition - quad_position_size[intIdx].xyz;

	float3	rightVector = normalize (float3 (-camera_distance.z, 0, camera_distance.x));
	newPosition.xz = rightVector.xz * IN.Position.x;

	newPosition.y = IN.Position.y;
	newPosition.xyz *= quad_position_size[intIdx].w;
	newPosition.xyz += quad_position_size[intIdx].xyz;
	newPosition.w = 1.0f;

	OUT.Position = mul (newPosition, matViewProjection);
	OUT.uv = IN.uv;
	//OUT.pixelInfo.xyz = quad_light_size[intIdx].xyz;
	// transparency
	//float	transparency = dot(camera_distance, camera_distance) / fSquaredFadeDistance;
	//OUT.pixelInfo.w = pow (saturate(transparency), 8.0f);

	return OUT;
}



//////////////////////////////////////////
float4 ps_main (VS_OUTPUT IN) : COLOR0
{
	float4 outColor = tex2D (sTex, IN.uv);
	//outColor.xyz *= IN.pixelInfo.xyz;	// lighting
	//outColor.w -= IN.pixelInfo.w;		// fade_distance alpha
	return outColor;
}


//--------------------------------------------------------------//
// Technique Section
//--------------------------------------------------------------//
technique t2
{
   pass Pass_0
   {
      ALPHABLENDENABLE = TRUE;
      CULLMODE = NONE;
      ZWRITEENABLE=TRUE;

      VertexShader = compile vs_2_0 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}

technique t3
{
   pass Pass_0
   {
      ALPHABLENDENABLE = TRUE;
      CULLMODE = NONE;
      ZWRITEENABLE=TRUE;

      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_2_0 ps_main();
   }
}
