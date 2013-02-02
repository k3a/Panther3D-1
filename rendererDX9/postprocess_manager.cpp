//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Postprocess manager class definition
//-----------------------------------------------------------------------------

#include "postprocess_manager.h"

REGISTER_SINGLE_CLASS (CP3DPostprocessManager, IP3DRENDERER_POSTPROCESSMANAGER); // zaregistruj tuto tøídu

static ConVar CVr_postprocess("r_postprocess", "1", CVAR_NOT_STRING | CVAR_ARCHIVE, "Aktivovat post_processing");

bool	g_bPostProcessingStarted;


//---------------------------------
CP3DPostprocessManager::CP3DPostprocessManager()
{
	m_pScreenQuadVertexDecl = NULL;
	m_pScreenQuadVertexBuffer = NULL;
	m_pPreviousTarget = NULL;
	m_dwEffectsCount = 0;
	m_PPEffectsXml = NULL;
}

//---------------------------------
CP3DPostprocessManager::~CP3DPostprocessManager()
{
	SAFE_RELEASE (m_pScreenQuadVertexDecl)
	SAFE_RELEASE (m_pScreenQuadVertexBuffer)
	FlushChain();
	if (g_pXML && m_PPEffectsXml)
		g_pXML->Free (m_PPEffectsXml);
}

//---------------------------------
void CP3DPostprocessManager::ClearCache ()
{
	for (DWORD i=0; i<CACHE_TABLE_LIMIT; i++)
		m_CacheTable[i].Clear();
}

//---------------------------------
void CP3DPostprocessManager::FlushChain ()
{
	for (DWORD i=0; i<MAX_POST_EFFECTS; i++)
		m_ProcessingChain[i].Flush ();

	m_dwEffectsCount = 0;
}

//---------------------------------
bool CP3DPostprocessManager::Init ()
{
	if ((g_pD3DDevice == NULL) || (g_pMaterialManager == NULL) || (g_pXML == NULL)) return false;

	// screen quad vertex declarations
	D3DVERTEXELEMENT9	vertex_elements[3] = {0};
	vertex_elements[0].Offset = 0;
	vertex_elements[0].Type = D3DDECLTYPE_FLOAT3;
	vertex_elements[0].Usage = D3DDECLUSAGE_POSITION;
	vertex_elements[1].Offset = sizeof (float) * 3;
	vertex_elements[1].Type = D3DDECLTYPE_FLOAT2;
	vertex_elements[1].Usage = D3DDECLUSAGE_TEXCOORD;
	// ukoncovaci element
	vertex_elements[2].Stream = 0xFF;
	vertex_elements[2].Type = D3DDECLTYPE_UNUSED;

	if (FAILED (g_pD3DDevice->CreateVertexDeclaration (vertex_elements, &m_pScreenQuadVertexDecl)))
	{
		CON(MSG_ERR_FATAL, "PostprocessManager: Can't create vertex declaration!");
		return false;
	}

	if (FAILED (g_pD3DDevice->CreateVertexBuffer (sizeof(POSTPROCESS_VERTEX_FORMAT) * 20, 0, POSTPROCESS_FVF, \
												D3DPOOL_MANAGED, &m_pScreenQuadVertexBuffer, NULL)))
	{
		CON(MSG_ERR_FATAL, "PostprocessManager: Can't create vertex buffer!");
		return false;
	}

	// screen quad vertex buffer, vytvori sa 5 rozmerov... 1, 1/2, 1/4, 1/8, 1/16
	POSTPROCESS_VERTEX_FORMAT *pData;
	if (FAILED (m_pScreenQuadVertexBuffer->Lock (0, 0, (void**)&pData, 0)))
	{
		CON(MSG_ERR_FATAL, "PostprocessManager: Can't lock vertex buffer!");
		return false;
	}

	for (int i=0; i<5; i++)
	{
		// posun mapovania screen quadu o 0.5 pixela pre presne zobrazenie pixelov na texely
		float fPixelShiftX = (0.5f * pow(2.0f,i)) / g_pEngSet.Width;
		float fPixelShiftY = (0.5f * pow(2.0f,i)) / g_pEngSet.Height;

		POSTPROCESS_VERTEX_FORMAT newScreenQuad[4];
		for (DWORD j=0; j<4; j++)
		{
			newScreenQuad[j] = ScreenQuad[j];
			newScreenQuad[j].u += fPixelShiftX;
			newScreenQuad[j].v += fPixelShiftY;
		}

		pData[i*4] =		newScreenQuad[0];
		pData[i*4 + 1] =	newScreenQuad[1];
		pData[i*4 + 2] =	newScreenQuad[2];
		pData[i*4 + 3] =	newScreenQuad[3];
	}
	m_pScreenQuadVertexBuffer->Unlock ();

	// nacitanie postprocess_effects.xml
	m_PPEffectsXml = g_pXML->Parse_file ("postprocess_effects.xml");
	if (m_PPEffectsXml == NULL) 
	{
		CON(MSG_ERR_FATAL, "PostprocessManager: Failed to load postprocess_effects.xml!");
		return false;
	}

	return true;
}

//---------------------------------
void CP3DPostprocessManager::Begin ()
{
	if (CVr_postprocess.GetBool() && (!CVr_wireframe.GetBool()) && (!g_pConsole->IsShown()))
	{
		if ((m_ProcessingChain[0].ChainLink[0].dwEffect == 0xFFFFFFFF) || (m_ProcessingChain[0].ChainLink[0].pSurface == NULL) || \
				(FAILED (g_pD3DDevice->GetRenderTarget (0, &m_pPreviousTarget))) || \
				(FAILED (g_pD3DDevice->SetRenderTarget (0, m_ProcessingChain[0].ChainLink[0].pSurface))))
			{
				FlushChain ();
				//CVr_postprocess.SetValue (0); // KEX 08052007: Tohle ne. Nekdy je treba mit zapny postprocess a zane postprocessy aktivni.
				return;
			}
		// nastavit pixel_size
		float	PixelSize[2];
		float	fTextureSize = 1.0f / pow (2.0f, m_ProcessingChain[0].ChainLink[0].intTextureSizeExp);
		PixelSize[0] = 1.0f / (float(g_pEngSet.Width) * fTextureSize);
		PixelSize[1] = 1.0f / (float(g_pEngSet.Height) * fTextureSize);
		g_pMaterialManager->OnViewportSizeChange (PixelSize);

		g_bPostProcessingStarted = true;
	}
}

//---------------------------------
void CP3DPostprocessManager::End ()
{
	float	PixelSize[2];
	unsigned int uiStartVertex = 0;

	if (g_bPostProcessingStarted && (!CVr_postprocess.GetBool ()) && (m_pPreviousTarget != NULL))
	{
		g_pD3DDevice->SetRenderTarget (0, m_pPreviousTarget);
		m_pPreviousTarget->Release(); m_pPreviousTarget = NULL;
		g_bPostProcessingStarted = false;

		PixelSize[0] = 1.0f / float(g_pEngSet.Width);
		PixelSize[1] = 1.0f / float(g_pEngSet.Height);
		g_pMaterialManager->OnViewportSizeChange (PixelSize);
		return;
	}
	if ((!g_bPostProcessingStarted)  ||  (!CVr_postprocess.GetBool ()) || (m_dwEffectsCount == 0))
		{g_bPostProcessingStarted = false; return;}

	STAT(STAT_SET_RENDER_STATE, 3);
	g_pD3DDevice->SetRenderState (D3DRS_LIGHTING, FALSE);
	g_pD3DDevice->SetRenderState (D3DRS_ZENABLE, D3DZB_FALSE);
	g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);

	g_pD3DDevice->SetVertexDeclaration (m_pScreenQuadVertexDecl);
	g_pD3DDevice->SetStreamSource (0, m_pScreenQuadVertexBuffer, 0, sizeof (POSTPROCESS_VERTEX_FORMAT));

	//////////////////////////////////////////////////////////////////////////
	for (DWORD i=0; i<m_dwEffectsCount; i++)
		for (DWORD j=0; j<m_ProcessingChain[i].wLinksCount; j++)
		{
			if ((m_ProcessingChain[i].ChainLink[j].dwEffect == 0xFFFFFFFF) || \
						(m_ProcessingChain[i].ChainLink[j].pSurface == NULL))
				continue;

			// ak je to posledny chainlink_efekt, tak nastavit target spat na backbuffer
			if ((i == m_dwEffectsCount - 1) && (j == m_ProcessingChain[i].wLinksCount - 1))
			{
				if (m_pPreviousTarget)
				{
					g_pD3DDevice->SetRenderTarget (0, m_pPreviousTarget);
					m_pPreviousTarget->Release(); m_pPreviousTarget = NULL;
				}
				uiStartVertex = 0;
			}
			// ak nie je posledny chainlink_efekt, tak sa ako render_target
			// nastavi surface nasledujuceho chainlink_efektu
			else
			{
				DWORD	IdxI, IdxJ;
				if (j == m_ProcessingChain[i].wLinksCount - 1)
					{IdxI = i + 1; IdxJ = 0;}
				else
					{IdxI = i; IdxJ = j + 1;}

				g_pD3DDevice->SetRenderTarget (0, m_ProcessingChain[IdxI].ChainLink[IdxJ].pSurface);
				uiStartVertex = m_ProcessingChain[IdxI].ChainLink[IdxJ].intTextureSizeExp;
			}

			// nastavit pixel_size
			float	fTextureSize = 1.0f / pow (2.0f, m_ProcessingChain[i].ChainLink[j].intTextureSizeExp);
			PixelSize[0] = 1.0f / (float(g_pEngSet.Width) * fTextureSize);
			PixelSize[1] = 1.0f / (float(g_pEngSet.Height) * fTextureSize);
			g_pMaterialManager->OnViewportSizeChange (PixelSize);

			//////////////////////////////////////////////////////////////////////////
			// rendering

			SetShaderParameters (i, j);
			g_pMaterialManager->SetMaterial (m_ProcessingChain[i].ChainLink[j].dwEffect);
			do{
				STAT(STAT_DRAW_CALLS, 1);
				STAT(STAT_DRAW_VERTS, 2);
				g_pD3DDevice->DrawPrimitive (D3DPT_TRIANGLESTRIP, uiStartVertex*4, 2);
			}while(g_pMaterialManager->NextPass (m_ProcessingChain[i].ChainLink[j].dwEffect));
		}

	g_bPostProcessingStarted = false;

	g_pMaterialManager->TurnOffShaders ();
	// nastavit pixel_size spat na rozmery back bufru
	PixelSize[0] = 1.0f / float(g_pEngSet.Width);
	PixelSize[1] = 1.0f / float(g_pEngSet.Height);
	g_pMaterialManager->OnViewportSizeChange (PixelSize);

	STAT(STAT_SET_RENDER_STATE, 2);
	g_pD3DDevice->SetRenderState (D3DRS_ZENABLE, D3DZB_TRUE);
	g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
}


//---------------------------------
void CP3DPostprocessManager::SetShaderParameters (DWORD dwEffectTableIdx, DWORD dwChainLinkIdx)
{
	MATERIAL_PARAMETER paramTable[SEM_NUM];
	WORD wAllParams, wParamsToSet;
	EFFECT_CHAINLINK	&ChainLink = m_ProcessingChain[dwEffectTableIdx].ChainLink[dwChainLinkIdx];

	g_pMaterialManager->QueryParameters (ChainLink.dwEffect, false, paramTable, wAllParams, wParamsToSet);

	for (DWORD i=0; i<wParamsToSet; i++)
	{
		switch (paramTable[i].SemanticID)
		{
		case SEM_TextureLayer0:
			STAT(STAT_SHADER_TEXTURES, 1);
			paramTable[i].pValue = &m_ProcessingChain[dwEffectTableIdx].ChainLink[ChainLink.wSourceTextureIdx0].pTexture;
			break;
		case SEM_TextureLayer1:
			STAT(STAT_SHADER_TEXTURES, 1);
			paramTable[i].pValue = &m_ProcessingChain[dwEffectTableIdx].ChainLink[ChainLink.wSourceTextureIdx1].pTexture;
			break;
		}
	}

	g_pMaterialManager->SetParameters (ChainLink.dwEffect, paramTable, wAllParams);
}

// nacita effect do cache tabulky
//---------------------------------
bool CP3DPostprocessManager::PreCache (const char *szPostEffectName)
{
	ezxml_t	effect_xml, chainlink_xml;
	char	*pStr;
	DWORD	i;
	int		intFreeIdx = -1;

	// test ci je uz nahrany
	for (i=0; i<CACHE_TABLE_LIMIT; i++)
		if (m_CacheTable[i].wLinksCount)
			if (stricmp (m_CacheTable[i].szEffectName, szPostEffectName) == 0)
				return true;

	// vyhladanie volneho prvku
	for (i=0; i<CACHE_TABLE_LIMIT; i++)
		if (m_CacheTable[i].wLinksCount == 0)
			{intFreeIdx = i; break;}
	if (intFreeIdx < 0) 
	{
		CON(MSG_CON_DEBUG, "PostprocessManager: Maximum number of cached postprocess (%d) effects exceeded!", CACHE_TABLE_LIMIT);
		return false;	// ziadne miesto v cache table
	}

	if ((szPostEffectName == NULL) || (szPostEffectName[0] == 0) || \
				((g_pXML == NULL) || (m_PPEffectsXml == NULL)))
		return false;

	for (effect_xml = g_pXML->Child (m_PPEffectsXml, "effect"); effect_xml != NULL; effect_xml = effect_xml->next)
	{
		if ((pStr = (char*)g_pXML->Attr (effect_xml, "name")) == NULL)
			continue;
		if (stricmp (pStr, szPostEffectName) == 0)
			break;
	}

	if (effect_xml == NULL) 
	{
		CON(MSG_CON_ERR, "PostprocessManager: Failed to load child definition for postprocess %s!", szPostEffectName);
		return false;
	}

	strncpy (m_CacheTable[intFreeIdx].szEffectName, szPostEffectName, sizeof(m_CacheTable[intFreeIdx].szEffectName)-1);
	pStr = (char*)g_pXML->Attr (effect_xml, "queue_priority");
	m_CacheTable[intFreeIdx].sQueuePriority = pStr ? (short)atoi(pStr) : DEFAUL_POSTFX_QUEUE_PRIORITY;
	if (pStr = (char*)g_pXML->Attr (effect_xml, "chainlinks_count"))
		m_CacheTable[intFreeIdx].wLinksCount = (unsigned short)atol(pStr);
	else
		goto quit_hand;

	if (m_CacheTable[intFreeIdx].wLinksCount > MAX_EFFECT_LINKS)
		m_CacheTable[intFreeIdx].wLinksCount = MAX_EFFECT_LINKS;

	for (i=0; i<(DWORD)m_CacheTable[intFreeIdx].wLinksCount; i++)
	{
		char chlBuf[32], szBuf[256] = "materials\\postprocess\\";
		sprintf (chlBuf, "chainlink%u", i);
		// 7mt_material
		if (chainlink_xml = g_pXML->Child (effect_xml, chlBuf))
		{
			pStr = (char*)g_pXML->Attr (chainlink_xml, "7mt_material");
			if (pStr == NULL) goto quit_hand;
			strcat (szBuf, pStr);
			strcat (szBuf, ".7mt");
			m_CacheTable[intFreeIdx].ChainLink[i].dwEffect = g_pMaterialManager->LoadMaterial (szBuf);
			if (m_CacheTable[intFreeIdx].ChainLink[i].dwEffect == 0xFFFFFFFF)
				goto quit_hand;
		}
		// src_tex_index0
		m_CacheTable[intFreeIdx].ChainLink[i].wSourceTextureIdx0 = (WORD)i;
		if (pStr = (char*)g_pXML->Attr (chainlink_xml, "src_tex_index0"))
			m_CacheTable[intFreeIdx].ChainLink[i].wSourceTextureIdx0 = (WORD)atol (pStr);
		// src_tex_index1
		m_CacheTable[intFreeIdx].ChainLink[i].wSourceTextureIdx1 = (WORD)i;
		if (pStr = (char*)g_pXML->Attr (chainlink_xml, "src_tex_index1"))
			m_CacheTable[intFreeIdx].ChainLink[i].wSourceTextureIdx1 = (WORD)atol (pStr);
		// texture_size_exponent
		m_CacheTable[intFreeIdx].ChainLink[i].intTextureSizeExp = 0;
		if (pStr = (char*)g_pXML->Attr (chainlink_xml, "texture_size_exponent"))
		{
			int intTexSizeExp = atoi (pStr);
			if ((intTexSizeExp < 0) || (intTexSizeExp > 4))
				intTexSizeExp = 0;
			m_CacheTable[intFreeIdx].ChainLink[i].intTextureSizeExp = intTexSizeExp;
		}
		// use_alpha_texture
		m_CacheTable[intFreeIdx].ChainLink[i].bUseAlphaTexture = false;
		if (pStr = (char*)g_pXML->Attr (chainlink_xml, "use_alpha_texture"))
			m_CacheTable[intFreeIdx].ChainLink[i].bUseAlphaTexture = (atoi(pStr) == 0) ? false : true;
	}

	return true;
quit_hand:
	m_CacheTable[intFreeIdx].Clear ();
	CON(MSG_CON_ERR, "PostprocessManager: Unhandled exception while loading postprocess %s!", szPostEffectName);
	return false;
}

// odstrani z cache tabulky, taky efekt potom nie je mozne pridat pomocou AddEffect()
//---------------------------------
bool CP3DPostprocessManager::UnCache (const char *szPostEffectName)
{
	int intIdxToUnCache = -1;
	if (szPostEffectName == NULL) return false;

	RemoveEffect (szPostEffectName);

	for (DWORD i=0; i<m_dwEffectsCount; i++)
		if (stricmp (szPostEffectName, m_ProcessingChain[i].szEffectName) == 0)
			{intIdxToUnCache = i; break;}

	if (intIdxToUnCache == -1) return false;

	m_CacheTable[intIdxToUnCache].Clear ();

	return true;
}

// zapne dany effect, effect musi byt predtym nacitany pomocou PreCache()
//---------------------------------
bool CP3DPostprocessManager::AddEffect (const char *szPostEffectName)
{
	DWORD	i, j, dwCacheIdx = 0xFFFFFFFF, dwChainIdx = 0xFFFFFFFF;
	bool	bFreeIdxExist = false;

	if ((szPostEffectName == NULL) || (szPostEffectName[0] == 0))
		return false;

	// vyhladanie v cache
	for (i=0; i<CACHE_TABLE_LIMIT; i++)
		if (stricmp (m_CacheTable[i].szEffectName, szPostEffectName) == 0)
		{
			for (j=0; j<m_CacheTable[i].wLinksCount; j++)
				if (m_CacheTable[i].ChainLink[j].dwEffect == 0xFFFFFFFF)
					{m_CacheTable[i].Clear (); continue;}
			dwCacheIdx = i; break;
		}
	if (dwCacheIdx == 0xFFFFFFFF) return false;

	// test na volne miesto v chain
	for (i=0; i<MAX_POST_EFFECTS; i++)
		if (m_ProcessingChain[i].wLinksCount == 0)
			{bFreeIdxExist = true; break;}
	if (!bFreeIdxExist) return false;

	// zaradenie do chain
	for (i=0; i<MAX_POST_EFFECTS; i++)
	{
		if (m_ProcessingChain[i].wLinksCount == 0)
			{dwChainIdx = i; break;}
		if (m_CacheTable[dwCacheIdx].sQueuePriority >= m_ProcessingChain[i].sQueuePriority)
			continue;
		else
			{dwChainIdx = i; break;}
	}
	if (dwChainIdx == 0xFFFFFFFF) return false;

	if (dwChainIdx < m_dwEffectsCount)
	{
		POSTPROCESS_EFFECT effectBuf = m_CacheTable[dwCacheIdx];
		for (i=dwChainIdx; i<=m_dwEffectsCount; i++)
		{
			POSTPROCESS_EFFECT fxSwapTemp = m_ProcessingChain[i];
			m_ProcessingChain[i] = effectBuf;
			effectBuf = fxSwapTemp;
		}
	}
	else
		m_ProcessingChain[dwChainIdx] = m_CacheTable[dwCacheIdx];

	m_dwEffectsCount++;

	// vytvorenie texture_surface
	for (i=0; i<m_ProcessingChain[dwChainIdx].wLinksCount; i++)
	{
		float fTextureSize = 1.0f / pow (2.0f, m_ProcessingChain[dwChainIdx].ChainLink[i].intTextureSizeExp);
		UINT uiWidth = (UINT) (float(g_pEngSet.Width) * fTextureSize);
		UINT uiHeight = (UINT) (float(g_pEngSet.Height) * fTextureSize);
		bool bWithAlpha = m_ProcessingChain[dwChainIdx].ChainLink[i].bUseAlphaTexture;

		// D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A16B16G16R16F (float_texture)
		if (FAILED (D3DXCreateTexture(g_pD3DDevice, uiWidth, uiHeight, 1, D3DUSAGE_RENDERTARGET, \
							bWithAlpha ? D3DFMT_A8R8G8B8 : D3DFMT_R8G8B8, D3DPOOL_DEFAULT, \
							&m_ProcessingChain[dwChainIdx].ChainLink[i].pTexture)))
			{RemoveEffect(dwChainIdx); return false;}

		if (FAILED (m_ProcessingChain[dwChainIdx].ChainLink[i].pTexture->GetSurfaceLevel (0, &m_ProcessingChain[dwChainIdx].ChainLink[i].pSurface)))
			{RemoveEffect(dwChainIdx); return false;}
	}

	return true;
}

//---------------------------------
bool CP3DPostprocessManager::RemoveEffect (DWORD dwIdx)
{
	DWORD i;
	if ((dwIdx >= MAX_POST_EFFECTS) || (m_dwEffectsCount == 0)) 
	{
		CON(MSG_CON_DEBUG, "PostprocessManager::RemoveEffect() Invalid postprocess id!");
		return false;
	}
	// odstranenie efektu
	m_ProcessingChain[dwIdx].Flush ();
	// posun nasledujucich efektov
	for (i=dwIdx; i<m_dwEffectsCount - 1; i++)
		m_ProcessingChain[i] = m_ProcessingChain[i+1];

	m_ProcessingChain[m_dwEffectsCount - 1].Clear ();
	m_dwEffectsCount--;

	return true;
}

// vypne effect, effect ostava v cache tabulke, je mozne ho neskor zapnut s AddEffect()
//---------------------------------
bool CP3DPostprocessManager::RemoveEffect (const char *szPostEffectName)
{
	int intIdxToRemove = -1;
	if (szPostEffectName == NULL) return false;

	for (DWORD i=0; i<m_dwEffectsCount; i++)
		if (stricmp (szPostEffectName, m_ProcessingChain[i].szEffectName) == 0)
			{intIdxToRemove = i; break;}

	if (intIdxToRemove == -1) return false;

	return RemoveEffect((DWORD)intIdxToRemove);
}
