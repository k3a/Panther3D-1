//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Dialog Select File
//-----------------------------------------------------------------------------
#include "SelectFile.h"

static bool newPath=false;

CP3DGUI_SelectFile::CP3DGUI_SelectFile()
{
	CreateWindow(g_pLocaliz->Translate_const("#GUI_SelectFile"), 
		0, 0, 400, 250, P3DWS_CAPTION | P3DWS_CLOSEBTN);
	CenterScreen();
	SetModal(true);
	SetElementFlags(EF_NOT_EDITABLE);

	m_lstFolders.CreateListBox(10, 10, 185, 184, this);
	 m_lstFolders.SetElementFlags(EF_NOT_EDITABLE);
	
	m_lstFiles.CreateListBox(205, 10, 185, 184, this);
	 m_lstFiles.SetElementFlags(EF_NOT_EDITABLE);
	
	m_lblPath.CreateLabel("...\\Cesta\\Cesta\\cesta\\cesta", 10, 198, 380, 20, false, LA_SINGLELINE | LA_LEFT, this);
	 m_lblPath.SetElementFlags(EF_NOT_EDITABLE);
	m_txtFilename.CreateTextBox("pokus.dat", 10, 217, 185, 22, this);
	 m_txtFilename.SetElementFlags(EF_NOT_EDITABLE);
	
	m_btnCancel.CreateButton(g_pLocaliz->Translate_const("#GUI_Cancel"), 303, 216, 87, 24, this);
	 m_btnCancel.SetElementFlags(EF_NOT_EDITABLE);
	m_btnOK.CreateButton(g_pLocaliz->Translate_const("#GUI_Ok"), 205, 216, 87, 24, this);
	 m_btnOK.SetElementFlags(EF_NOT_EDITABLE);

	onOK = false;

	SetPath(NULL);
}

void CP3DGUI_SelectFile::Update()
{
	// tlacitka
	onOK = false;
	if (m_btnOK.OnPress())
	{
		onOK = true;
		Visible(false);
		return;
	}
	if (m_btnCancel.OnPress())
	{
		Visible(false);
		return;
	}
	
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
	if (m_lstFiles.OnSelect() || newPath)
	{
		m_txtFilename.SetText(m_lstFiles.GetItem(m_lstFiles.GetSelectedItem()));
		newPath = false;
	}
	else if (m_lstFiles.OnDblClick())  // vybran soubor + zavri dialog
	{
		onOK = true;
		Visible(false);
	}
}

bool CP3DGUI_SelectFile::SetPath(const char* path)
{
	if (!path || path[0]==0) // neni nastaven hlavn adresar
	{
		// nacti home adresar hry
		ConVar* home = g_pConsole->FindConVar("e_home_path");
		strcpy(m_szPath, home->GetString());
	}
	else // je nastaven zakl. adresar
	{
		// je nastaven relativni adresar?
		if (path[1]!=':')
		{
			// nacti home adresar hry
			ConVar* home = g_pConsole->FindConVar("e_home_path");
			strcpy(m_szPath, home->GetString());
			if (m_szPath[strlen(m_szPath)-1]!='/' && m_szPath[strlen(m_szPath)-1]!='\\') strcat(m_szPath, "\\"); // pridej lomitko jestli neni
			strcat(m_szPath, path);
		}
		else
			strcpy(m_szPath, path); // je absolutni cesta
	}
	
	if (m_szPath[strlen(m_szPath)-1]=='/' || m_szPath[strlen(m_szPath)-1]=='\\') m_szPath[strlen(m_szPath)-1] = 0; // odstran lomitko
	strcat(m_szPath, "\\*"); // pridej lomitko a *
	
	int nValid;
	HANDLE searchHandle=NULL;
	WIN32_FIND_DATA fileData;
	
	searchHandle = FindFirstFile(m_szPath, &fileData);
	nValid = (searchHandle == INVALID_HANDLE_VALUE) ? 0 : 1;
	
	if (!nValid) return false;
	
	// nova cesta, aktualizuj nazev souboru
	newPath = true;
	
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
	m_lblPath.SetCaption(m_szPath);
	
	return true;
}

bool CP3DGUI_SelectFile::GetPath(OUT char* path, int len)const
{
	char fname[MAX_PATH];
	strcpy(fname, m_szPath);
	strcat(fname, "\\");
	strcat(fname, m_txtFilename.GetText());
	if (len-1 < (int)strlen(fname))
	{
		CON(MSG_CON_ERR, "CP3DGUI_SelectFile::GetPath: Buffer is too small!");
		return false;
	}
	strcpy(path, fname);
	return true;
}