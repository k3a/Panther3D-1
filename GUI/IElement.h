//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI Element (Component)
//-----------------------------------------------------------------------------
#pragma once

#define EF_LINE			(1 << 0)
#define EF_SPRITE		(1 << 1)
#define EF_CUSTOM		(1 << 2)
#define EF_NOT_EDITABLE (1 << 3) // bude-li nastaven tento flag, nebude mozne editovat prvek a i v editor modu bude fungovat normalne

#define MAX_CHILDS 50 //WARN: nejvice 50 deti!

enum renderType_e
{
	RT_CUSTOM=0,
	RT_LINE,
	RT_SPRITE
};

class IGuiElement
{
public:
	IGuiElement(){m_nElementFlags=0;m_szName=0; m_nHeaderHeight=0; m_nElementID=0; m_pParent=0; m_nChilds=0; m_nParentX=0; m_nParentY=0; m_bHasFocus=false; m_bHidden=false;};
	virtual ~IGuiElement(){};
	virtual void RenderLine(float deltaTime){}; // vola se vzdy - vola se 2.
	virtual void RenderSprite(float deltaTime){}; // vola se vzdy - vola se 3.
	virtual void RenderCustom(float deltaTime){}; // vola se vzdy - vola se 1.
	virtual void Update(float deltaTime){}; // vola se jen v pripade, ze je okno na kterem je prvek aktivni - ma focus
	virtual void Update(){}; // virtualni metoda bude nahrazena v konkretnim okne, vola ji GUISystem
	virtual void Render(){}; // virtualni metoda bude nahrazena v konkretnim okne, vola ji GUISystem, musi si sam nastavit vse potrebne pro kresleni, line_begin() a tak
	void UpdateChildren(float deltaTime);
	void RenderChildren(float deltaTime, renderType_e type);
	void SetElementFlags(int flags){m_nElementFlags|=flags;};
	int  GetElementFlags()const{return m_nElementFlags;};
	void SetChild(IGuiElement* pChild){
		if (m_nChilds==MAX_CHILDS) return;
		m_pChilds[m_nChilds]=pChild; 
		m_nChilds++;
	};
	void SetSize(int x, int y){m_nWidth=x; m_nHeight=y;};
	void SetPosition(int x, int y){m_nPosX=x; m_nPosY=y;};
	void SetParentPosition(int x, int y){m_nParentX=x;m_nParentY=y;}; // absolutni pozice rodice!
	void Visible(bool bVisible){m_bHidden=!bVisible;};
public:
	IGuiElement* m_pParent;
	IGuiElement* m_pChilds[MAX_CHILDS];
	unsigned int m_nChilds;
	unsigned char m_nElementID; // Nastavuje konkretni prvek. Nemusi byt nastaveno, nastavi jen prvky jako Option, aby se dalo zjistit, ktery prvek je Option
	//
	int m_nWidth;
	int m_nHeight;
	int m_nHeaderHeight; // vyska titulku okna
	int m_nPosX;
	int m_nPosY;
	int m_nParentX;
	int m_nParentY;
	bool  m_bHasFocus;
	//
	bool m_bHidden; // je-li true, neprochazet deti - ani nevykreslovat ani neupdatovat
	const char* m_szName; // muze byt NULL
private:
	int m_nElementFlags;
};

#define INIT_ELEMENT(uniqueName) uniqueName .m_szName = #uniqueName