//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Asset Browser Helper
//-----------------------------------------------------------------------------
#include "editor_assetbrowser.h"

REGISTER_SINGLE_CLASS(CP3DEditorAssetBrowser, IP3DENGINE_EDITORASSETBROWSER);

CP3DEditorAssetBrowser::CP3DEditorAssetBrowser()
{

}

CP3DEditorAssetBrowser::~CP3DEditorAssetBrowser()
{
	SAFE_DELETE(m_pRT);
}

bool CP3DEditorAssetBrowser::Init()
{
	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);
	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);
	m_pRT = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE);
	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);

	m_pRT->Create(98, 98, 1, TF_RGB8, true);
	return true;
}


void CP3DEditorAssetBrowser::DrawTexture(IP3DTexture *pTexture)
{
	if (!pTexture) return;

	m_pRT->SetAsRenderTarget(0);

	g_p2D->DrawTexture(pTexture, 0, 0, 98, 98);

	g_pRenderer->SetDefaultRenderTarget();

	g_p2D->DrawTexture(m_pRT, 0, 0, g_pEngSet.Width, g_pEngSet.Height);
}