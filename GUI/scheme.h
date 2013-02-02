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
#pragma once
#include "common.h"
#include "IP3DTexture.h"
#include "IP3DFont.h"

enum ColorTypes
{
	COL_BG, // pozadi oken a prvku
	COL_LIGHT, // svetle ohraniceni
	COL_SHADOW, // tmave ohraniceni
	COL_LIGHTSHADOW, // svetlejsi tmave ohraniceni
	COL_TITLE, // pozadi titulku okna
	COL_TITLE_ACTIVE, // pozadi titulku aktivniho okna
	COL_TITLE_TEXT, // text titulku okna
	COL_TEXT, // barva textu + vubec popredi
	COL_HIGHLIGHT, // vysviceni, napr. pokud je mys nad polozkou menu, nebo taky progressbar
	_COL_MAX
};

class CScheme
{
public:
	CScheme();
	~CScheme();
	DWORD GetColor(int nType);
	bool Init();
	IP3DFont* GetFontTitle()const{return _pTitleFont;};
	IP3DFont* GetFontLabel()const{return _pLabelFont;};
	IP3DFont* GetFontLabelBold()const{return _pLabelFontBold;};
private:
	DWORD m_lColors[_COL_MAX];
public:
	IP3DTexture* _pCursorTex;
	IP3DFont* _pTitleFont;
	IP3DFont* _pLabelFont;
	IP3DFont* _pLabelFontBold;
	IP3DFont* _pMenuTitleFont;
};