//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	ProgressBar |====------|
//-----------------------------------------------------------------------------
#include "ProgressBar.h"

CProgressBar::CProgressBar()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_fValue=0.0f;
	m_fValueMin=0.0f;
	m_fValueMax=1.0f;

	m_bDrawOnlyPartitions = false;
	m_nMaxParts = 1;

	m_bChanged = true;

	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_LINE); // kreslime line
}

bool CProgressBar::CreateProgressBar(int x, int y, int w, int h, bool drawOnlyPartitions, IGuiElement* pParent)
{
	// nastav parentu jako child tento prvek
	pParent->SetChild(this);
	m_pParent = pParent;

	// nastav
	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;
	m_bDrawOnlyPartitions = drawOnlyPartitions;

	// vypocti velikost jedne casti tak, aby byla umerna vysce a pri plnem progressbaru nechybelo do konce
	m_fPartSize = m_nHeight/1.75f + 2.0f; // priblizna sirka jedne casti + mezera 2 pixely
	m_nMaxParts = (int)((m_nWidth-4.0f)/m_fPartSize); // pocet celych casti do celeho progressbaru
	float fRestSize = m_nWidth-2.0f-(m_nMaxParts*m_fPartSize); // pocet zbyvajicich pixelu
	m_fPartSize+=fRestSize/m_nMaxParts; // pridej tak, aby se doplnil zbytek
	return true;
}

void CProgressBar::RenderLine(float deltaTime)
{
	// vykresli pozadi dle hodnoty
	float fValueWid = (m_nWidth-4.0f)/(m_fValueMax-m_fValueMin)*(m_fValue-m_fValueMin);
	
	if (!m_bDrawOnlyPartitions) // vykresli plny progressbar
		g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+2, m_nPosY+m_nParentY+2, (int)fValueWid, m_nHeight-4, g_Scheme.GetColor(COL_HIGHLIGHT));
	else // vykresli progressbar se segmenty
	{
		unsigned int numParts = (int)(m_nMaxParts/(m_fValueMax-m_fValueMin)*(m_fValue-m_fValueMin));
		for (unsigned int i=0;i<numParts;i++)
		{
			g_p2D->Line_DrawFilledRect(2+m_nPosX+m_nParentX+(int)(i*m_fPartSize), m_nPosY+m_nParentY+2, (int)m_fPartSize-2/*odecistMezeru2Pix*/, m_nHeight-4, g_Scheme.GetColor(COL_HIGHLIGHT));
		}
	}

	// vykresli ohraniceni
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_SHADOW), g_Scheme.GetColor(COL_LIGHT));
}

void CProgressBar::Update(float deltaTime)
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
}