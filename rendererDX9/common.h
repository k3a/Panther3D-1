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
#include "d3dx9.h"
#include "ip3dconsole.h"
#include "ip3dengine.h" // kvuli EngineSettings_t
#include "IP3DMesh.h"
#include "IP3DMaterialManager.h"
#include "IP3DLightManager.h"
#include "IP3DPostprocessManager.h"
#include "IP3DTimer.h"
#include "symbols.h" // useful functions and macros
#include "renderer.h" // globální promìnná
#include "ip3dfilesystem.h"
#include "texture_loader.h"
#include "IP3DXML.h"
#include "IP3DAlphaManager.h"
#include "IP3DPhysEngine.h"
#include "IP3DFrustum.h"
#include "profiler.h"
#include "stats.h" // STATS
#include "IP3DSoundEngine.h"
#include "IP3DEntityMgr.h"
// renderer:
#include "resource_manager.h"
#include "draw2d.h"

class CP3DTextureLoader;
extern LPDIRECT3D9             g_pD3D; // Used to create the D3DDevice
extern LPDIRECT3DDEVICE9       g_pD3DDevice; // Our rendering device
extern EngineSettings_t g_pEngSet; // engine settings
extern IP3DConsole *g_pConsole;  // globální konzola
extern CP3DRenderer *g_pRenderer; // globální promìnná
extern CP3DTextureLoader g_TextureLoader;
extern IP3DMaterialManager *g_pMaterialManager;
extern IP3DMeshLoader	*g_pMeshLoader;
extern IP3DPostprocessManager *g_pPostProcessMgr;
extern IP3DAlphaManager	*g_pAlphaManager;
extern IP3DFileSystem *g_pFS;
extern IP3DXML	*g_pXML;
extern IP3DEngine *g_pEngine;
extern IP3DPhysEngine *g_pPhysEngine;
extern IP3DFrustum* g_pFrustum;
extern IP3DLightManager	*g_pLightManager;
extern IP3DTimer *g_pTimer;
extern IP3DSoundManager *g_pSoundMan;
extern IP3DEntityMgr *g_pEntMgr;
// renderer:
extern CP3DResourceManager *g_pResMgr;
extern CP3DDraw2D *g_p2D;

extern D3DXMATRIXA16 g_matView;
extern D3DXMATRIXA16 g_matProj;
extern D3DXMATRIXA16 g_matViewProj;
extern P3DXVector3D		g_cameraPosition;
extern float			g_currentFOV;

extern CP3DStats g_stats; // STATS

extern MATERIAL_PARAMETER g_paramTable[SEM_NUM];

//*********************************** funkce ***********************************
void DXErrInfo (HRESULT hr, char *szFile = NULL, DWORD dwLine = 0);
