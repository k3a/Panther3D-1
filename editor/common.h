#pragma once

// --------------------------------- EDITOR --------------------------------------

#include <wx/wx.h>
#include "settings.h"
#include "frmMain.h"

extern frmMain *g_pFrmMain;
extern char g_sGameModule[];
extern int b_nRealtimeWindows;

// ---------------------------- GAME AND MODULES ---------------------------------

#include "interface.h"
#include "symbols.h" // macros SAFE_... and more
#include "IP3DConsole.h"
#include "IP3DEngine.h"
#include "IP3DEntityMgr.h"
#include "IP3DBSPMap.h"
#include "IP3DInput.h"
#include "ip3dsoundengine.h"
#include "ip3ddraw2d.h"
#include "ip3dphysengine.h"
#include "ip3dxml.h"
#include "IP3DAlphaManager.h"
#include "IP3DRenderer.h"
#include "ip3dsky.h"
#include "IP3DLightManager.h"
#include "ip3dmesh.h"
#include "IP3DFrustum.h"
#include "IP3DTimer.h"
#include "IP3DLocalization.h"
#include "ip3dfilesystem.h"

using namespace std;

// T¿ÕDY
extern IP3DEngine* g_pEngine;
extern IP3DConsole *g_pConsole;
extern IP3DEntityMgr *g_pEntMgr;
extern IP3DBSPMap *g_pBSPMap;
extern IP3DPhysCharacter *g_pCharCtrl;
extern IP3DInput *g_pInput;
extern IP3DDraw2D *g_p2D;
extern IP3DXML* g_pXml;
extern IP3DAlphaManager* g_pAlphaMan;
extern IP3DRenderer* g_pRenderer;
extern IP3DSky*	g_pSkyBox;
extern IP3DLightManager* g_pLightManager;
extern IP3DMeshLoader*	g_pMeshLoader;
extern IP3DFrustum*	g_pFrustum;
extern IP3DTimer* g_pTimer;
extern IP3DLocalization* g_pLocaliz;
extern IP3DFileSystem* g_pFS;

// ZVUK
extern IP3DSoundManager	*g_pSoundMan;
extern IP3DSoundContainer	*g_pSoundContainer;

//PHYS. WORLD
extern IP3DPhysEngine		*g_pPhysEngine;

// ------------------------------------------------------------------------------------

extern bool Sys_LoadCommon(HWND hWnd );
extern void Sys_UnloadModules();