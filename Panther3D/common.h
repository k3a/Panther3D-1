//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Declaration of common variables and functions
//-----------------------------------------------------------------------------
#pragma once

#include "symbols.h" // useful functions and macros

#include "ip3drenderer.h"
#include "ip3dengine.h"
#include "ip3dconsole.h"
#include "ip3dinput.h"
#include "ip3ddraw2d.h"
#include "types.h"
#include "ip3dbspmap.h"
#include "ip3dmesh.h"
#include "IP3DPostprocessManager.h"
#include "IP3DStereoscopic.h"
#include "ip3dphysengine.h"
#include "IP3DTimer.h"
#include "ip3dxml.h"
#include "IP3DFrustum.h"
#include "IP3DGUI.h"
#include "ip3dfilesystem.h"
#include "profiler.h"
#include "IP3DEntityMgr.h"
#include "IP3DAlphaManager.h"
#include "ip3dsky.h"
#include "IP3DSoundEngine.h"
#include "IP3DHelperObject.h"
#include "IP3DLightManager.h"
#include "localization.h"

// GLOBALNI UKAZATELE NA CASTO POUZIVANE SINGLETONY

extern IP3DRenderer* g_pRenderer; // renderer class
extern IP3DMeshLoader* g_pMeshLoader;
extern EngineSettings_t g_pEngSet;  // engine settings
extern IP3DConsole* g_pConsole; // global console
extern IP3DTimer* g_pTimer; // global timer
extern IP3DInput *g_pInput; // global input
extern IP3DDraw2D* g_p2D;  // pro 2D kreslení
extern IP3DXML*	g_pXML;
extern IP3DBSPMap*	g_pBSPMap;
extern IP3DFrustum*	g_pFrustum;
extern IP3DEngine* g_pEngine;
extern IP3DFileSystem* g_pFS;
extern IP3DPostprocessManager	*g_pPPManager;
extern IP3DStereoscopic	*g_pStereoscopic;
extern IP3DEntityMgr *g_pEntMgr;
extern IP3DAlphaManager	*g_pAlphaManager;
extern IP3DSky				*g_pSkyBox;
extern IP3DSoundManager *g_pSoundMan;
extern IP3DHelperObject	*g_pHelperObject;
extern IP3DLightManager	*g_pLightManager;
extern CP3DLocalization *g_pLocaliz;

extern IP3DPhysEngine	 *g_pPhysEngine;

// fonts
extern IP3DFont* fntA15; // 15 Bold font
extern IP3DFont* fntA17; // 17 font

// GUI
extern IP3DGUISystem *g_pGUISystem;

// delta time
extern float g_fTimeDelta;
inline float GetDeltaTime(){ return g_fTimeDelta; };