//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	TabStrip
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

#define MAX_TABS 10

class CTabStrip : public IGuiElement
{
public:
	CTabStrip();
	~CTabStrip();
	void Update(float deltaTime);
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	bool CreateTabStrip(int x, int y, int w, int h, IGuiElement* pParent);
	void AddTab(const char* szCaption, IGuiElement* pTabFrame); // pTabFrame - ukazatel na CFrame, ktery musi byt ditetem TabStribu!
	void SelectTab(unsigned int i);
	unsigned int GetSelectedTab()const{return m_nCurrentTab;};
	bool OnSelect()const{return m_bOnSelect;};
private:
	char* m_strTab[MAX_TABS];
	IGuiElement* m_pTabFrame[MAX_TABS];
	unsigned int m_nCurrentTab; // aktualni zalozka
	unsigned int m_nLastTab; // stara zalozka
	unsigned int m_nTabs; // pocet zalozek v poli
	bool m_bOnSelect;
	//
	inline void DrawTab(int x, int w, unsigned int nIndex);

	// jen pro spravne ohraniceni
	float m_fActiveX1, m_fActiveX2;
};