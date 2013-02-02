//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Sky rendering class definiton
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "ip3dsky.h"


#define SKY_HALFSIZE			20000
#define CLOUDS_HALFSIZE		39000
#define CLOUDS_ALTITUDE		5000


#define SKY_FVF	(D3DFVF_XYZ | D3DFVF_TEX1)

struct SKY_VERTEX
{
	float x, y, z;
	float tu, tv;
};


class CP3DSky : public IP3DSky
{
public:
	CP3DSky();
	~CP3DSky();
	bool Init();
	bool CreateSkybox (const char* strSkyTexturePath, const char* strCloudsTexturePath, float fSkyAngle);
	void Render (bool drawSky, bool drawClouds);
private:
	bool			bLoaded;
	LPDIRECT3DVERTEXBUFFER9	m_SkyVB;
	LPDIRECT3DVERTEXDECLARATION9	m_pSkyBoxDeclaration;

	LPD3DXEFFECT				m_pSkyShader;
	D3DXHANDLE					m_pSkyParams[4];
	D3DXHANDLE					m_pTechniques[2];
	LPDIRECT3DCUBETEXTURE9	m_pSkyTex;
// 	LPD3DXEFFECT				m_pCloudShader;
// 	D3DXHANDLE					m_pCloudParams[3];
	LPDIRECT3DTEXTURE9		m_pCloudsTex;
	float					m_fSkyAngle;
};