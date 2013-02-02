#include "common.h"

// ---------------------------------- EDITOR -------------------------------------------

frmMain *g_pFrmMain=NULL;
char g_sGameModule[128];
int b_nRealtimeWindows=0;

// ---------------------------- ENGINE AND MODULES -------------------------------------

// TØÍDY
IP3DConsole *g_pConsole = NULL;
IP3DEngine* g_pEngine = NULL;
IP3DEntityMgr *g_pEntMgr = NULL;
IP3DBSPMap *g_pBSPMap = NULL;
IP3DPhysCharacter *g_pCharCtrl = NULL;
IP3DInput *g_pInput = NULL;
IP3DDraw2D *g_p2D = NULL;
IP3DXML* g_pXml = NULL;
IP3DAlphaManager* g_pAlphaMan;
IP3DSky*	g_pSkyBox;
IP3DRenderer* g_pRenderer=NULL;
IP3DLightManager* g_pLightManager=NULL;
IP3DMeshLoader*	g_pMeshLoader=NULL;
IP3DFrustum*	g_pFrustum=NULL;
IP3DTimer*	g_pTimer=NULL;
IP3DLocalization* g_pLocaliz=NULL;
IP3DFileSystem* g_pFS=NULL;

// ZVUK
IP3DSoundManager	*g_pSoundMan=NULL;
IP3DSoundContainer	*g_pSoundContainer=NULL;	// FIXME: soundcontainer isn't global singleton, modify

//PHYS. WORLD
IP3DPhysEngine		*g_pPhysEngine=NULL;

// --------------------------------------------------------------------------------------

class CListener : public IP3DConsoleListener
{
public:
	void Message(ConsoleMsg_t type, const char* text){
		if (g_pFrmMain) g_pFrmMain->AddConsoleMsg(text, "");
	};
	void MessageEx(ConsoleMsg_t type, const char* scopeName, const char* text){};
	/*void MessageEx(ConsoleMsg_t type, const char* scopeName, const char* text)
	{
		if (g_pFrmMain) g_pFrmMain->AddConsoleMsg(text, scopeName);
	}*/
	bool Command(const char *cmd){return false;};
};
static CListener s_listener;

bool Sys_LoadCommon(HWND hWnd)
{
	// LOAD GLOBAL SINGLETONS
	// TODO: check return values?!

	// XML_loader
/*	g_pXml = (IP3DXML*)I_GetClass(IP3DENGINE_XML);

	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);

	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);

	// bsp_map_loader
	g_pBSPMap = (IP3DBSPMap*)I_GetClass(IP3DRENDERER_BSPMAP);

	// get global 2D drawing class
	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);

	// INIT PHYSICS MODULE
	g_pPhysEngine = (IP3DPhysEngine*)I_GetClass(IP3DPHYS_PHYSENGINE);
	g_pCharCtrl = (IP3DPhysCharacter*)I_GetClass(IP3DPHYS_CHARACTER);

	// init input
	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);
	if ((!g_pInput) || (g_pInput->CreateDevices(hWnd, hWnd, false) != S_OK))  // TOOD: hwndViewport??? Why??? Use main app frame's hwnd first, then change at run-time
	{ MessageBox(0, "Can't create input device!", "Error!", MB_ICONSTOP | MB_SYSTEMMODAL); return false;}

	g_pSkyBox = (IP3DSky*)I_GetClass(IP3DRENDERER_SKY);
	if (g_pSkyBox == NULL)
	{ MessageBox(0, "Can't initialize skybox class!", "Error!", MB_ICONSTOP | MB_SYSTEMMODAL); return false;}

	// init sound
	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
	if (!g_pSoundMan->Initialize(hWnd))
	{ MessageBox(0, "Can't initialize SoundManager!", "Error!", MB_ICONSTOP | MB_SYSTEMMODAL); return false;}

	// FIXME: it's not a singleton ???
	// init sound container
	g_pSoundContainer = (IP3DSoundContainer*)I_GetClass(IP3DSOUND_SOUNDCONTAINER);
	if (!g_pSoundContainer) { MessageBox(0, "Can't get g_pSoundContainer!", "Error!", MB_ICONSTOP | MB_SYSTEMMODAL); return false;}

	// získej globální konzolu
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	g_pConsole->RegisterLastConVar(g_pLastConVar);*/
	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);

	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);
	g_pConsole->SetConsoleListener(&s_listener);
/*
	// Alpha manager
	g_pAlphaMan = (IP3DAlphaManager*)I_GetClass(IP3DRENDERER_ALPHAMANAGER);
*/
	return true; // ok
}

