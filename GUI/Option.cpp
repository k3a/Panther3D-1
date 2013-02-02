//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	OptionButton - Radio
//-----------------------------------------------------------------------------
#include "option.h"

#define CBOX_WID 12 // sirka policka - delitelne 2 beze zbytku
#define CBOX_HEI 12 // vyska policka  - delitelne 2 beze zbytku

COption::COption()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_bBold = false;
	m_bValue = false;
	m_bDrawGrayed = false;
	m_bDownFirstTime = false;

	m_bOnSelect = false;
	m_bOnUnselect = false;

	m_szCaption = new char[7];
	strcpy(m_szCaption, "Option");

	m_nElementID=1; // option :)
	
	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_SPRITE | EF_LINE); // kreslime text + line
}

bool COption::CreateOption(const char* szCaption, int x, int y, int w, int h, bool bBold, IGuiElement* pParent)
{
	// nastav parentu jako child tento prvek
	pParent->SetChild(this);
	m_pParent = pParent;

	// nastav
	if(szCaption)
	{
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	}
	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;

	m_bBold = bBold;

	return true;
}

void COption::RenderLine(float deltaTime)
{
	// vykresli zaskrtavaci policko
	if(!m_bDrawGrayed) g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, CBOX_WID-1, CBOX_HEI, g_Scheme.GetColor(COL_LIGHTSHADOW));
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, CBOX_WID-1, CBOX_HEI, g_Scheme.GetColor(COL_SHADOW), g_Scheme.GetColor(COL_LIGHT));

	// vykresli zaskrtnuti, pokud je
	if(m_bValue)
	{
		m_nPosX-=1;
		g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+4, m_nPosY+m_nParentY+4, CBOX_WID-8, CBOX_HEI-8, g_Scheme.GetColor(COL_LIGHT));
		//g_p2D->Line_DrawSimple(m_nPosX+m_nParentX+3, m_nPosY+m_nParentY+3, m_nPosX+m_nParentX+CBOX_WID-2, m_nPosY+m_nParentY+CBOX_HEI-2, g_Scheme.GetColor(COL_TEXT)); /* \ */
		//g_p2D->Line_DrawSimple(m_nPosX+m_nParentX+CBOX_WID-2, m_nPosY+m_nParentY+2, m_nPosX+m_nParentX+3, m_nPosY+m_nParentY+CBOX_HEI-3, g_Scheme.GetColor(COL_TEXT)); /* / */
		m_nPosX+=1;
	}
}

void COption::RenderSprite(float deltaTime)
{
	// VYKRESLI TEXT
	if (m_bBold)
		g_Scheme.GetFontLabelBold()->DrawText(FA_LEFT, m_szCaption, (int)(m_nPosX+m_nParentX+CBOX_WID+4), (int)(m_nPosY+m_nParentY)-1, (int)(m_nPosX+m_nParentX+m_nWidth), (int)(m_nPosY+m_nParentY+m_nHeight), g_Scheme.GetColor(COL_TEXT));
	else
		g_Scheme.GetFontLabel()->DrawText(FA_LEFT, m_szCaption, (int)(m_nPosX+m_nParentX+CBOX_WID+4), (int)(m_nPosY+m_nParentY)-1, (int)(m_nPosX+m_nParentX+m_nWidth), (int)(m_nPosY+m_nParentY+m_nHeight), g_Scheme.GetColor(COL_TEXT));
}

void COption::Update(float deltaTime)
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
	
	m_bDrawGrayed=false;
	m_bOnSelect = false;
	m_bOnUnselect = false;

	if(g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
		&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight)
	{
		if (g_bLMouseDownFirstTime) m_bDownFirstTime=true;
		if(g_bLMouseDown && m_bDownFirstTime) m_bDrawGrayed=true; // vykresli zesedle
		if(g_bLMouseUpFirstTime)
		{
			if (m_bDownFirstTime) {
				// vsechny nastav jako false
				for(unsigned int i=0;i<m_pParent->m_nChilds;i++)
				{
					if(m_pParent->m_pChilds[i]->m_nElementID == 1) // je to Option
					{
						// pokud to neni tento Option a neni skryt
						if(!m_pParent->m_pChilds[i]->m_bHidden)
						{
							COption* pOpt = (COption*)m_pParent->m_pChilds[i];
							pOpt->SetValue(false); // nastav jako false
						}
					}
				}
				m_bValue = true; // vyber tento jako aktivni
			}
			m_bDownFirstTime = false;
		}
	}

	// zjisti zda byl aktivovan nebo deaktivovan
	if (m_bLastValue == false && m_bValue == true)
		m_bOnSelect = true;
	else if (m_bLastValue == true && m_bValue == false)
		m_bOnUnselect = true;

	m_bLastValue = m_bValue;
}