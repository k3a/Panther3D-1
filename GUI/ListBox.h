//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	ListBox se scrollbarem
//			POZOR! Vsechny metody nebyly odzkouseny!!
//-----------------------------------------------------------------------------
#pragma once

#include "IElement.h"
#include "common.h"

#include <vector>
using namespace std;

#pragma warning(disable : 4267)

// ID jsou pocitany od 1, prvni polozka ma id 1

#define MAX_LB_ITEMS_PLUS_ONE 16 // max. 15 items
class CScrollBarVert;
class CListBox : public IGuiElement
{
public:
	CListBox();
	~CListBox();
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	void Update(float deltaTime);
	bool CreateListBox(int x, int y, int w, int h, IGuiElement* pParent);
	char* GetItem(UINT nID)const{if(nID>m_vItems.size())nID=m_vItems.size(); return m_vItems[nID-1];}; // NEMÌNIT ZISKANY STRING! JEN KOPIROVAT!
	UINT GetNumItem()const{return m_vItems.size();};
	void AddItem(const char* pszItemText);
	void Clear();
	void SelectItem(UINT nID){selItem=nID; if(selItem>m_vItems.size())selItem=m_vItems.size(); m_bNowSelected=true;};
	UINT GetSelectedItem()const{return selItem;};
	UINT OnSelect(){
		bool old=m_bNowSelected;
		m_bNowSelected = false;
		if(old)
			return selItem;
		return 0;
	};
	UINT OnDblClick()
	{
		bool old = m_bDoubleClick;
		m_bDoubleClick = false;
		if(old) 
			return selItem;
		return 0;
	}
private:
	CScrollBarVert* m_pScroll;
	bool m_bNowSelected;

	vector<char*> m_vItems;
	UINT selItem;
	//
	bool m_bDoubleClick;
		float m_fLastClickTime;
	float m_fCurrentTime;
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};