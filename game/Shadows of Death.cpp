//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	General game cpp (window, ...)
//-----------------------------------------------------------------------------

#include "exception.h"

#include "Shadows of Death.h"
#include "common.h" // game common variables and functions
#include "entitymgr.h"
#include "msgbox.h"
#include "collisiongroups.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
REGISTER_SINGLE_CLASS(CP3DGame, IP3DGAME_GAME);
bool CP3DGame::InitGame(const char* cmdLine, bool bInEditor)
{
	// memory leaks detection
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(178);

	// every module should be registered for statistic purposes
	I_RegisterModule("main.game");

	// editor will init engine on its own, but if not in editor mode, initialize it
	if (!bInEditor) 
	{
		g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
		if (!g_pEngine || !g_pEngine->InitEngine(cmdLine, false)) { 
			MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't initialize engine!"); 
			return false;
		}
	}

	// GET GLOBAL SINGLETONS / it is not needed to get all, cause entities
	// will get this again in ctor, but why not also here ;)
	//////////////////////////////////////////////////////////////////////////
	// base singletons

	// in case of editor-mode, get engine class, it is the only really needed :)
	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);

	// XML_loader
	g_pXml = (IP3DXML*)I_GetClass(IP3DENGINE_XML);
	if(!g_pXml)
	{
		MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't load XML class!"); 
		return false;
	}

	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);

	// bsp_map_loader
	g_pBSPMap = (IP3DBSPMap*)I_GetClass(IP3DRENDERER_BSPMAP);

	// get global 2D drawing class
	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);

	// INIT PHYSICS MODULE
	g_pPhysEngine = (IP3DPhysEngine*)I_GetClass(IP3DPHYS_PHYSENGINE);
	g_pCharCtrl = (IP3DPhysCharacter*)I_GetClass(IP3DPHYS_CHARACTER);
	SetCollisonGroups(); // define collision groups

	// init input
	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);
	if (!g_pInput) 
	{ MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't create input device!"); return false;}

	g_pSkyBox = (IP3DSky*)I_GetClass(IP3DRENDERER_SKY);
	if (g_pSkyBox == NULL)
	{ MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't initialize skybox class!"); return false;}

	// init sound
	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
	if (!g_pSoundMan){MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't initialize SoundManager!"); return false;}

	// entity manager
	g_pEntMgr = (IP3DEntityMgr*)I_GetClass(IP3DGAME_ENTITYMGR); // nacte z tohodle dll ;) ale singleton... ufff...

	// init sound container
	g_pSoundContainer = (IP3DSoundContainer*)I_GetClass(IP3DSOUND_SOUNDCONTAINER);  // FIXME: Why it isn't singleton???
	if (!g_pSoundContainer) { MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't get g_pSoundContainer!"); return false;}

	// získej globální konzolu
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	g_pConsole->RegisterLastConVar(g_pLastConVar);
	// get cvars, advantage of it is that when it gets changed, you will get new values
	CVr_width = g_pConsole->FindConVar("r_width");
	CVr_height = g_pConsole->FindConVar("r_height");
	CVr_windowed = g_pConsole->FindConVar("r_windowed");

	// dynamicke svetlo
	g_pLightManager = (IP3DLightManager*)I_GetClass(IP3DRENDERER_LIGHTMANAGER);
	if (!g_pLightManager) { MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't get g_pLightManager!"); return false;}

	// Alpha manager
	g_pAlphaMan = (IP3DAlphaManager*)I_GetClass(IP3DRENDERER_ALPHAMANAGER);
	if (!g_pAlphaMan) { MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't get g_pAlphaMan!"); return false;}

	g_pMeshLoader = (IP3DMeshLoader*)I_GetClass(IP3DRENDERER_MESHLOADER);
	if (!g_pMeshLoader) { MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't get g_pMeshLoader!"); return false;}

	g_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM);
	if (!g_pFrustum) { MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't get g_pFrustum!"); return false;}

	g_pTimer = (IP3DTimer*)I_GetClass(IP3DENGINE_TIMER);
	if (!g_pTimer) { MsgBox(0, MB_ICONSTOP | MB_SYSTEMMODAL, "Error!", "Can't get g_pTimer!"); return false;}

	g_pLocaliz = (IP3DLocalization*)I_GetClass(IP3DENGINE_LOCALIZATION);
	return true;
}
//-----------------------------------------------------------------------------
void CP3DGame::RunGame()
{
	bool bRunning = true;
	while( bRunning )
	{
		// DRAW AND UPDATE GAME
		eEngineState engState = g_pEngine->RenderScene();
		if (engState==ES_QUITTING) bRunning = false;
		// ---------------------
		g_pEngine->UpdateScene();
	}
	SAFE_DELETE(g_pSoundContainer); // FIXME: Why it isn't singleton???
}
//-----------------------------------------------------------------------------
void CP3DGame::GetGameInfo(OUT GameInfo &pGameInfo)
{
	strcpy(pGameInfo.szVendor, "Reversity Studios");
	strcpy(pGameInfo.szName, "Shadows of Death");
	strcpy(pGameInfo.szWebsite, "http://www.reversity.org/");
	pGameInfo.szVersion[0] = 1;
	pGameInfo.szVersion[1] = 0;
	pGameInfo.szVersion[2] = 0;
	pGameInfo.szVersion[3] = 0;
	pGameInfo.bSendStats = true;
	pGameInfo.bOpenBrowser = false;
}