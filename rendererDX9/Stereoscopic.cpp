//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Stereoscopic manager (renderer)
//-----------------------------------------------------------------------------

#include "Stereoscopic.h"

REGISTER_SINGLE_CLASS (CP3DStereoscopic, IP3DRENDERER_STEREOSCOPIC); // zaregistruj tuto tøídu

static bool	g_bSTEREOSCOPICStarted;


//---------------------------------
CP3DStereoscopic::CP3DStereoscopic()
{
	m_pScreenQuadVertexDecl = NULL;
	m_pScreenQuadVertexBuffer = NULL;
	m_pPreviousTarget = NULL;

	m_pTexLeft=NULL;
	m_pSurfLeft=NULL;
	m_pTexRight=NULL;
	m_pSurfRight=NULL;
}

//---------------------------------
CP3DStereoscopic::~CP3DStereoscopic()
{
	SAFE_RELEASE (m_pScreenQuadVertexDecl)
	SAFE_RELEASE (m_pScreenQuadVertexBuffer)
	
	SAFE_RELEASE (m_pTexLeft);
	SAFE_RELEASE (m_pSurfLeft);

	SAFE_RELEASE (m_pTexRight);
	SAFE_RELEASE (m_pSurfRight);
}

//---------------------------------
bool CP3DStereoscopic::Init ()
{
	if ((g_pD3DDevice == NULL) || (g_pMaterialManager == NULL) || (g_pXML == NULL)) return false;

	// screen quad vertex declarations
	D3DVERTEXELEMENT9	vertex_elements[3] = {0};
	vertex_elements[0].Offset = 0;
	vertex_elements[0].Type = D3DDECLTYPE_FLOAT4;
	vertex_elements[0].Usage = D3DDECLUSAGE_POSITIONT;
	vertex_elements[1].Offset = sizeof (float) * 4;
	vertex_elements[1].Type = D3DDECLTYPE_FLOAT2;
	vertex_elements[1].Usage = D3DDECLUSAGE_TEXCOORD;
	// ukoncovaci element
	vertex_elements[2].Stream = 0xFF;
	vertex_elements[2].Type = D3DDECLTYPE_UNUSED;

	if (FAILED (g_pD3DDevice->CreateVertexDeclaration (vertex_elements, &m_pScreenQuadVertexDecl)))
	{
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't create vertex declaration!");
		return false;
	}

	if (FAILED (g_pD3DDevice->CreateVertexBuffer (sizeof(STEREOSCOPIC_VERTEX_FORMAT) * 4, 0, STEREOSCOPIC_FVF, \
												D3DPOOL_MANAGED, &m_pScreenQuadVertexBuffer, NULL)))
	{
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't create vertex buffer!");
		return false;
	}

	// screen quad vertex buffer, udelam dva quady
	STEREOSCOPIC_VERTEX_FORMAT *pData;
	if (FAILED (m_pScreenQuadVertexBuffer->Lock (0, 0, (void**)&pData, 0)))
	{
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't lock vertex buffer!");
		return false;
	}

		// vastav vb

		int i=0;
		pData[i*4] =		ScreenQuadSTEREOSCOPIC[0];
		pData[i*4 + 1] =	ScreenQuadSTEREOSCOPIC[1];
		pData[i*4 + 2] =	ScreenQuadSTEREOSCOPIC[2];
		pData[i*4 + 3] =	ScreenQuadSTEREOSCOPIC[3];

		pData[i*4 + 0].x = 0.0f;
		pData[i*4 + 0].y = 0.0f;
		pData[i*4 + 1].x = (float)g_pEngSet.Width;
		pData[i*4 + 1].y = 0.0f;
		pData[i*4 + 2].x = 0.0f;
		pData[i*4 + 2].y = (float)g_pEngSet.Height;
		pData[i*4 + 3].x = (float)g_pEngSet.Width;
		pData[i*4 + 3].y = (float)g_pEngSet.Height;

	m_pScreenQuadVertexBuffer->Unlock ();



	// vytvor render targets
	
	// LEVE
	STAT(STAT_TEXTURE_ALL_TEXELS, g_pEngSet.Width*g_pEngSet.Height);
	if (FAILED (D3DXCreateTexture(g_pD3DDevice, g_pEngSet.Width, g_pEngSet.Height, 1, D3DUSAGE_RENDERTARGET, \
						D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, \
						&m_pTexLeft)))
	{ 
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't create left eye texture!");
		return false;
	}

	if (FAILED (m_pTexLeft->GetSurfaceLevel (0, &m_pSurfLeft)))
	{ 
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't get first surface!");
		return false;
	}
		
	// PRAVE
		
	if (FAILED (D3DXCreateTexture(g_pD3DDevice, g_pEngSet.Width, g_pEngSet.Height, 1, D3DUSAGE_RENDERTARGET, \
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, \
		&m_pTexRight)))
	{ 
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't create right eye texture!");
		return false;
	}

	if (FAILED (m_pTexRight->GetSurfaceLevel (0, &m_pSurfRight)))
	{ 
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't get second surface!");
		return false;
	}
	
	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	m_dwMat = g_pMaterialManager->LoadMaterial("materials/spec_stereo_anaglyph.7mt");
	if (m_dwMat==0xFFFFFFFF)
	{
		CON(MSG_ERR_FATAL, "Stereoscopic: Can't load spec_stereo_anaglyph.7mt!");
		return false;
	}

	return true;
}

//---------------------------------
void CP3DStereoscopic::Begin (int eye)
{
		LPDIRECT3DSURFACE9 pSurface;
		pSurface = eye ? m_pSurfLeft : m_pSurfRight;

		g_pD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1, 0);

		if (eye==0)
		{
			if (FAILED (g_pD3DDevice->GetRenderTarget (0, &m_pPreviousTarget)))
			{
				return;
			}
		}

		if ((pSurface == NULL) || \
		(FAILED (g_pD3DDevice->SetRenderTarget (0, pSurface))))
		{
			return;
		}
			
		g_bSTEREOSCOPICStarted = true;
}

//---------------------------------
void CP3DStereoscopic::SetShaderParameters()
{
	MATERIAL_PARAMETER paramTable[SEM_NUM];
	WORD wAllParams, wParamsToSet;

	g_pMaterialManager->QueryParameters (m_dwMat, false, paramTable, wAllParams, wParamsToSet);

	for (DWORD i=0; i<wParamsToSet; i++)
	{
		switch (paramTable[i].SemanticID)
		{
		case SEM_TextureLayer0:
			if (m_pTexLeft) 
			{
				STAT(STAT_SHADER_TEXTURES, 1);
				paramTable[i].pValue = &m_pTexLeft;
			}
			break;
		case SEM_TextureLayer1:
			m_pTexLeft=m_pTexLeft;
			if (m_pTexRight)
			{
				STAT(STAT_SHADER_TEXTURES, 1);
				paramTable[i].pValue = &m_pTexRight;
			}
			break;
		}
	}

	g_pMaterialManager->SetParameters (m_dwMat, paramTable, wAllParams);
}

//---------------------------------
void CP3DStereoscopic::End (int eye)
{
	static bool bSetDefaultViewportPixelSize = false;

	if (!g_bSTEREOSCOPICStarted)
		{g_bSTEREOSCOPICStarted = false; return;}
		
	if (eye==1) // prave oko uz bylo, vykresli vysledek
	{
		g_pMaterialManager->TurnOffShaders();
	
		STAT(STAT_SET_RENDER_STATE, 3);
		g_pD3DDevice->SetRenderState (D3DRS_LIGHTING, FALSE);
		g_pD3DDevice->SetRenderState (D3DRS_ZENABLE, D3DZB_FALSE);
		g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);

		g_pD3DDevice->SetVertexDeclaration (m_pScreenQuadVertexDecl);
		g_pD3DDevice->SetStreamSource (0, m_pScreenQuadVertexBuffer, 0, sizeof (STEREOSCOPIC_VERTEX_FORMAT));

		//////////////////////////////////////////////////////////////////////////

		// KONEC
		if (m_pPreviousTarget)
		{
			g_pD3DDevice->SetRenderTarget (0, m_pPreviousTarget);
			m_pPreviousTarget->Release(); m_pPreviousTarget = NULL;
		}

		//////////////////////////////////////////////////////////////////////////

		SetShaderParameters();
		g_pMaterialManager->SetMaterial(m_dwMat);
		
		// vykresli
		do{
		//g_pD3DDevice->SetTexture(0, m_pTexRight);
			STAT(STAT_DRAW_CALLS, 1);
			g_pD3DDevice->DrawPrimitive (D3DPT_TRIANGLESTRIP, 0, 2);
		}while(g_pMaterialManager->NextPass(m_dwMat));

		g_bSTEREOSCOPICStarted = false;

		STAT(STAT_SET_RENDER_STATE, 2);
		g_pD3DDevice->SetRenderState (D3DRS_ZENABLE, D3DZB_TRUE);
		g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
		//g_pD3DDevice->SetRenderState (D3DRS_LIGHTING, TRUE);
	
	}
}