//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Vertical ScrollBar
//-----------------------------------------------------------------------------
#include "ScrollBarVert.h"

#define BAR_SIZE_X 5 // velikost posuvnicku X
#define BAR_SIZE_Y 15 // velikost posuvnicku Y

CScrollBarVert::CScrollBarVert()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;
	//
	m_fValue=0.0f;
	m_fValueMin=0.0f;
	m_fValueMax=1.0f;
	m_bChanged = true;
	m_bRelY = -1;

	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_LINE); // kreslime line
}

bool CScrollBarVert::CreateScrollBar(int x, int y, int w, int h, IGuiElement* pParent)
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

void CScrollBarVert::RenderLine(float deltaTime)
{
	// vykresli pozadi
	g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_LIGHTSHADOW));
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_LIGHTSHADOW), g_Scheme.GetColor(COL_LIGHT));

	// vykresli krajni tlacitka
	g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+1, m_nWidth-2, m_nWidth-2, g_Scheme.GetColor(COL_LIGHT));
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+1, m_nWidth-2, m_nWidth-2, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_LIGHTSHADOW));

	g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+m_nHeight-m_nWidth+1, m_nWidth-2, m_nWidth-2, g_Scheme.GetColor(COL_LIGHT));
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+m_nHeight-m_nWidth+1, m_nWidth-2, m_nWidth-2, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_LIGHTSHADOW));

	// vykresli posuvnik
	int y = (int)((m_nHeight-BAR_SIZE_X-(m_nWidth)*2)/(m_fValueMax-m_fValueMin)*(m_fValue-m_fValueMin));
	g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+m_nWidth+y, m_nWidth-2, BAR_SIZE_X, g_Scheme.GetColor(COL_BG));
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX+1, m_nPosY+m_nParentY+m_nWidth+y, m_nWidth-2, BAR_SIZE_X, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_LIGHTSHADOW));
}

void CScrollBarVert::Update(float deltaTime)
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

	m_bChanged = false;

	if(g_bLMouseDownFirstTime)
	{
		// pokud na posuvniku, zjisti rel. pozici od posuvniku
		float fPosuvnikX = (float)m_nPosX+m_nParentX;
		float fPosuvnikY = (float)m_nPosY+m_nParentY+((m_nHeight-BAR_SIZE_X)/(m_fValueMax-m_fValueMin)*(m_fValue-m_fValueMin));
		if (g_nMouseY >= fPosuvnikY && g_nMouseY <= fPosuvnikY + BAR_SIZE_X
			&& g_nMouseX >= fPosuvnikX && g_nMouseX <= fPosuvnikX + BAR_SIZE_Y)
		{
			m_bRelY = g_nMouseX-(int)fPosuvnikX;
		}
		else // pokud je mimo posuvnik
		{
			if (g_nMouseY >= m_nPosY+m_nParentY+m_nWidth-1 && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight-m_nWidth+1
				&& g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth)
			{
				// pokud je vubec v oblasti tohoto prvku
				// posuneme tam slider a chovame se, jako by hybal se sliderem - odlisnost od Windows
				m_bRelY = BAR_SIZE_X/2;
			}
		}
	}

	if(g_bLMouseUpFirstTime) 
	{
		if (m_bRelY>-1) m_bChanged=true;
		m_bRelY=-1;
	}

	if (m_bRelY > -1) // posun posuvnikem = zmena hodnoty
	{
		float fPoziceYPosuvniku = (float)g_nMouseY;
		// uprav na min a max
		if (fPoziceYPosuvniku < (float)m_nPosY+m_nParentY+m_nWidth-1+BAR_SIZE_X/2) fPoziceYPosuvniku = (float)m_nPosY+m_nParentY+m_nWidth-1+BAR_SIZE_X/2;
		if (fPoziceYPosuvniku > (float)m_nPosY+m_nParentY+m_nHeight-m_nWidth-BAR_SIZE_X/2) fPoziceYPosuvniku = (float)m_nPosY+m_nParentY+m_nHeight-m_nWidth-BAR_SIZE_X/2;
		fPoziceYPosuvniku = fPoziceYPosuvniku-m_nPosY-m_nParentY-m_nWidth+1-BAR_SIZE_X/2;
		float fVal = (m_fValueMax-m_fValueMin)/(m_nHeight-BAR_SIZE_X/2-2*(m_nWidth+1))*fPoziceYPosuvniku;
		fVal += m_fValueMin;
		m_fValue = fVal;
		// Kvuli mym matematickym chybam to proste omezime ;)
		if (m_fValue < m_fValueMin) m_fValue=m_fValueMin;
		if (m_fValue > m_fValueMax) m_fValue=m_fValueMax;
		//m_fValue = m_fValueMax-m_fValue; // slidujeme na opacnou stranu - kvuli prepisu z hor. slidebaru
	}
}