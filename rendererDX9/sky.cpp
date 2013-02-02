//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Sky rendering
//-----------------------------------------------------------------------------
#include "sky.h"

REGISTER_SINGLE_CLASS(CP3DSky, IP3DRENDERER_SKY); // zaregistruj tuto tøídu


CP3DSky::CP3DSky()
{
	bLoaded = false;
	m_SkyVB = NULL; m_pSkyBoxDeclaration = NULL;
	m_pSkyShader = NULL;
	m_pTechniques[0] = NULL; m_pTechniques[1] = NULL;
	m_pSkyParams[0] = NULL; m_pSkyParams[1] = NULL; m_pSkyParams[2] = NULL; m_pSkyParams[3] = NULL;
	m_pSkyTex = NULL; m_pCloudsTex = NULL;
	m_fSkyAngle = 0.0f;
}

CP3DSky::~CP3DSky()
{
	if (m_pSkyTex) g_TextureLoader.UnloadTexture (m_pSkyTex);
	if (m_pCloudsTex) g_TextureLoader.UnloadTexture (m_pCloudsTex);
	SAFE_RELEASE (m_pSkyBoxDeclaration)
	SAFE_RELEASE (m_pSkyShader)
	SAFE_RELEASE (m_SkyVB)
}

bool CP3DSky::Init ()
{
	if (g_pD3DDevice == NULL) return false;

	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);

	// VERTEX BUFFER CREATION
	SKY_VERTEX pVertices[40];
	float fBufU, fBufV;
	DWORD i;

	// nastavenie texturových koordinátov
	for (i =0; i<6; i++)
	{
		switch (i)
		{
		case 0: fBufU = 0; fBufV = 1; break;
		case 1:
		case 4: fBufU = 0; fBufV = 0; break;
		case 2:
		case 3: fBufU = 1; fBufV = 1; break;
		case 5: fBufU = 1; fBufV = 0;
		}
		pVertices[i].tu =  pVertices[i+6].tu = pVertices[i+12].tu = \
			pVertices[i+18].tu = pVertices[i+24].tu = pVertices[i+30].tu = fBufU;
		pVertices[i].tv =  pVertices[i+6].tv = pVertices[i+12].tv = \
			pVertices[i+18].tv = pVertices[i+24].tv = pVertices[i+30].tv = fBufV;
	}
	pVertices[36].tu = pVertices[37].tu = 0;
	pVertices[38].tu = pVertices[39].tu = 1;
	pVertices[36].tv = pVertices[38].tv = 1;
	pVertices[37].tv = pVertices[39].tv = 0;


	//Top
	for (i=0; i<6; i++)
		pVertices[i].y = SKY_HALFSIZE;
	pVertices[0].x = pVertices[1].x = pVertices[4].x = -SKY_HALFSIZE;
	pVertices[2].x = pVertices[3].x = pVertices[5].x = SKY_HALFSIZE;
	pVertices[0].z = pVertices[2].z = pVertices[3].z = SKY_HALFSIZE;
	pVertices[1].z = pVertices[4].z = pVertices[5].z = -SKY_HALFSIZE;
	//Down
	for (i=6; i<12; i++)
		pVertices[i].y = -SKY_HALFSIZE;
	pVertices[6].x = pVertices[7].x = pVertices[10].x = -SKY_HALFSIZE;
	pVertices[8].x = pVertices[9].x = pVertices[11].x = SKY_HALFSIZE;
	pVertices[6].z = pVertices[8].z = pVertices[9].z = -SKY_HALFSIZE;
	pVertices[7].z = pVertices[10].z = pVertices[11].z = SKY_HALFSIZE;
	//Front
	for (i=12; i<18; i++)
		pVertices[i].z = SKY_HALFSIZE;
	pVertices[12].x = pVertices[13].x = pVertices[16].x = -SKY_HALFSIZE;
	pVertices[14].x = pVertices[15].x = pVertices[17].x = SKY_HALFSIZE;
	pVertices[12].y = pVertices[14].y = pVertices[15].y = -SKY_HALFSIZE;
	pVertices[13].y = pVertices[16].y = pVertices[17].y = SKY_HALFSIZE;
	//Back
	for (i=18; i<24; i++)
		pVertices[i].z = -SKY_HALFSIZE;
	pVertices[18].x = pVertices[19].x = pVertices[22].x = SKY_HALFSIZE;
	pVertices[20].x = pVertices[21].x = pVertices[23].x = -SKY_HALFSIZE;
	pVertices[18].y = pVertices[20].y = pVertices[21].y = -SKY_HALFSIZE;
	pVertices[19].y = pVertices[22].y = pVertices[23].y = SKY_HALFSIZE;
	//Left
	for (i=24; i<30; i++)
		pVertices[i].x = -SKY_HALFSIZE;
	pVertices[24].z = pVertices[25].z = pVertices[28].z = -SKY_HALFSIZE;
	pVertices[26].z = pVertices[27].z = pVertices[29].z = SKY_HALFSIZE;
	pVertices[24].y = pVertices[26].y = pVertices[27].y = -SKY_HALFSIZE;
	pVertices[25].y = pVertices[28].y = pVertices[29].y = SKY_HALFSIZE;
	//Right
	for (i=30; i<36; i++)
		pVertices[i].x = SKY_HALFSIZE;
	pVertices[30].z = pVertices[31].z = pVertices[34].z = SKY_HALFSIZE;
	pVertices[32].z = pVertices[33].z = pVertices[35].z = -SKY_HALFSIZE;
	pVertices[30].y = pVertices[32].y = pVertices[33].y = -SKY_HALFSIZE;
	pVertices[31].y = pVertices[34].y = pVertices[35].y = SKY_HALFSIZE;
	//Clouds
	for (i=36; i<40; i++)
		pVertices[i].y = CLOUDS_ALTITUDE;
	pVertices[36].x = pVertices[37].x = -CLOUDS_HALFSIZE;
	pVertices[38].x = pVertices[39].x = CLOUDS_HALFSIZE;
	pVertices[36].z = pVertices[38].z = CLOUDS_HALFSIZE;
	pVertices[37].z = pVertices[39].z = -CLOUDS_HALFSIZE;

	// vytvorenie vertex bufru
	if (FAILED (g_pD3DDevice->CreateVertexBuffer (sizeof(SKY_VERTEX) * 40, 0, SKY_FVF, D3DPOOL_MANAGED, &m_SkyVB, NULL)))
		return false;

	void *pVertexBuf;
	if (FAILED (m_SkyVB->Lock (0, 0, &pVertexBuf, 0)))
	{SAFE_RELEASE (m_SkyVB); CON(MSG_ERR_FATAL, "Sky: Can't lock vertex buffer!");  return false;}
	memcpy (pVertexBuf, pVertices, sizeof(SKY_VERTEX) * 40);
	m_SkyVB->Unlock();

	//////////////////////////////////////////////////////////////////////////
	// nacitanie efektu
	if (g_pMaterialManager && g_pFS)
	{
		FSFILE	*pFile;
		BYTE		*pData;
		DWORD		dwFileSize;
		HRESULT	hr;

		// nacitanie skybox efektu
		if ((pFile = g_pFS->Load ("shaders\\spec_skybox.fx", pData, dwFileSize)) == NULL)
		{
			CON(MSG_ERR_FATAL, "Sky: Skybox shader (shaders\\spec_skybox.fx) doesn't exist!");
			return false;
		}
		hr = D3DXCreateEffect (g_pD3DDevice, pData, dwFileSize, NULL, NULL, \
				D3DXFX_NOT_CLONEABLE, (LPD3DXEFFECTPOOL)g_pMaterialManager->GetPoolHandle (), \
				&m_pSkyShader, NULL);
		g_pFS->UnLoad (pFile, pData);
		if (FAILED (hr))
		{
			CON(MSG_ERR_FATAL, "Sky: Invalid skybox shader (shaders\\spec_skybox.fx)!");
			return false;
		}
		m_pSkyParams[0] = m_pSkyShader->GetParameterBySemantic (NULL, "World");
		m_pSkyParams[1] = m_pSkyShader->GetParameterBySemantic (NULL, "TextureCubeMap");
		m_pSkyParams[2] = m_pSkyShader->GetParameterBySemantic (NULL, "Time");
		m_pSkyParams[3] = m_pSkyShader->GetParameterBySemantic (NULL, "Texture");

		m_pTechniques[0] = m_pSkyShader->GetTechniqueByName ("spec_skybox");
		m_pTechniques[1] = m_pSkyShader->GetTechniqueByName ("spec_clouds");

		// skybox vertex declaration
		D3DVERTEXELEMENT9 sky_declaration[] = 
		{
			{0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0},
			{0, 12, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	0},
			D3DDECL_END()
		};
		if (FAILED (g_pD3DDevice->CreateVertexDeclaration (sky_declaration, &m_pSkyBoxDeclaration)))
		{
			CON(MSG_ERR_FATAL, "Sky: Can't create vertex declaration!!");
			return false;
		}
	}

	return true;
}

bool CP3DSky::CreateSkybox(const char* strSkyTexturePath, const char* strCloudsTexturePath, float fSkyAngle)
{
	if (strSkyTexturePath == NULL) return false;
	// uvolnenie predoslych zdrojov
	if (m_pSkyTex) g_TextureLoader.UnloadTexture (m_pSkyTex);
	m_pSkyTex = NULL;
	if (m_pCloudsTex) g_TextureLoader.UnloadTexture (m_pCloudsTex);
	m_pCloudsTex = NULL;
	bLoaded = false;

	// nacitanie textur
	m_pSkyTex = g_TextureLoader.CreateCubeTexture (strSkyTexturePath);
	if (strCloudsTexturePath)				// clouds
		m_pCloudsTex = g_TextureLoader.CreateTexture (strCloudsTexturePath);

	m_fSkyAngle = fSkyAngle;
	bLoaded = true;
	return true;
}

void CP3DSky::Render (bool drawSky, bool drawClouds)
{
	if (!bLoaded) return;

	if (drawSky)
	{
		if (g_pMaterialManager) g_pMaterialManager->TurnOffShaders ();

		g_pD3DDevice->SetVertexDeclaration (m_pSkyBoxDeclaration);
		g_pD3DDevice->SetStreamSource (0, m_SkyVB, 0, sizeof(SKY_VERTEX));

		// world transform
		P3DXMatrix	matTransform;
		matTransform.SetIdentityMatrix ();
		matTransform.RotateY(m_fSkyAngle);
		matTransform._41 = g_cameraPosition.x;
		matTransform._42 = g_cameraPosition.y;
		matTransform._43 = g_cameraPosition.z;

		// skybox
		STAT(STAT_SHADER_SET_TECHNIQUE, 1);
		m_pSkyShader->SetTechnique (m_pTechniques[0]);
		m_pSkyShader->SetMatrix(m_pSkyParams[0], (D3DXMATRIX*)&matTransform);
		m_pSkyShader->SetTexture (m_pSkyParams[1], m_pSkyTex);
		UINT passes;
		m_pSkyShader->Begin (&passes, 0);
		STAT(STAT_SHADER_BEGIN_PASS, 1);
		m_pSkyShader->BeginPass (0);
		STAT(STAT_DRAW_CALLS, 1);
		STAT(STAT_DRAW_VERTS, 12);
		g_pD3DDevice->DrawPrimitive (D3DPT_TRIANGLELIST, 0, 12);
		m_pSkyShader->EndPass ();
		m_pSkyShader->End ();

		// clouds
		if (drawClouds)
		{
			STAT(STAT_SHADER_SET_TECHNIQUE, 1);
			m_pSkyShader->SetTechnique (m_pTechniques[1]);
			float fTime = ((float)g_pTimer->GetTickCount_ms()) * 0.001f;
			m_pSkyShader->SetFloat (m_pSkyParams[2], fTime);
			m_pSkyShader->SetTexture (m_pSkyParams[3], m_pCloudsTex);

			m_pSkyShader->Begin (&passes, 0);
			STAT(STAT_SHADER_BEGIN_PASS, 1);
			m_pSkyShader->BeginPass (0);
			STAT(STAT_DRAW_CALLS, 1);
			STAT(STAT_DRAW_VERTS, 2);
			g_pD3DDevice->DrawPrimitive (D3DPT_TRIANGLESTRIP, 36, 2);
			m_pSkyShader->EndPass ();
			m_pSkyShader->End ();
		}
	}
}
