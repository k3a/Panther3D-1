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
#pragma once

#include "common.h"
#include "IElement.h"
#include <string>

using namespace std; // pro string

class CTextBox : public IGuiElement
{
public:
	CTextBox();
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	bool CreateTextBox(const char* szText, int x, int y, int w, int h, IGuiElement* pParent);
	void Update(float deltaTime);
	void SetText(const char* szText);
	const char* GetText()const{ return m_strText.c_str();};
private:
	string m_strText;
	//
	float lineTime; // blikani kurzoru
	float m_fCursorX;
	float m_fCursorStartX;
	float m_fCursorEndX;
	//
	int m_nCurrChar; // znak, ZA kterym je akt. kurzor
	int m_nFirstSelChar; // znak ZA kterym byl kurzor pri zacatku vyberu
	int m_nLastSelChar; // znak ZA kterym byl kurzor pri zacatku vyberu
	//
	int m_nCurrTextWidth; // aktualni sirka textu - TextBox je jen jednoradkovy a nesmi presahnout svou sirku -4
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};