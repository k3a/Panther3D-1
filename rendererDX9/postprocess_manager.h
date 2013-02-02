//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Postprocess manager class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"


extern ConVar CVr_wireframe;

#define POSTPROCESS_FVF	(D3DFVF_XYZ | D3DFVF_TEX1)


struct POSTPROCESS_VERTEX_FORMAT {
	float	x, y, z;
	float	u, v;
};

const POSTPROCESS_VERTEX_FORMAT ScreenQuad[4] = {	{-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
												{1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
												{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
												{1.0f, -1.0f, 0.0f, 1.0f, 1.0f} };


#define MAX_POST_EFFECTS	10		// max pocet sucasne aktivnych post efektov
#define MAX_EFFECT_LINKS	10		// max pocet chain linkov, teda pocet shaderov na jeden post efekt
#define CACHE_TABLE_LIMIT	40		// max pocet post-efektov v cache

#define DEFAUL_POSTFX_QUEUE_PRIORITY	5


struct EFFECT_CHAINLINK {
	LPDIRECT3DTEXTURE9	pTexture;
	LPDIRECT3DSURFACE9	pSurface;
	DWORD				dwEffect;
	WORD				wSourceTextureIdx0;
	WORD				wSourceTextureIdx1;
	int					intTextureSizeExp;
	bool				bUseAlphaTexture;
	//***********************************
	EFFECT_CHAINLINK() {Clear ();};
	void Clear ()
	{
		pSurface = NULL; pTexture = NULL; dwEffect = 0xFFFFFFFF; wSourceTextureIdx0 = 0;
		wSourceTextureIdx1 = 0; intTextureSizeExp = 0; bUseAlphaTexture = false;
	}
	void Flush ()
		{SAFE_RELEASE(pSurface); SAFE_RELEASE(pTexture); Clear ();}
};

struct POSTPROCESS_EFFECT {
	EFFECT_CHAINLINK		ChainLink[MAX_EFFECT_LINKS];
	char					szEffectName[64];
	unsigned short			wLinksCount;		// pocet efektov v ChainLink
	short					sQueuePriority;
	//***********************************
	POSTPROCESS_EFFECT() {Clear ();}
	void Clear ()
	{
		for (DWORD i=0; i<MAX_EFFECT_LINKS; i++)
			ChainLink[i].Clear ();
		memset (szEffectName, 0, sizeof(szEffectName));
		wLinksCount = 0;
		sQueuePriority = DEFAUL_POSTFX_QUEUE_PRIORITY;
	}
	// to iste co Clear() ale s uvolnenim objektov textury a surface
	void Flush ()
	{
		for (DWORD i=0; i<MAX_EFFECT_LINKS; i++)
			ChainLink[i].Flush ();
		memset (szEffectName, 0, sizeof(szEffectName));
		wLinksCount = 0;
		sQueuePriority = DEFAUL_POSTFX_QUEUE_PRIORITY;
	}
};


//---------------------------------
class CP3DPostprocessManager : public IP3DPostprocessManager
{
private:
	LPDIRECT3DVERTEXDECLARATION9	m_pScreenQuadVertexDecl;
	LPDIRECT3DVERTEXBUFFER9			m_pScreenQuadVertexBuffer;
	LPDIRECT3DSURFACE9				m_pPreviousTarget;

	POSTPROCESS_EFFECT				m_ProcessingChain[MAX_POST_EFFECTS];	// tabulka aktivnych post efektov
	POSTPROCESS_EFFECT				m_CacheTable[CACHE_TABLE_LIMIT];		// cache tabulka vsetkych post efektov

	DWORD							m_dwEffectsCount;		// pocet post-efektov v m_ProcessingChain
	ezxml_t							m_PPEffectsXml;

	void SetShaderParameters (DWORD dwEffectTableIdx, DWORD dwChainLinkIdx);
	bool RemoveEffect (DWORD dwIdx);

public:
	CP3DPostprocessManager();
	~CP3DPostprocessManager();
	bool Init();
	void Begin ();
	void End ();
	void FlushChain ();
	void ClearCache ();
//	void ResolutionChangeNotify ();	 TODO: pridat !!!

	bool PreCache (const char *szPostEffectName);
	bool UnCache (const char *szPostEffectName);
	bool AddEffect (const char *szPostEffectName);
	bool RemoveEffect (const char *szPostEffectName);
};
