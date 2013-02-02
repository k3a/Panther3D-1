//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Main menu with logo
//-----------------------------------------------------------------------------
#pragma once

#include "IP3DFont.h"
#include "common.h"

class CMainMenu
{
public:
	CMainMenu(){m_nLastId=0;m_nSelItem=0;};
	void Shutdown();
	bool Initialize(); // vytvori textury a fonty, musi se volat!!!
	void Clear();
	void AddItem(const char* szInterText); // ID se pocita od 1 vcetne... max. 10 polozek
	void ChangeItem(UINT nId, const char* szInterText); // zmeni text (+localization) - id zacina od 1
	UINT GetSelected()const{return m_nSelItem;}; // 0 pokud nic nebylo zmacknuto, musi se volat az po vykresleni
	void Render();
private:
	UINT m_nLastId;
	char* m_szTexts[10]; // id je v poli o jedno mensi!
	UINT m_nSelItem;
	//
	IP3DFont* m_pItemText;
};