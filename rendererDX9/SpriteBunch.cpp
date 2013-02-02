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
#include "SpriteBunch.h"
#include "IP3DVertexBuffer.h"
#include "common.h"

REGISTER_SINGLE_CLASS(CP3DSpriteBunch, IP3DRENDERER_SPRITEBUNCH);

static CP3DVertexBuffer s_vertexBuffer;
static CP3DIndexBuffer s_indexBuffer;
static SpriteShader s_shaders[MAX_SHADERS];

struct VertexPosTex
{
	P3DXPoint3D pos;
	P3DXPoint2D tex;
};

CP3DSpriteBunch::CP3DSpriteBunch()
{
	for(int i=0; i<MAX_SHADERS; i++)
		s_shaders[i].refCount = 0;
}

CP3DSpriteBunch::~CP3DSpriteBunch()
{
	RemoveAllShaders();
}

bool CP3DSpriteBunch::RemoveAllShaders()
{
	for(int i=0; i<MAX_SHADERS; i++)
	{
		if (s_shaders[i].refCount>0)
		{
			s_shaders[i].effect->Release();
			s_shaders[i].refCount=0;
		}
	}

	return true;
}

SHADER CP3DSpriteBunch::AddShader(const char* shaderName)
{
	int volny=-1;
	for(int i=0; i<MAX_SHADERS; i++)
	{
		// je v cache?
		if (!strcmpi(s_shaders[i].name, shaderName))
		{
			s_shaders[i].refCount++;
			return i;
		}
		// je volny?
		if (s_shaders[i].refCount==0) volny=i;
	}
	// PLNO!!!!
	if (volny==-1)
	{
		CON(MSG_CON_DEBUG, "Sprite bunch: Max shaders (%d) exceeded!", MAX_SHADERS);
		return -1; //plno
	}

	// mame volny shader
	s_shaders[volny].refCount++;
	strcpy(s_shaders[volny].name, shaderName);

	// nacteni do pameti
	BYTE *tmpData; ULONG tmpDataSize=0;
	FSFILE* tmpFile; char tmpFName[MAX_PATH]="shaders\\";
	strcat(tmpFName, shaderName);
	tmpFile = g_pFS->Load(tmpFName, tmpData, tmpDataSize);
	if (!tmpFile) 
	{
		CON(MSG_CON_ERR, "Sprite bunch: Can't load %s shader to memory!", shaderName);
		s_shaders[volny].refCount = 0;
		return -1;
	}

	// nacteni efektu
	if (FAILED (D3DXCreateEffectEx (g_pD3DDevice, tmpData, (UINT)tmpDataSize, \
		NULL, NULL, NULL, D3DXFX_NOT_CLONEABLE, (LPD3DXEFFECTPOOL)g_pMaterialManager->GetPoolHandle(), \
		&s_shaders[volny].effect, NULL)))
	{
		CON(MSG_CON_ERR, "Sprite bunch: Can't compile %s shader!", shaderName);
		g_pFS->UnLoad(tmpFile, tmpData);
		s_shaders[volny].refCount = 0;
		return -1;
	}

	g_pFS->UnLoad(tmpFile, tmpData);

	// ziskani parametru
	s_shaders[volny].hArray1 = (__w64 unsigned int)s_shaders[volny].effect->GetParameterBySemantic(NULL, "Array1");
	s_shaders[volny].hArray2 = (__w64 unsigned int)s_shaders[volny].effect->GetParameterBySemantic(NULL, "Array2");
	s_shaders[volny].hTexture = (__w64 unsigned int)s_shaders[volny].effect->GetParameterBySemantic(NULL, "Texture");

	// nastaveni techniky
	DWORD dwVertVer, dwPixVer;
	g_pMaterialManager->GetShaderVersion (dwVertVer, dwPixVer);
	STAT(STAT_SHADER_SET_TECHNIQUE, 1);
	if (D3DSHADER_VERSION_MAJOR (dwVertVer) >= 3)
		s_shaders[volny].effect->SetTechnique ("t3");
	else
		s_shaders[volny].effect->SetTechnique ("t2");

	return volny;
}

bool CP3DSpriteBunch::RemoveShader(const char* shaderName)
{
	for(int i=0; i<MAX_SHADERS; i++)
	{
		// je v cache?
		if (!strcmpi(s_shaders[i].name, shaderName))
		{
			if (s_shaders[i].refCount==0) 
			{
				CON(MSG_CON_DEBUG, "Shader already removed! ;)");
				return true; // jiz uvolnen
			}
			s_shaders[i].refCount--;
			if (s_shaders[i].refCount==0) s_shaders[i].effect->Release();
			return true;
		}
	}
	return false;
}

bool CP3DSpriteBunch::RemoveShader(SHADER shaderHandle)
{
	if (shaderHandle==-1) 
	{
		CON(MSG_CON_ERR, "Invalid shader handle!");
		return false;
	}

	if (s_shaders[shaderHandle].refCount==0) 
	{
		CON(MSG_CON_DEBUG, "Shader already removed! ;)");
		return true; // jiz uvolnen
	}

	s_shaders[shaderHandle].refCount--;
	if (s_shaders[shaderHandle].refCount==0) s_shaders[shaderHandle].effect->Release();
	return true;
}

bool CP3DSpriteBunch::Init()
{
	void	*pData;
	P3DPOS1TEX	*pVertexData;
	DWORD	i;

	VertexPosTex quad_buf[4] = {
								{P3DXPoint3D(-0.5f, 1.0f, 0.0f), P3DXPoint2D(0.0f, 0.0f)},
								{P3DXPoint3D(0.5f, 1.0f, 0.0f ), P3DXPoint2D(1.0f, 0.0f)},
								{P3DXPoint3D(-0.5f, 0.0f, 0.0f), P3DXPoint2D(0.0f, 1.0f)},
								{P3DXPoint3D(0.5f, 0.0f, 0.0f ), P3DXPoint2D(1.0f, 1.0f)}
							   };

	P3DVertexElement ve[3];
	ve[0] = P3DVertexElement(P3DVD_FLOAT3, P3DVU_POSITION);
	ve[1] = P3DVertexElement(P3DVD_FLOAT2, P3DVU_TEXCOORD);
	ve[2] = P3DVE_END();

	if (FAILED(s_vertexBuffer.CreateVB(4 * NUM_SPRITES, ve, sizeof(VertexPosTex))))
	{
		CON(MSG_CON_ERR, "Can't create vertex buffer for sprite bunch!");
		return false;
	}
	if (SUCCEEDED (s_vertexBuffer.Lock(&pData)))
	{
		VertexPosTex *pVertexData2 = (VertexPosTex*)pData;
		for (i=0; i<NUM_SPRITES; i++)
		{
			pVertexData2[i*4] = quad_buf[0];
			pVertexData2[i*4].pos.z = float(i);	// z sa pouziva ako index pre shader instancing
			pVertexData2[i*4+1] = quad_buf[1];
			pVertexData2[i*4+1].pos.z = float(i);
			pVertexData2[i*4+2] = quad_buf[2];
			pVertexData2[i*4+2].pos.z = float(i);
			pVertexData2[i*4+3] = quad_buf[3];
			pVertexData2[i*4+3].pos.z = float(i);
		}
		s_vertexBuffer.UnLock();
	}

	// index buffer - grass
	if (FAILED (s_indexBuffer.CreateIB((sizeof(WORD) * 6 * NUM_SPRITES))))
	{
		CON(MSG_CON_ERR, "Can't create index buffer for sprite bunch!");
		return false;
	}
	if (SUCCEEDED (s_indexBuffer.Lock(&pData)))
	{
		unsigned short *pIndexData = (unsigned short*)pData;
		for (i=0; i<NUM_SPRITES; i++)
		{
			pIndexData[i*6] = (unsigned short) i*4;
			pIndexData[i*6+1] = (unsigned short) i*4 + 1;
			pIndexData[i*6+2] = (unsigned short) i*4 + 2;
			pIndexData[i*6+3] = (unsigned short) i*4 + 2;
			pIndexData[i*6+4] = (unsigned short) i*4 + 1;
			pIndexData[i*6+5] = (unsigned short) i*4 + 3;
		}
		s_indexBuffer.UnLock();
	}

	return true;
}

void CP3DSpriteBunch::Render(int numSprites, IP3DTexture *pTex, SHADER shaderHandle, P3DVector4D *vArray1, P3DVector4D *vArray2)
{
	// FIXME: debug !!!
	return;
	// FIXME: debug !!!
	if (shaderHandle==-1) 
	{
		CON(MSG_CON_ERR, "Invalid shader handle!");
		return;
	}

	// nastav parametry
	if (pTex) s_shaders[shaderHandle].effect->SetTexture((D3DXHANDLE)s_shaders[shaderHandle].hTexture, (LPDIRECT3DBASETEXTURE9)pTex->GetD3DTexture());
	if (vArray1) s_shaders[shaderHandle].effect->SetVectorArray((D3DXHANDLE)s_shaders[shaderHandle].hArray1, (const D3DXVECTOR4 *)vArray1, NUM_SPRITES);
	if (vArray2) s_shaders[shaderHandle].effect->SetVectorArray((D3DXHANDLE)s_shaders[shaderHandle].hArray2, (const D3DXVECTOR4 *)vArray2, NUM_SPRITES);

	// vykresli
	s_shaders[shaderHandle].effect->Begin(NULL, 0);
	s_shaders[shaderHandle].effect->BeginPass(0);
	 s_vertexBuffer.Use();
	 s_indexBuffer.Use();
	 s_vertexBuffer.RenderIndexed(P3DPT_TRIANGLELIST, 0, 0, numSprites * 4, 0, numSprites * 2);
	s_shaders[shaderHandle].effect->EndPass();
	s_shaders[shaderHandle].effect->End();
}

bool CP3DSpriteBunch::FindParams(FXParamBatch* paramsArray, int numParams)
{
	CON(MSG_CON_ERR, "NOT IMPLEMENTED!"); //TODO
	return false;
}

bool CP3DSpriteBunch::SetParams(FXParamBatch* paramsArray, int numParams)
{
	CON(MSG_CON_ERR, "NOT IMPLEMENTED!"); //TODO
	return false;
}