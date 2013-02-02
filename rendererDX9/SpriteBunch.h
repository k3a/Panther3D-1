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
#include "IP3DSpriteBunch.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

#define NUM_SPRITES 128
#define MAX_SHADERS 16

// DOCASNE!!! UDELAT WRAPPER NA SHADERY
#include "d3dx9.h" 
// -------------------------

struct SpriteShader
{
	char name[32];
	ID3DXEffect *effect;
	int			refCount;
	_W64 unsigned int hArray1;
	_W64 unsigned int hArray2;
	_W64 unsigned int hTexture;
};

class CP3DSpriteBunch : public IP3DSpriteBunch
{
public:
	CP3DSpriteBunch();
	~CP3DSpriteBunch();
	bool Init();
	SHADER AddShader(const char* shaderName);
	bool RemoveShader(const char* shaderName);
	bool RemoveShader(SHADER shaderHandle);
	void Render(int numSprites, IP3DTexture *pTex, SHADER shaderHandle, P3DVector4D *vArray1, P3DVector4D *vArray2=NULL);
	bool FindParams(FXParamBatch* paramsArray, int numParams);
	bool SetParams(FXParamBatch* paramsArray, int numParams);
private:
	bool RemoveAllShaders();
};