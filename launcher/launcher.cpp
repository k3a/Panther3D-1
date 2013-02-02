#include "IP3dGame.h"
#include "symbols.h"
#include <wininet.h>

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hi, LPSTR lpCmdLine, INT iMain)
{
	// memory leaks detection
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// NACTI DLL MODULY Z ADRESARE VE KTEREM JE EXE
	char szExePath[MAX_PATH];
	GetModuleFileName(hInst, szExePath, MAX_PATH);
	for(int i=(int)strlen(szExePath); i > 0; i--)
	{
		if (szExePath[i]=='/' || szExePath[i]=='\\') 
		{
			szExePath[i]=0;
			break;
		}
	}
	// ---------------------------------
	I_Initialize(szExePath); // initialize dll module system
	// ---------------------------------

	// get game module name
	char szGame[64]="main";
	for(unsigned int i=0;i<=strlen(lpCmdLine);i++)
	{
		if (!strnicmp(&lpCmdLine[i], "-game ", 6))
		{
			for(unsigned int ii=i+6;ii<=strlen(lpCmdLine);ii++)
			{
				if (lpCmdLine[i]==' ' || lpCmdLine[i]==0)
				{
					strncpy(szGame, &lpCmdLine[i+6], ii-i);
					break;
				}
			}
			break;
		}
	}
	strcat(szGame, ".game");

	// Load game module
	// ---------------------------------
	I_LoadNewModule(szGame); // load game module
	IP3DGame *pGame = (IP3DGame *)I_GetClass(IP3DGAME_GAME);
	if (!pGame)
	{
		MessageBox(0, "Can't load main game singleton!", "CAN'T LAUNCH GAME!", MB_ICONSTOP | MB_SYSTEMMODAL);
		I_Shutdown(); // shutdown module dll system
		return -1;
	}

	// Run game
	// ---------------------------------
	DWORD nMinPlayed=0;
	if (pGame->InitGame(lpCmdLine, false))
	{
		DWORD tickStart = GetTickCount();
		pGame->RunGame();
		nMinPlayed = (GetTickCount()-tickStart)/60000;
	}

	// post-run actions :-P
	if (!IsDebuggerPresent())
	{
		// get game information
		GameInfo game;
		pGame->GetGameInfo(game);

		// send statistics / show browser window when website is available
		if (game.szWebsite && game.szWebsite[0]!=0)
		{
			// create url
			char url[512];
			sprintf(url, "%s?game=%s&version=%d.%d.%d.%d&time=%d", game.szWebsite, game.szName, game.szVersion[0],
				game.szVersion[1], game.szVersion[2], game.szVersion[3], nMinPlayed);

			// send statistics
			if (game.bSendStats)
			{
				HINTERNET hInet = InternetOpen("P3DStats", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
				HINTERNET hFile = InternetOpenUrl(hInet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
				if (hInet && hFile) 
				{
					DWORD size;
					char dummy[16];
					if (InternetReadFile(hFile, dummy, 8, &size))
					InternetCloseHandle(hFile);
					InternetCloseHandle(hInet);
				}
			}
			// open browser window with game information
			if (game.bOpenBrowser)
			{
				ShellExecute(0, "open", url, "", "", 1);
			}
		}
	}

	// Shutdown
	// ----------------------------------
	I_Shutdown(); // shutdown module dll system

	return 0;
}