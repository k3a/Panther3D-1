//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Update manager - can find new updates and download them
//-----------------------------------------------------------------------------
#pragma once

#include "IP3DUpdateManager.h"
#include <vector>
using namespace std;

class P3DUpdateManager : public IP3DUpdateManager
{
public:
	P3DUpdateManager();
	~P3DUpdateManager();
	bool Init(){return true;};
	int CheckForUpdates(const char* updateServer); // vrati pocet updatu
	int GetNumUpdates()const{return m_nUpdates;}; // vrati pocet updatu (muselo byt ale predtim volano CheckForUpdates!)
	bool DownloadUpdates(); // stahne updaty
	float GetPercentDownloaded()const{ return m_fPercentDownloaded; };
	int GetNumDownloaded()const{ return m_nDownloaded; }; // vrati pocet skutecne stazenych updatu. Pokud se shoduje s poctem updatu je to stazeno vsechno
private:
	static unsigned long DownloadThread(void* lpParam);
	//
	static vector<char*> m_pUpdLinks;
	static unsigned int m_nUpdates;
	// stav
	static float m_fPercentDownloaded;
	static unsigned int m_nDownloaded;
};