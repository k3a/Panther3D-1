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
#include "menu.h"
#include "common.h"

CMenu g_pMenu;

void CMenu::Clear(bool forceNull)
{
	for(int i=0;i<MAX_ITEMS_PLUS_ONE;i++)
	{
		if (forceNull) 
			m_pszItems[i]=NULL;
		else
			if(m_pszItems[i]) SAFE_DELETE_ARRAY(m_pszItems[i]);
	}
	lastID=0;
	selItem=0;
}

void CMenu::DrawAndUpdate(float deltaTime)
{
	if (selItem!=0 && selItem!=99) 
	{
		selItem=99;
		Clear();
	}
	else
		selItem=0;

	if (!lastID) return; // neni co vykreslovat

	// vykresli pozadi menu (lastID je vlastne pocet polozek)
	// spocti max sirku
	UINT wid=minWid;
	for(UINT i=1;i<=lastID;i++)
	{
		if(m_pszItems[i])
		{
			int currWid=0;
			g_Scheme.GetFontLabel()->GetTextSize(m_pszItems[i], NULL, &currWid, NULL);
			currWid+=20; // mezera pred a za polozkou
			if ((UINT)currWid>wid)wid=(UINT)currWid;
		}
	}
	g_p2D->Line_DrawFilledRect(posX, posY, wid, lastID*15+6, g_Scheme.GetColor(COL_BG));
	g_p2D->Line_Draw3DRect(posX, posY, wid, lastID*15+6, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_LIGHTSHADOW));

	// vykresli a ziskej polozku pod kurzorem
	int currY=posY+3;
	bool bSelected=false;
	for(UINT i=1;i<=lastID;i++)
	{
		if(m_pszItems[i])
		{
			// mys nad polozkou?
			if(g_nMouseX>(int)posX && g_nMouseX < (int)posX+(int)wid && g_nMouseY > currY && g_nMouseY < currY+15)
			{	
				g_p2D->Line_DrawFilledRect(posX+2, currY, wid-4, 15, g_Scheme.GetColor(COL_HIGHLIGHT));
				if (g_bLMouseDownFirstTime && !bSelected) 
				{
					selItem=i;
					bSelected=true;
				}
			}
			g_Scheme.GetFontLabel()->DrawText(FA_NOCLIP, m_pszItems[i], posX+10, currY, 0, 0, g_Scheme.GetColor(COL_TEXT));
			currY+=15;
		}
	}
	if (!bSelected && g_bLMouseDownFirstTime) // klikl mimo menu
	{
		Clear();
	}
}

void CMenu::AddItem(const char* szItemText)
{
	lastID++;
	if (lastID==MAX_ITEMS_PLUS_ONE-1)
	{
		CON(MSG_CON_ERR, "GUI: Max CMenu items!!! (MAX_ITEMS_PLUS_ONE)");
		return;
	}
	m_pszItems[lastID]=new char[strlen(szItemText)+1];
	strcpy(m_pszItems[lastID], szItemText);
}