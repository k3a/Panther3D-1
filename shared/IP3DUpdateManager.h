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

#include "interface.h"

class IP3DUpdateManager : public IP3DBaseInterface
{
public:
	virtual int CheckForUpdates(const char* updateServer)=0; // vrati pocet updatu
	virtual int GetNumUpdates()const=0; // vrati pocet updatu (muselo byt ale predtim volano CheckForUpdates!)
	virtual bool DownloadUpdates()=0; // stahne updaty
	virtual float GetPercentDownloaded()const=0;
	virtual int GetNumDownloaded()const=0; // vrati pocet skutecne stazenych updatu. Pokud se shoduje s poctem updatu je to stazeno vsechno
};

#define IP3DENGINE_UPDATEMANAGER "P3DUpdateManager_1" // nazev ifacu