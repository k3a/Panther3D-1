//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Definition of common variables and functions
//-----------------------------------------------------------------------------
#include "common.h"

// TODO: Zbavit se tìchto glob. promìnných nebo ne???
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

// ZVUK
IP3DSoundManager	*g_pSoundMan=NULL;
IP3DSoundContainer	*g_pSoundContainer=NULL;	// FIXME: soundcontainer nie je globalna zalezitost, upravit

//PHYS. WORLD
IP3DPhysEngine		*g_pPhysEngine=NULL;

// GLOBAL CONSOLE VARS
ConVar CVphys_draw("phys_draw", "0", CVAR_ARCHIVE|CVAR_NOT_STRING, "Vykreslit fyzikální modely?" );
ConVar* CVr_width=NULL;
ConVar* CVr_height=NULL;
ConVar* CVr_windowed=NULL;

// Spolecne klavesy
ConVar CVk_use("k_use", "18", CVAR_ARCHIVE|CVAR_NOT_STRING); // key E