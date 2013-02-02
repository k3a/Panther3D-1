#pragma once

#include "IP3DGame.h"

class CP3DGame : public IP3DGame
{
public:
	bool InitGame(const char* cmdLine, bool bInEditor);
	void RunGame();
	void GetGameInfo(OUT GameInfo &pGameInfo);
};