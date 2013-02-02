//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Frame (Group)
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

class CFrame : public IGuiElement
{
public:
	CFrame();
	~CFrame(){ if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption);  };
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	bool CreateFrame(const char* szCaption, int x, int y, int w, int h, bool bDrawBorder, IGuiElement* pParent);
	void SetCaption(const char* szCaption){
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	};
private:
	char* m_szCaption;
	bool m_bDrawBorder;
};