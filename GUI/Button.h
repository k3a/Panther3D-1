//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Buton
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

class CButton : public IGuiElement
{
public:
	CButton();
	~CButton(){ if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption);  };
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	void Update(float deltaTime);
	bool CreateButton(const char* szCaption, int x, int y, int w, int h, IGuiElement* pParent);
	void SetCaption(const char* szCaption){
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	};
	bool OnPress(){ bool old=m_bPressed;m_bPressed=false; return old;};
private:
	char* m_szCaption;
	//
	bool m_bButtonDownFirstTime;
	bool m_bButtonDown;
	bool m_bPressed;
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};

#define INIT_ELEMENT(uniqueName) uniqueName .m_szName = #uniqueName