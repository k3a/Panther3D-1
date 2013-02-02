//----------- Copyright � 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Dialog Select File
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"

class CP3DGUI_SelectFile : public CWindow
{
public:
	CP3DGUI_SelectFile();
	void Update();
	bool SetPath(const char* path);
	bool GetPath(OUT char* path, int len)const; // vrati false pri malem vstupnim bufferu
	bool OnOK(){bool old=onOK;onOK=false; return old;};
private:
	CListBox m_lstFolders;
	CListBox m_lstFiles;
	CButton m_btnOK;
	CButton m_btnCancel;
	CLabel m_lblPath;
	CTextBox m_txtFilename;
	//
	char m_szPath[MAX_PATH];
	bool onOK;
};