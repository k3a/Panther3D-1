//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	ComboBox
//-----------------------------------------------------------------------------
#include "ComboBox.h"

CComboBox::CComboBox()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_bButtonDownFirstTime = false;
	m_bButtonDown = false;
	m_bPressed = false;
	m_bWaitMenu = false;

	Clear(true);
	
	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_LINE | EF_SPRITE); // kreslime line + text
}

bool CComboBox::CreateComboBox(int x, int y, int w, int h, IGuiElement* pParent)
{
	// nastav parentu jako child tento prvek
	pParent->SetChild(this);
	m_pParent = pParent;

	// nastav
	
	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;

	return true;
}

void CComboBox::RenderLine(float deltaTime)
{
	// vykresli ohraniceni
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_LIGHTSHADOW), g_Scheme.GetColor(COL_LIGHT));

	// vykresli tlacitko
	if (m_bButtonDown)
	{
		g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX+m_nWidth-m_nHeight+2, m_nPosY+m_nParentY+2, m_nHeight-4, m_nHeight-4, g_Scheme.GetColor(COL_SHADOW), g_Scheme.GetColor(COL_LIGHT));
	}
	else
	{
		g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX+m_nWidth-m_nHeight+2, m_nPosY+m_nParentY+2, m_nHeight-4, m_nHeight-4, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_SHADOW));
	}
}
void CComboBox::RenderSprite(float deltaTime)
{
	// VYKRESLI TEXT
	// vykresli tlacitko
	if (m_bButtonDown)
	{
		g_Scheme.GetFontLabel()->DrawText(FA_NOCLIP, "v", m_nPosX+m_nParentX+m_nWidth-m_nHeight/2-3, m_nPosY+m_nParentY+m_nHeight/2-6, 6, 12, g_Scheme.GetColor(COL_TEXT));
	}
	else
	{
		g_Scheme.GetFontLabel()->DrawText(FA_NOCLIP, "v", m_nPosX+m_nParentX+m_nWidth-m_nHeight/2-3, m_nPosY+m_nParentY+m_nHeight/2-7, 6, 14, g_Scheme.GetColor(COL_TEXT));
	}

	// vykresli vybrany text
	if (m_pszItems[selItem]) g_Scheme.GetFontLabel()->DrawText(FA_LEFT, m_pszItems[selItem], m_nPosX+m_nParentX+3, m_nPosY+m_nParentY+m_nHeight/2-6, m_nPosX+m_nParentX+m_nWidth-m_nHeight-1, m_nPosY+m_nParentY+m_nHeight, g_Scheme.GetColor(COL_TEXT));
}

void CComboBox::Update(float deltaTime)
{
	// GUI EDITOR
	if(CVgui_editor.GetBool() && !(GetElementFlags() & EF_NOT_EDITABLE))
	{
		if (!m_szName) return; // neoznacene prvky nelze upravovat

		// popisek
		char str[64];
		bool popisek=false;

		if (g_bLMouseDownFirstTime)
		{
			if ((g_nMouseX>m_nPosX+m_nParentX+m_nWidth-3 && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth)
				&& (g_nMouseY>m_nPosY+m_nParentY+m_nHeight-3 && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight))
			{
				edit_mode = 2; // resize
			}
			else if (g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
				&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight)
			{
				edit_mode = 1; // move
				edit_nRelPosX = g_nMouseX - m_nPosX;
				edit_nRelPosY = g_nMouseY - m_nPosY;
			}
		}
		else if (g_bLMouseUpFirstTime)
		{
			edit_mode = 0; // konec
		}

		// pokud je co delat, tak to udelej
		if (edit_mode==1) // move
		{
			m_nPosX = g_nMouseX - edit_nRelPosX;
			m_nPosY = g_nMouseY - edit_nRelPosY;
			sprintf(str, "X%d Y%d", m_nPosX, m_nPosY);
			g_Scheme._pLabelFont->DrawText(str, g_nMouseX+10, g_nMouseY+10, P3DCOLOR_XRGB(255, 255, 255));
			popisek = true;
		}
		else if (edit_mode==2) // resize
		{
			m_nWidth = g_nMouseX - m_nPosX - m_nParentX;
			m_nHeight = g_nMouseY - m_nPosY - m_nParentY;
			sprintf(str, "W%d H%d", m_nWidth, m_nHeight);
			g_Scheme._pLabelFont->DrawText(str, g_nMouseX+10, g_nMouseY+10, P3DCOLOR_XRGB(255, 255, 255));
			popisek = true;
		}

		// pokud neni zaden popisek a mys je nad timto prvkem, udelej popisek
		if (g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
			&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight)
		{
			// popisek
			char str[64];
			if (!popisek)
			{
				// vykresli okraje
				g_p2D->Line_DrawRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, P3DCOLOR_ARGB(100, 0, 255, 0));

				sprintf(str, "%s", m_szName);
				g_Scheme._pLabelFont->DrawText(str, g_nMouseX+10, g_nMouseY+10, P3DCOLOR_ARGB(100, 255, 255, 255));
			}
		}
		return; // editor mode
	}
	//-GUI EDITOR

	m_bNowSelected=false;
	m_bPressed = false;
	//Aktualizuj stav tlacitka
	if (g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
		&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight) 
	{
		if(g_bLMouseDownFirstTime) m_bButtonDownFirstTime = true;
		if(m_bButtonDownFirstTime) m_bButtonDown = true; else m_bButtonDown = false;

		if (m_bButtonDown && g_bLMouseUpFirstTime) m_bPressed=true;// TLACITKO ZMACKUTO A POVOLENO!
	}
	else
		m_bButtonDown = false;

	if(g_bLMouseUpFirstTime) m_bButtonDownFirstTime = false;

	if (m_bPressed) // tlacitko zmacknuto
	{
		m_bWaitMenu = true;
		g_pMenu.Clear();
		for(UINT i=1;i<=lastID;i++)
		{
			g_pMenu.AddItem(m_pszItems[i]);
		}
		g_pMenu.SetPos(m_nPosX+m_nParentX, m_nPosY+m_nParentY+m_nHeight, m_nWidth);
	}

	if (m_bWaitMenu) // je zobrazeno menu
	{
		UINT nSel = g_pMenu.GetSelectedItem();
		if (nSel==99) { 
			m_bWaitMenu=false;
		}
		else if (nSel==0)
		{
		}
		else // nejake konkretni id
		{
			m_bNowSelected=true;
			selItem = nSel;
		}
	}
}

void CComboBox::AddItem(const char* pszItemText)
{
	lastID++;
	if (lastID==MAX_CB_ITEMS_PLUS_ONE-1)
	{
		CON(MSG_CON_ERR, "GUI: Max CComboBox items!!! (MAX_CB_ITEMS_PLUS_ONE)");
		return;
	}
	m_pszItems[lastID]=new char[strlen(pszItemText)+1];
	strcpy(m_pszItems[lastID], pszItemText);
}

void CComboBox::Clear(bool forceNull)
{
	for(int i=0;i<MAX_CB_ITEMS_PLUS_ONE;i++)
	{
		if (forceNull) 
			m_pszItems[i]=NULL;
		else
			if(m_pszItems[i]) SAFE_DELETE_ARRAY(m_pszItems[i]);
	}
	lastID=0;
	selItem=1;
}