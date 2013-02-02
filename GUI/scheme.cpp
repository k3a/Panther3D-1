//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI Color Scheme
//-----------------------------------------------------------------------------
#include "scheme.h"

CScheme::CScheme()
{
	_pTitleFont = NULL;
	_pLabelFont = NULL;
	_pLabelFontBold = NULL;
	_pMenuTitleFont = NULL;
	_pCursorTex = NULL;
}

CScheme::~CScheme()
{
	SAFE_DELETE(_pTitleFont);
	SAFE_DELETE(_pLabelFont);
	SAFE_DELETE(_pLabelFontBold);
	SAFE_DELETE(_pMenuTitleFont);
	SAFE_DELETE(_pCursorTex);
}

DWORD CScheme::GetColor(int nType)
{
	return m_lColors[nType];
}

bool CScheme::Init()
{
	//TODO: V budoucnu nacist ze souboru?

	// ---> BARVY
	m_lColors[COL_BG] = P3DCOLOR_ARGB(240, 130, 130, 130);
	m_lColors[COL_HIGHLIGHT] = P3DCOLOR_ARGB(240, 80, 0, 0); //FIXME:
	m_lColors[COL_LIGHT] = P3DCOLOR_ARGB(240, 150, 150, 150);
	m_lColors[COL_SHADOW] = P3DCOLOR_ARGB(240, 60, 60, 60);
	m_lColors[COL_LIGHTSHADOW] = P3DCOLOR_ARGB(240, 80, 80, 80);
	m_lColors[COL_TITLE] = P3DCOLOR_ARGB(240, 110, 110, 110);
	m_lColors[COL_TITLE_ACTIVE] = P3DCOLOR_ARGB(240, 110, 100, 100);
	m_lColors[COL_TITLE_TEXT] = P3DCOLOR_ARGB(255, 180, 180, 180);
	m_lColors[COL_TEXT] = P3DCOLOR_ARGB(255, 200, 200, 200);

	// ---> FONTY
	_pTitleFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	_pTitleFont->CreateFont("MS Sans Serif", 12, true, false);
	_pLabelFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	_pLabelFont->CreateFont("MS Sans Serif", 12, false, false);
	_pLabelFontBold = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	_pLabelFontBold->CreateFont("MS Sans Serif", 12, true, false);
	_pMenuTitleFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	_pMenuTitleFont->CreateFont("Arial", 60, true, false);

	// kurzor
	_pCursorTex = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE);
	if(!_pCursorTex->Load("gui/cursor.dds", TT_2D))
	{
		SAFE_DELETE(_pCursorTex);
		CON(MSG_CON_ERR, "GUI: Can't load cursor texture gui/cursor.dds!");
		return false;
	}
	return true;
}