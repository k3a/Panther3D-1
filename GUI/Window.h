//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Window
//-----------------------------------------------------------------------------
#pragma once
#pragma warning (disable:4005)

#include "common.h"
#include "IElement.h"

#define TITLE_HEIGHT 20

#define P3DWS_HIDDEN 1
#define P3DWS_CAPTION 2
#define	P3DWS_CLOSEBTN 4

#define CreateWindow CreateWindow

class CWindow : public IGuiElement
{
public:
	CWindow();
	~CWindow(){ if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption);  };
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	void Update(float deltaTime);
	bool CreateWindow(const char* szCaption, int x, int y, int w, int h, int nStyle);
	void CenterScreen();
	void SetCaption(const char* szCaption){
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	};
	void SetModal(bool bModal=true){m_bModal=bModal;}; // da se nastavit jak v konstruktoru konkretniho okna tak za behu
	bool IsModal()const{return m_bModal;};
	void SetStyle(int nStyle){m_nStyle=nStyle;};
	bool OnClose(bool bDontClose){
		if(m_bHidden) 
			if(bDontClose) 
				m_bHidden = false;
	};
	void Show(bool bForeground=true);
	bool LoadFromFile(const char* guiFileFromHomePath); // nacte pozice a sirky !pojmenovanych! prvku ze souboru
private:
	char* m_szCaption;
	int m_nStyle;
	// posouvani oknem
	int relX; // je-li -1, neposouva se, je-li 0 nebo vetsi, je to rel pozice od leveho horniho rohu okna vcetne titulku
	int relY;
	// tlacitko X
	bool m_bXDownFirstTime;
	bool m_bXDown;
	bool m_bModal;
};