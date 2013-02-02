//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (2.8.2006)
// Purpose:	Emitter properties
//-----------------------------------------------------------------------------
#include "Emitter.h"

CP3DGUI_Emitter::CP3DGUI_Emitter()
{
	char title[30];
	sprintf(title, "%s 1", g_pLocaliz->Translate_const("#GUI_PEditEmitter") );
	
	CreateWindow(title, 0, 0, 400, 250, P3DWS_CAPTION | P3DWS_CLOSEBTN);
	CenterScreen();
	SetModal();

	m_lstFolders.CreateListBox(10, 10, 185, 184, this);
	m_lstFiles.CreateListBox(205, 10, 185, 184, this);

	m_lblPath.CreateLabel("...\\Cesta\\Cesta\\cesta\\cesta", 10, 198, 380, 20, false, LA_SINGLELINE | LA_RIGHT, this);
	m_txtFilename.CreateTextBox("pokus.dat", 10, 217, 185, 22, this);

	m_btnCancel.CreateButton(g_pLocaliz->Translate_const("#GUI_Cancel"), 303, 216, 87, 24, this);
	m_btnOK.CreateButton(g_pLocaliz->Translate_const("#GUI_Ok"), 205, 216, 87, 24, this);

	onOK = false;

	SetPath("C:\\");
}

void CP3DGUI_Emitter::Update()
{
	// tlacitka
	onOK = false;
	if (m_btnOK.OnPress())
	{
		// sestav cestu
		strcat(m_szPath, m_txtFilename.GetText());
		onOK = true;
	}
	if (m_btnCancel.OnPress())
		m_bHidden = true;

	// vybran adresar
	if (m_lstFolders.OnDblClick())
	{
		char* folder = m_lstFolders.GetItem(m_lstFolders.GetSelectedItem());
		if (!strcmp(folder, "..")) // o uroven nahoru
		{
			// odstran posledni slozku
			for(int i=strlen(m_szPath)-2;i>0;i--)
			{
				if (m_szPath[i]=='\\')
				{
					m_szPath[i] = 0;
					break;
				}
			}
			SetPath(m_szPath);
			return; // neni to obycejna slozka
		}

		if (!folder) return;
		// vybrana slozka
		strcat(m_szPath, folder); // uz ma lomitko, tak lze jen strcat
		SetPath(m_szPath);
	}

	// vybran soubor - jedno kliknuti
	if (m_lstFiles.OnSelect())
		m_txtFilename.SetText(m_lstFiles.GetItem(m_lstFiles.GetSelectedItem()));
	else if (m_lstFiles.OnDblClick())  // vybran soubor + zavri dialog
	{
		m_txtFilename.SetText(m_lstFiles.GetItem(m_lstFiles.GetSelectedItem()));
		// sestav cestu + zavrit dialog
		strcat(m_szPath, m_txtFilename.GetText());
		onOK = true;
	}

}

bool CP3DGUI_Emitter::SetPath(const char* path)
{
	strcpy(m_szPath, path);
	if (m_szPath[strlen(path)-1]=='/' || m_szPath[strlen(path)-1]=='\\') m_szPath[strlen(path)-1] = 0; // odstran lomitko
	strcat(m_szPath, "\\*"); // pridej lomitko a *

	int nValid;
	HANDLE searchHandle=NULL;
	WIN32_FIND_DATA fileData;

	searchHandle = FindFirstFile(m_szPath, &fileData);
	nValid = (searchHandle == INVALID_HANDLE_VALUE) ? 0 : 1;

	if (!nValid) return false;

	m_lstFolders.Clear();
	m_lstFiles.Clear();
	if (strlen(m_szPath)>4) m_lstFolders.AddItem(".."); // pokud nejsme v koreni disku, zobraz moznost prechodu naahoru

	while (nValid)
	{
		if (strcmpi (fileData.cFileName, ".") != 0 && strcmpi (fileData.cFileName, "..") != 0)
		{
			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // adresar
				m_lstFolders.AddItem(fileData.cFileName);
			else
				m_lstFiles.AddItem(fileData.cFileName);
		}
		nValid = FindNextFile(searchHandle, &fileData);
	}
	FindClose(searchHandle);

	// nastav tuto cestu
	m_szPath[strlen(m_szPath)-1]=0;
	m_lblPath.SetCaption(path);

	return true;
}