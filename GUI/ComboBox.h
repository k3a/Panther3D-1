//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	ComboBox
//			POZOR! Vsechny metody nebyly odzkouseny!!
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

// ID jsou pocitany od 1, prvni polozka ma id 1

#define MAX_CB_ITEMS_PLUS_ONE 16 // max. 15 items

class CComboBox : public IGuiElement
{
public:
	CComboBox();
	~CComboBox() { Clear(false); };
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	void Update(float deltaTime);
	bool CreateComboBox(int x, int y, int w, int h, IGuiElement* pParent);
	char* GetItem(UINT nID)const{if(nID>lastID)nID=lastID; return m_pszItems[nID];}; // NEMÌNIT ZISKANY STRING! JEN KOPIROVAT!
	UINT GetNumItem()const{return lastID;};
	void AddItem(const char* pszItemText);
	void Clear(bool forceNull=false); // forceNull only for internal purpose - DON'T USE!
	void SelectItem(UINT nID){selItem=nID; if(selItem>lastID)selItem=lastID; m_bNowSelected=true;};
	UINT GetSelectedItem()const{return selItem;};
	UINT OnSelect()const{
		if(m_bNowSelected)
			return selItem;
		else
			return 0;
	};
private:
	char* m_pszItems[MAX_ITEMS_PLUS_ONE];
	UINT lastID; // id v poli posledniho pridaneho prvku, novy = tohle+1
	UINT selItem;
	//
	bool m_bButtonDownFirstTime;
	bool m_bButtonDown;
	bool m_bPressed;
	bool m_bWaitMenu;
	bool m_bNowSelected;
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};