//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	ListBox se scrollbarem
//-----------------------------------------------------------------------------
#include "ListBox.h"

CListBox::CListBox()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;
	
	m_fCurrentTime=0.0f;
	m_fLastClickTime=0.0f;
	m_bDoubleClick=false;
	
	m_pScroll=NULL;

	Clear();
	
	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_LINE | EF_SPRITE); // kreslime line + text
}

CListBox::~CListBox()
{
	SAFE_DELETE(m_pScroll);
	for(UINT i=0; i<m_vItems.size();i++)
	{
		SAFE_DELETE_ARRAY(m_vItems[i]);
	}
};

bool CListBox::CreateListBox(int x, int y, int w, int h, IGuiElement* pParent)
{
	// nastav parentu jako child tento prvek
	pParent->SetChild(this);
	m_pParent = pParent;

	// nastav

	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;
	
	// posuvnicek
	m_pScroll = new CScrollBarVert(); // musi byt zde, kvuli parents
	m_pScroll->CreateScrollBar(m_nWidth-15, 0, 15, m_nHeight, this);
	m_pScroll->SetMinMax(0.0f, 100.0f);

	return true;
}

void CListBox::RenderLine(float deltaTime)
{
	// vykresli ohraniceni
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_LIGHTSHADOW), g_Scheme.GetColor(COL_LIGHT));

	// zvyrazni vybranou polozku
	int h = g_Scheme._pLabelFont->GetLineH();
	int numLines = ((m_nHeight-2)-((m_nHeight-2)%h)) / h; // zaokrouhlit dolu - zjistime pocet radku, ktere se tam 100% vejdou 

	int LineOffset = (int)((m_vItems.size()-numLines)/100.0f*m_pScroll->GetValue());

	if (m_vItems.capacity() <= (UINT)numLines) LineOffset=0; // pokud neni treba posun (vejde vse)

	int allH=1; int n=0; int numItems=0;
	for(vector<char*>::iterator i=m_vItems.begin(); i != m_vItems.end(); ++i, n++)
	{
		if (n < LineOffset) continue; // posun
		if (numItems < numLines)
		{
			if (LineOffset+numItems+1 == selItem) // je to vybrana polozka, tak ji zvyrazni
			{
				g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+2, m_nPosY+m_nParentY+allH, m_nWidth-2-15, h, g_Scheme.GetColor(COL_HIGHLIGHT));
				break;
			}
			
			allH+=h;
			numItems++;
		}
	}
}
void CListBox::RenderSprite(float deltaTime)
{
	// VYKRESLI TEXT + ZJISTI ZDA JE MYS NAD POLOZKOU A ZPRACUJ
	m_fCurrentTime+=deltaTime;

	int h = g_Scheme._pLabelFont->GetLineH();
	int numLines = ((m_nHeight-2)-((m_nHeight-2)%h)) / h; // zaokrouhlit dolu - zjistime pocet radku, ktere se tam 100% vejdou 

	int LineOffset = (int)((m_vItems.size()-numLines)/100.0f*m_pScroll->GetValue());

	if (m_vItems.size() <= (UINT)numLines) LineOffset=0; // pokud neni treba posun (vejde vse)

	int allH=1; int n=0; int numItems=0;
	for(vector<char*>::iterator i=m_vItems.begin(); i != m_vItems.end(); ++i, n++)
	{
		if (n < LineOffset) continue; // posun
		if (numItems < numLines)
		{
			if (g_nMouseX > m_nPosX+m_nParentX+1 && g_nMouseX < m_nPosX+m_nParentX+m_nWidth-2-15
			&& g_nMouseY > m_nPosY+m_nParentY+allH && g_nMouseY < m_nPosY+m_nParentY+allH+h)
			{
				bool customIF=true;
				// zjisti zda to neni v editoru a zda okno na kterem je ma focus
				if (CVgui_editor.GetBool() && !(GetElementFlags() & EF_NOT_EDITABLE))
					customIF = false;
				if (customIF && this->m_pParent)
				{
					customIF = false;
					if (this->m_pParent->m_pParent)
					{
						if (this->m_pParent->m_pParent == GetFirstVisibleWindow())
							customIF = true;
					}
					else
						if (this->m_pParent == GetFirstVisibleWindow())
							customIF = true;
				}
					
				// mys je nad touto polozkou
				if (g_bLMouseDownFirstTime && customIF)
				{
					selItem = LineOffset+numItems+1;
					m_bNowSelected = true;
					
					if (m_fCurrentTime-m_fLastClickTime < 0.3f) { // dvojklik - 300ms bude asi ok
						m_bDoubleClick=true; 
						m_fLastClickTime=0.0f;
					} 
					else // obycejny klik
						m_fLastClickTime = m_fCurrentTime;
				}
			}
			
			if (*i) g_Scheme._pLabelFont->DrawText(FA_LEFT, const_cast<const char*>(*i), m_nPosX+m_nParentX+2, m_nPosY+m_nParentY+allH, m_nPosX+m_nParentX+m_nWidth-2-15, m_nPosY+m_nParentY+m_nHeight, g_Scheme.GetColor(COL_TEXT));
			allH+=h;
			numItems++;
		}
	}
}

void CListBox::Update(float deltaTime)
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
	
	//m_bNowSelected=false;
	//m_bDoubleClick=false;
}

void CListBox::AddItem(const char* pszItemText)
{
	char* tmp = new char[strlen(pszItemText)+1];
	strcpy(tmp, pszItemText);
	m_vItems.push_back(tmp);
	
}

void CListBox::Clear()
{
	for(vector<char*>::iterator i=m_vItems.begin(); i != m_vItems.end(); ++i)
	{
//		if (i) delete *i;		// FIXME: ???
	}

	m_vItems.clear();

	selItem=1;
	if (m_pScroll) m_pScroll->SetValue(0.0f);
}