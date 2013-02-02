//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Foreground menu (like singleton)
//-----------------------------------------------------------------------------
#pragma once

#include <windows.h>
#include "IP3DFont.h"

#define MAX_ITEMS_PLUS_ONE 16 // max. 15 items

class CMenu
{
public:
	CMenu::CMenu(){Clear(true);posX=0;posY=0;};
	void DrawAndUpdate(float deltaTime);
	void AddItem(const char* szItemText);
	void Clear(bool forceNull=false); // forceNull jen pro vnitrni potreby - NEPOUZIVAT
	UINT GetSelectedItem()const{return selItem;}; // cisluje se od 1 - prvni polozka ma id 1. Vrati 99 pokud zmizelo - bylo kliknuto mimo menu
	void SetPos(UINT nPosX, UINT nPosY, UINT nMinWidth){posX=nPosX;posY=nPosY;minWid=nMinWidth;};
private:
	char* m_pszItems[MAX_ITEMS_PLUS_ONE];
	UINT lastID; // id v poli posledniho pridaneho prvku, novy = tohle+1
	UINT selItem;
	UINT posX, posY, minWid;
};

extern CMenu g_pMenu;