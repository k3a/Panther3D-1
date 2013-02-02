//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	TextBox
//-----------------------------------------------------------------------------
#include "TextBox.h"

CTextBox::CTextBox()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	m_nElementID = 2; // textBox

	lineTime = 0;
	m_fCursorX = 0.0f;
	m_nFirstSelChar = 0;
	m_nLastSelChar = 0;
	m_nCurrChar = 0;

	m_fCursorX=0.0f;
	m_fCursorStartX=0.0f;
	m_fCursorEndX=0.0f;

	m_nCurrTextWidth = 0;

	m_strText.assign("TextBox");
	//m_strText.insert(2, 1, 'Q');
	//m_strText.replace(2, 4, 1, 'X');
	
	// GUI EDITOR
	edit_mode=0;

	SetElementFlags(EF_LINE | EF_SPRITE); // kreslime line
}

bool CTextBox::CreateTextBox(const char* szText, int x, int y, int w, int h, IGuiElement* pParent)
{
	// nastav parentu jako child tento prvek
	pParent->SetChild(this);
	m_pParent = pParent;

	// nastav
	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;

	if(szText) m_strText.assign(szText);

	return true;
}

void CTextBox::RenderLine(float deltaTime)
{
	// vykresli pozadi
	g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_LIGHTSHADOW));

	// vykresli ohraniceni
	g_p2D->Line_Draw3DRect(m_nPosX+m_nParentX, m_nPosY+m_nParentY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_SHADOW), g_Scheme.GetColor(COL_LIGHT));

	// vykresli oznaceni textu - vyber
	if(m_bHasFocus) g_p2D->Line_DrawFilledRect(m_nPosX+m_nParentX+2+(int)m_fCursorStartX, m_nPosY+m_nParentY+2, (int)m_fCursorEndX-(int)m_fCursorStartX, m_nHeight-4, g_Scheme.GetColor(COL_HIGHLIGHT));
	
	// vykresli kurzor + blikani
	if(m_bHasFocus && lineTime > 0)
	{
		g_p2D->Line_DrawSimple(m_nPosX+m_nParentX+2+(int)m_fCursorX, m_nPosY+m_nParentY+2, m_nPosX+m_nParentX+2+(int)m_fCursorX, m_nPosY+m_nParentY+m_nHeight-4, g_Scheme.GetColor(COL_TEXT));
	}
}

void CTextBox::RenderSprite(float deltaTime)
{
	int hei;
	g_Scheme.GetFontLabel()->GetTextSize(m_strText.c_str(), NULL, NULL, &hei);
	g_Scheme.GetFontLabel()->DrawText(FA_SINGLELINE, m_strText.c_str(), (int)(m_nPosX+m_nParentX)+2, (int)(m_nPosY+m_nParentY+m_nHeight/2-hei/2), (int)(m_nPosX+m_nParentX+m_nWidth)-3, (int)(m_nPosY+m_nParentY+m_nHeight)-2, g_Scheme.GetColor(COL_TEXT));
}

void CTextBox::Update(float deltaTime)
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

	unsigned short chrChar = g_pInput->GetKeyChar();

	lineTime += deltaTime;
	if (lineTime > 0.5f) lineTime=-0.5f;

	// zjisti sirku textu
	g_Scheme.GetFontLabel()->GetTextSize(m_strText.c_str(), NULL, &m_nCurrTextWidth, NULL);

	// zjisti aktualni pozici kurzoru
	int wid, lastWid=0;
	if (g_bLMouseDown)
	{
		// postupne projdi x velikosti znaku textu
		for(unsigned int i=1;i<=m_strText.length();i++)
		{
			if (g_nMouseX < m_nPosX+m_nParentX+2) // pokud je jeste pred polickem - specialni pripad
			{
				m_fCursorX = 0.0f;
				m_nCurrChar = 0;
				break;
			}

			if(m_strText.length()==i) // je to posledni pismeno
			{
				g_Scheme.GetFontLabel()->GetTextSize(m_strText.c_str(), NULL, &wid, NULL);
				if(m_strText.length()==1) // pokud mame jen jeden znak
				{
					if(g_nMouseX < m_nPosX+m_nParentX+2+wid/2) // zjistime zda je kurzor pred
					{
						m_fCursorX = 0;
						m_nCurrChar = 0;
					}
					else // nebo za znakem
					{
						m_fCursorX = (float)wid;
						m_nCurrChar = 1;
					}
				}
				else // pokud mame vic znaku, jde o konec
				{
					m_fCursorX = (float)wid;
					m_nCurrChar = i;
				}
				break;
			}

			// ziskej x
			string tmpStr = m_strText.substr(0, i);
			g_Scheme.GetFontLabel()->GetTextSize(tmpStr.c_str(), NULL, &wid, NULL);
			if(m_nPosX+m_nParentX+2+wid>=g_nMouseX && m_nPosX+m_nParentX+2+lastWid<=g_nMouseX)
			{
				if(g_nMouseX < m_nPosX+m_nParentX+2+lastWid+(wid-lastWid)/2)
				{
					m_fCursorX = (float)lastWid;
					m_nCurrChar = i-1;
				}
				else
				{
					m_fCursorX = (float)wid;
					m_nCurrChar = i;
				}
				break;
			}
			lastWid = wid;
		}
	}

	if(g_nMouseX >= m_nPosX+m_nParentX && g_nMouseX <= m_nPosX+m_nParentX+m_nWidth
		&& g_nMouseY >= m_nPosY+m_nParentY && g_nMouseY <= m_nPosY+m_nParentY+m_nHeight)
	{
		if (g_bLMouseDownFirstTime && !m_bHasFocus) // pokud nema focus, tak ho nastav a jinym vem
		{
			// oznac vsechny jine jako neaktivni - TODO: ne jen s jednim parentem, ale vice, az do CWindow, ktery nema parent
			// TODO: UDELAT NA TO NOVOU METODU PRIMO DO GUISYSTEMU, ABY SE PROSLI VSECHNY PRVKY A NASTAVILY SE BEZ FOCUSU!!!
			for(unsigned int i=0;i<m_pParent->m_nChilds;i++)
			{
				if(m_pParent->m_pChilds[i]->m_nElementID == 2) // je to TextBox
				{
					// pokud neni skryt
					if(!m_pParent->m_pChilds[i]->m_bHidden)
					{
						CTextBox* pOpt = (CTextBox*)m_pParent->m_pChilds[i];
						pOpt->m_bHasFocus=false; // nastav jako bez ohniska
					}
				}
			}
			// oznac tento jako aktivni
			m_bHasFocus = true;
			lineTime = 0;
		}
		// zjisti pocatecni pismeno(vcetne)-index a pocatecni pozici
		if(g_bLMouseDownFirstTime)
		{
			m_nFirstSelChar = m_nCurrChar;
			m_nLastSelChar = -1;
			m_fCursorStartX = m_fCursorX;
		}
	}
	else // FIXME: Docasne, pokud nebude jinak vyresen focus
	{
		if (g_bLMouseDownFirstTime && m_bHasFocus) m_bHasFocus=false;
	}


	if (!m_bHasFocus) return;

	if(g_bLMouseDown) m_fCursorEndX = m_fCursorX;
	// zjisti konecne pismeno(vcetne)-index a pocatecni pozici
	if(g_bLMouseUpFirstTime)
	{
		m_nLastSelChar = m_nCurrChar;
		m_fCursorEndX = m_fCursorX;
	}

	// zjisti ktery je mensi a prohod - dulezite
	if (m_nLastSelChar>-1)
	{
		if(m_nLastSelChar < m_nFirstSelChar)
		{
			int tmpN = m_nLastSelChar;
			m_nLastSelChar = m_nFirstSelChar;
			m_nFirstSelChar = tmpN;
		}
	}

	// JE POTREBA SMAZAT TEXT?
	if (g_pInput->IsKeyDownFirstTime(DIK_DELETE) && m_nLastSelChar == m_nFirstSelChar) // koncovy je pocatecni - odebirame znak
	{
		m_strText.erase(m_nCurrChar, 1);
	}
	else if (m_nLastSelChar > -1 && (g_pInput->IsKeyDownFirstTime(DIK_DELETE) || chrChar == 8)) // mame vyber - odebirame znaky
	{
		m_strText.erase(m_nFirstSelChar, m_nLastSelChar-m_nFirstSelChar);
		// odebrat vyber + nove pozice
		m_nLastSelChar = -1;
		m_fCursorX = min(m_fCursorStartX,m_fCursorEndX);
		m_fCursorStartX = m_fCursorX;
		m_fCursorEndX = m_fCursorStartX;
		m_nCurrChar = m_nFirstSelChar;
		m_nLastSelChar = m_nFirstSelChar;
	}

	// UPRAV TEXT POMOCI INPUTU - MUSI BYT POSLEDNI V TETO METODE KVULI RETURN;
	if (chrChar!=0) // pokud mame nejaky znak
	{
		// dopredu si zjistime velikost pridavaneho znaku
		int wid; char cc[2];
		cc[0] = (char)chrChar; cc[1]=0;
		g_Scheme.GetFontLabel()->GetTextSize(cc, NULL, &wid, NULL);

		if(m_nLastSelChar == m_nFirstSelChar && chrChar == 8) // backspace - vyjimka
		{
			if (m_nCurrChar==0) return; // nelze umazat vice nez vsechno :)
			int widDelChr; // velikost mazaneho znaku
			char ccDel[2];
			ccDel[0] = m_strText[m_nCurrChar-1]; ccDel[1]=0;
			g_Scheme.GetFontLabel()->GetTextSize(ccDel, NULL, &widDelChr, NULL);
			m_strText.erase(m_nCurrChar-1, 1);
			m_fCursorX -= widDelChr;
			m_fCursorStartX = m_fCursorX;
			m_fCursorEndX = m_fCursorStartX;
			m_nCurrChar--; // odebirame novy znak
			m_nFirstSelChar = m_nCurrChar;
			m_nLastSelChar = m_nFirstSelChar;
			return;
		}

		if (m_nCurrTextWidth+wid-abs(m_fCursorEndX-m_fCursorStartX) >= m_nWidth-4) return; // nelze pridat dalsi text

		if (m_nLastSelChar == m_nFirstSelChar) // koncovy je pocatecni - pridavame znak
		{
			m_strText.insert(m_nCurrChar, 1, (char)chrChar);
			// uprav pozice
			m_fCursorX += wid;
			m_fCursorStartX = m_fCursorX;
			m_fCursorEndX = m_fCursorStartX;
			m_nCurrChar++; // pridavame novy znak
			m_nFirstSelChar = m_nCurrChar;
			m_nLastSelChar = m_nFirstSelChar;
		}
		else if (m_nLastSelChar > -1) // mame vyber - prepisujeme znak
		{
			m_strText.replace(m_nFirstSelChar, m_nLastSelChar-m_nFirstSelChar, 1, (char)chrChar);
			// odebrat vyber + nove pozice
			m_nLastSelChar = -1;
			m_fCursorX = min(m_fCursorStartX,m_fCursorEndX)+wid;
			m_fCursorStartX = m_fCursorX;
			m_fCursorEndX = m_fCursorStartX;
			m_nCurrChar = m_nFirstSelChar+1;
			m_nLastSelChar = m_nFirstSelChar;
		}
	}
}

void CTextBox::SetText(const char* szText)
{
	m_fCursorX = 0.0f;
	m_nFirstSelChar = 0;
	m_nLastSelChar = 0;
	m_nCurrChar = 0;

	m_fCursorX=0.0f;
	m_fCursorStartX=0.0f;
	m_fCursorEndX=0.0f;

	m_strText.assign(szText);
}