//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	CheckBox
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

class CCheckBox : public IGuiElement
{
public:
	CCheckBox();
	~CCheckBox(){ if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption);  };
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	bool CreateCheckBox(const char* szCaption, int x, int y, int w, int h, bool bBold, IGuiElement* pParent);
	void SetCaption(const char* szCaption){
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	};
	bool GetValue()const{return m_bValue;};
	void SetValue(bool bValue){m_bValue=bValue;}
	void Update(float deltaTime);
	bool OnCheck()const{return m_bOnCheck;};
	bool OnUncheck()const{return m_bOnUncheck;};
private:
	char* m_szCaption;
	bool m_bBold;
	//
	bool m_bValue;
	//
	bool m_bDrawGrayed;
	bool m_bDownFirstTime;
	//
	bool m_bOnCheck;
	bool m_bOnUncheck;
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};