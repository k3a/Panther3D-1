//----------- Copyright © 2005-2008, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Base game class. It manages game module
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

/** Game information buffer to be filled by game calling GetGameInfo method. */
struct GameInfo
{
	/** Game/mod vendor */
	char szVendor[128];
	/** Game name */
	char szName[128];
	/** Game version as array of bytes, f.e. szVersion={1, 5, 97, 0}; equals 1.5.97.0 */
	char szVersion[4];
	/** Game information website. Must end with / */
	char szWebsite[128];
	/** Send statistics? \Remarks If set to true, browser windows will be opened on game exit. Url will be szWebsite?game=szName&version=x.x.x.x&time=TimePlayedInMinutes (x can be 1-3 chars).*/
	bool bSendStats;
	/** Open browser with ULR szWebsite?game=szName&version=x.x.x.x&time=TimePlayedInMinutes (where x is 1-3 chars) after game played? */
	bool bOpenBrowser;
};

/** Base game class interface. Every game module must implement this interface. */
class IP3DGame : public IP3DBaseInterface
{
public:
	/** When NOT in editor mode (bInEditor==false) you must call IP3DEngine::InitEngine() first and then get all required singletons and change their settings.  */
	virtual bool InitGame(const char* cmdLine, bool bInEditor)=0;
	/** Create game loop. In this loop you render scene end update necessary state. This isn't called by editor! */
	virtual void RunGame()=0;
	/** Retrieve game information. Every game must fill in GameInfo structure */
	virtual void GetGameInfo(OUT GameInfo &pGameInfo)=0;
};

#define IP3DGAME_GAME "P3DGame_1" // iface name