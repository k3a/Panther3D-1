//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Label
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

enum LabelAlign
{
	LA_LEFT = 0x00000000,
	LA_CENTER = 0x00000001,
	LA_RIGHT = 0x00000002,
	LA_VCENTER = 0x00000004, // pouze pokud je zaroven LA_SINGLELINE
	LA_BOTTOM = 0x00000008, // pouze pokud je zaroven LA_SINGLELINE
	LA_WORDBREAK = 0x00000010,
	LA_SINGLELINE = 0x00000020,
	LA_NOCLIP = 0x00000100 // nebude zarovnavano na rect
};

class CLabel : public IGuiElement
{
public:
	CLabel();
	~CLabel()
	{
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption);
	}
	void RenderSprite(float deltaTime);
	bool CreateLabel(const char* szCaption, int x, int y, int w, int h, bool bBold, int align, IGuiElement* pParent);
	void SetCaption(const char* szCaption){
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	};
	void Update(float deltaTime);
private:
	char* m_szCaption;
	bool m_bBold;
	int m_eAlign;
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};