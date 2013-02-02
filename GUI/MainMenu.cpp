//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Main menu with logo, TODO: logo + stmivani
//-----------------------------------------------------------------------------
#include "MainMenu.h"

#define ITEM_SEPARATOR_SIZE 40

void CMainMenu::Clear()
{
	UINT i;
	for(i=0;i<m_nLastId;i++)
	{
		SAFE_DELETE_ARRAY(m_szTexts[i]);
	}
}

void CMainMenu::AddItem(const char* szInterText)
{
	m_szTexts[m_nLastId] = new char[strlen(szInterText)+1];
	strcpy(m_szTexts[m_nLastId], szInterText);
	m_nLastId++;
}

void CMainMenu::Render()
{
	//VYKRESLI TITULEK
	int widT=0;
	g_Scheme._pMenuTitleFont->GetTextSize(g_pLocaliz->Translate_const("#GUI_GameTitle"), NULL, &widT, NULL);
	g_Scheme._pMenuTitleFont->DrawText(FA_NOCLIP, g_pLocaliz->Translate_const("#GUI_GameTitle"), CVr_width->GetInt()/2-widT/2, 45, 0, 0, P3DCOLOR_XRGB(200, 200, 200));

	m_nSelItem=0;
	int nTotalWid=0, nTotalHei=0;
	UINT i;
	for(i=0;i<m_nLastId;i++)
	{
		int wid=0, hei=0;
		m_pItemText->GetTextSize(m_szTexts[i], NULL, &wid, &hei);
		if (hei>nTotalHei)nTotalHei=hei;
		nTotalWid+=wid+ITEM_SEPARATOR_SIZE;
	}

	int currX=0, currY=0;
	currY = 150; // TODO:!
	currX = (int)(CVr_width->GetFloat()/2-nTotalWid/2);
	IGuiElement* pForegroundWin = GetFirstVisibleWindow();
	for(i=0;i<m_nLastId;i++)
	{
		// zmer nove x
		int wid=0, hei=0;
		m_pItemText->GetTextSize(m_szTexts[i], NULL, &wid, &hei);

		// vykresli item
		if (g_nMouseX >= currX && g_nMouseX <= currX+wid && g_nMouseY >= currY && g_nMouseY <= currY + hei
			&& GetWindowAtPos(g_nMouseX, g_nMouseY) == NULL && (!pForegroundWin || !((CWindow*)pForegroundWin)->IsModal()))
		{ // mys je nad polozkou a nekoliduje s zadnym oknem
			m_pItemText->DrawText(FA_NOCLIP, m_szTexts[i], currX, currY, 0, 0, P3DCOLOR_ARGB(255, 255, 255, 255));
			if (g_bLMouseDownFirstTime)
				m_nSelItem = i+1;
		}
		else
			m_pItemText->DrawText(FA_NOCLIP, m_szTexts[i], currX, currY, 0, 0, P3DCOLOR_ARGB(255, 200, 200, 200));

		currX+=wid+ITEM_SEPARATOR_SIZE;
	}
}

bool CMainMenu::Initialize()
{
	m_pItemText = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	if(SUCCEEDED(m_pItemText->CreateFont("Arial", 20, true, false))) // TODO: lepsi font
		return true;
	else
	{
		CON(MSG_CON_ERR, "CMainMenu: Failed to load font!");
		return false;
	}
}

void CMainMenu::Shutdown()
{
	UINT i;
	for(i=0;i<m_nLastId;i++)
	{
		SAFE_DELETE_ARRAY(m_szTexts[i]);
	}

	SAFE_DELETE(m_pItemText);
}

void CMainMenu::ChangeItem(UINT nId, const char* szInterText)
{
	SAFE_DELETE_ARRAY(m_szTexts[nId-1]);
	m_szTexts[nId-1]=new char[strlen(szInterText)+1];
	strcpy(m_szTexts[m_nLastId], szInterText);
}