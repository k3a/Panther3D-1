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
#include "Frame.h"

#define TAB_HEI 22 // vyska zalozky

CTabStrip::CTabStrip()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_nCurrentTab=0;
	m_nLastTab=0;
	m_nTabs=0;
	m_bOnSelect=false;

	m_fActiveX1=0.0f;
	m_fActiveX2=0.0f;

	SetElementFlags(EF_LINE | EF_SPRITE); // kreslime line + text
}

CTabStrip::~CTabStrip()
{
	for(unsigned int i=0;i<m_nTabs;i++)
		SAFE_DELETE_ARRAY(m_strTab[i]); // uvolni retezec - popisek
}

bool CTabStrip::CreateTabStrip(int x, int y, int w, int h, IGuiElement* pParent)
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

void CTabStrip::RenderLine(float deltaTime)
{
	// vykresli ohraniceni
	//g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY+TAB_HEI, m_nWidth, m_nHeight-TAB_HEI, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_LIGHTSHADOW));
	
	// vykresli zalozky
	int nLastX=(int)(m_nPosX+m_nParentX);
	for(unsigned int n=0;n<m_nTabs;n++)
	{
		// vypocti sirku zalozky
		int wid;
		g_Scheme.GetFontLabel()->GetTextSize(m_strTab[n], NULL, &wid, NULL);
		wid+=10; // nejake volne misto na okrajich

		DrawTab(nLastX, wid, n);

		nLastX+=wid+2; // +mezera 2 pixely
	}

	if (m_fActiveX1 == m_nPosX+m_nParentX) // pokud je aktivni prvni
	{
		P3DVector2D v[5];
		v[0].x = (float)m_fActiveX1; v[0].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		v[1].x = (float)m_fActiveX1; v[1].y = (float)m_nPosY+m_nParentY+m_nHeight;
		g_p2D->Line_Draw(v, 2, g_Scheme.GetColor(COL_LIGHT));

		v[0].x = (float)m_fActiveX1; v[0].y = (float)m_nPosY+m_nParentY+m_nHeight;
		v[1].x = (float)m_nPosX+m_nParentX+m_nWidth; v[1].y = (float)m_nPosY+m_nParentY+m_nHeight;
		v[2].x = (float)m_nPosX+m_nParentX+m_nWidth; v[2].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		g_p2D->Line_Draw(v, 3, g_Scheme.GetColor(COL_LIGHTSHADOW));

		v[0].x = (float)m_nPosX+m_nParentX+m_nWidth; v[0].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		v[1].x = (float)m_fActiveX2; v[1].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		g_p2D->Line_Draw(v, 2, g_Scheme.GetColor(COL_LIGHT));

	}
	else // pokud je aktivni druha
	{
		P3DVector2D v[5];
		v[0].x = (float)m_fActiveX1; v[0].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		v[1].x = (float)m_nPosX+m_nParentX; v[1].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		v[2].x = (float)m_nPosX+m_nParentX; v[2].y = (float)m_nPosY+m_nParentY+m_nHeight;
		g_p2D->Line_Draw(v, 3 , g_Scheme.GetColor(COL_LIGHT));

		v[0].x = (float)m_nPosX+m_nParentX; v[0].y = (float)m_nPosY+m_nParentY+m_nHeight;
		v[1].x = (float)m_nPosX+m_nParentX+m_nWidth; v[1].y = (float)m_nPosY+m_nParentY+m_nHeight;
		v[2].x = (float)m_nPosX+m_nParentX+m_nWidth; v[2].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		g_p2D->Line_Draw(v, 3, g_Scheme.GetColor(COL_LIGHTSHADOW));

		v[0].x = (float)m_nPosX+m_nParentX+m_nWidth; v[0].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		v[1].x = (float)m_fActiveX2; v[1].y = (float)m_nPosY+m_nParentY+TAB_HEI;
		g_p2D->Line_Draw(v, 2, g_Scheme.GetColor(COL_LIGHT));
	}
}

void CTabStrip::RenderSprite(float deltaTime)
{
	// VYKRESLI TEXT ZALOZEK
	//g_Scheme.GetFontLabel()->DrawText(FA_LEFT, m_szCaption, (int)(m_nPosX+m_nParentX+8), (int)(m_nPosY+m_nParentY+1), (int)(m_nPosX+m_nParentX+m_nWidth-3), (int)(m_nPosY+m_nParentY+12), g_Scheme.GetColor(COL_TEXT));
	// vykresli zalozky
	int nLastX=(int)(m_nPosX+m_nParentX);
	for(unsigned int n=0;n<m_nTabs;n++)
	{
		// vypocti sirku zalozky
		int wid;
		g_Scheme.GetFontLabel()->GetTextSize(m_strTab[n], NULL, &wid, NULL);
		wid+=10; // nejake volne misto na okrajich

		if (n==m_nCurrentTab) 
			g_Scheme.GetFontLabel()->DrawText(FA_LEFT, m_strTab[n], nLastX+5, (int)(m_nPosY+m_nParentY+3), nLastX+wid, (int)(m_nPosY+m_nParentY+1)+TAB_HEI, g_Scheme.GetColor(COL_TEXT));
		else
			g_Scheme.GetFontLabel()->DrawText(FA_LEFT, m_strTab[n], nLastX+5, (int)(m_nPosY+m_nParentY+5), nLastX+wid, (int)(m_nPosY+m_nParentY+1)+TAB_HEI, g_Scheme.GetColor(COL_TEXT));

		nLastX+=wid+2; //mezera 2 pix
	}
}

void CTabStrip::AddTab(const char* szCaption, IGuiElement* pTabFrame)
{
	m_strTab[m_nTabs] = new char[strlen(szCaption)+1];
	strcpy(m_strTab[m_nTabs], szCaption);
	
	m_pTabFrame[m_nTabs] = pTabFrame;

	m_nTabs++;

	SelectTab(0); // udelame aktivni prvni
}

void CTabStrip::SelectTab(unsigned int i)
{
	if (i>=m_nTabs) i = m_nTabs-1; // pokud je vetsi nez nejvyssi index

	//projdi vsechny framy, skryj je a zobraz jen ten, ktery je potreba
	for(unsigned int n=0;n<m_nTabs;n++)
		m_pTabFrame[n]->Visible(false);

	m_pTabFrame[i]->Visible(true); // zobraz frame pridruzeny k zalozce

	m_nCurrentTab = i;
}

inline void CTabStrip::DrawTab(int x, int w, unsigned int nIndex)
{
	P3DVector2D v[4];

	float active = nIndex==m_nCurrentTab;

	//hned v renderu zjistime, zda tuto zalozku nekdo neaktivoval
	if(g_bLMouseDownFirstTime && !this->m_bHidden) // ale nesmi byt prvek skryt!
	{
		if(!active && g_nMouseX >= x && g_nMouseX <= x+w
			&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+(float)TAB_HEI-1)
		{
			active=true;
			m_nCurrentTab = nIndex;
			SelectTab(nIndex);
		}
	}

	// svetla cast
	v[0].x = (float)x; v[0].y = m_nPosY+m_nParentY+(float)TAB_HEI-1;
	v[1].x = (float)x; v[1].y = active? (float)m_nPosY+m_nParentY : (float)m_nPosY+m_nParentY+2;
	v[2].x = (float)x+w; v[2].y = active? (float)m_nPosY+m_nParentY : (float)m_nPosY+m_nParentY+2;
	g_p2D->Line_Draw(v, 3, g_Scheme.GetColor(COL_LIGHT));

	// jen jedna tmava cara
	g_p2D->Line_DrawSimple(x+w, active? m_nPosY+m_nParentY : m_nPosY+m_nParentY+2, x+w, m_nPosY+m_nParentY+TAB_HEI-2, g_Scheme.GetColor(COL_LIGHTSHADOW));

	//pokud je aktivni - cara pozadi
	//if(active) g_p2D->Line_DrawSimple((float)x+1, m_nPosY+m_nParentY+(float)TAB_HEI, (float)x+w-1, m_nPosY+m_nParentY+(float)TAB_HEI, g_Scheme.GetColor(COL_BG));

	if(active)
	{
		m_fActiveX1 = (float)x;
		m_fActiveX2 = (float)x+w;
	}
}

void CTabStrip::Update(float deltaTime)
{
	if (m_nLastTab != m_nCurrentTab)
		m_bOnSelect=true;
	else
		m_bOnSelect=false;

	m_nLastTab = m_nCurrentTab;
}