//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Sprite bunch - uses shader instancing (grass, particles, ...)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include "IP3DMaterialManager.h"
#include "IP3DTexture.h"

typedef int SHADER;

struct FXParamBatch
{
	/** Will be converted to paramHandle after call of IP3DSpriteBunch::FindParams() */
	const char paramName[32];
	FXSemanticsType paramType;
	unsigned int    paramHandle;
	/** Must be pointer in case of single value or array (pointer to the first element) in case of array (e.g. flaot3) */
	void*			paramValue;
};

/**
Sprite bunch renderer. Uses shader instancing. 
You can draw up to 128 sprites which consists of 4 vertices (two triangles) with ONE DRAW CALL. 
But a real number of sprites in bunch depends on custom shader parameters used in current shader.

Shader must have texture with semantic name Texture and can have two float4[128] arrays with semantics Array1 and Array2.

Shader must have two techniques, t2 for shader model 2.0 and t3 for shader model 3.0 (but can use the same vert. and pix. shaders).
*/
class IP3DSpriteBunch : public IP3DBaseInterface
{
public:
	/** Adds shader or increment shader reference count if it was previously added. Shader handle for rendering will be returned. When this shader is no longer used, call RemoveShader - this decrements ref. count or delete shader from memory... \param shaderName Shader name (e.g. spec_particles.fx) \see RemoveShader */
	virtual SHADER AddShader(const char* shaderName)=0;
	/** Removes shader / decrement ref. count or delete from memory \see AddShader */
	virtual bool RemoveShader(const char* shaderName)=0;
	/** Removes shader / decrement ref. count or delete from memory \see AddShader */
	virtual bool RemoveShader(SHADER shaderHandle)=0;
	/** Set parameters arrays and render sprite bunch with numSprites sprites. Previously set parameters via SetParam will be used... 
	\param numSprites Number of sprites to be rendered 
	\param vArray1 Current data in array with size of 128. This will be send to shader as Array1. This array can be used in many ways. For example position(float3) and rotation(float) or light information for every sprite. 
	\param vArray2 Current data in array with size of 128. This will be send to shader as Array2. This array can be used in many ways. For example position(float3) and rotation(float) or light information for every sprite.  
	\param pTex Sprite texture :)*/
	virtual void Render(int numSprites, IP3DTexture *pTex, SHADER shaderHandle, P3DVector4D *vArray1, P3DVector4D *vArray2=NULL)=0;
	/** This must be called before SetParams (once, at shader loading time), otherwise parameters won't be set! */
	virtual bool FindParams(FXParamBatch* paramsArray, int numParams)=0;
	/** Set parameters array. FindParams must be called before (at shader loading time)! \see FindParams */
	virtual bool SetParams(FXParamBatch* paramsArray, int numParams)=0;
};

#define IP3DRENDERER_SPRITEBUNCH "P3DSpriteBunch_1" // nazev ifacu