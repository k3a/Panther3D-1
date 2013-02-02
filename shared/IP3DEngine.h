//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Engine interface (engine)
//-----------------------------------------------------------------------------
#pragma once

// !!! NASTAVENI VERZE ENGINU !!! // NUTNO ZMENIT TAKY VERSION_INFO V RESOURCES KAZDEHO BIN
#define P3DVER_1 0
#define P3DVER_2 1
#define P3DVER_3 0
#define P3DVER_4 0
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "interface.h"
#include "IP3DEntityMgr.h"

#include "profiler.h"
typedef Prof_Zone_Stack * (*Prof_StackAppendFn) (Prof_Zone *zone); // ukazatel na funkci pro profiler

struct EngineSettings_t {
    UINT Width, Height;
    HWND hWnd;
    BOOL Windowed;
};

enum eEngineState
{
	ES_QUITTING=0,
	ES_RUNNING,
};

// render scene flags
// - game
#define RS_BSP			1<<0
#define RS_MODELS		1<<1
#define RS_ENTITIES		1<<2
#define RS_DYNLIGHTS	1<<3
#define RS_SHADOWS		1<<4
#define RS_POSTPROCESS	1<<5
#define RS_PARTICLES	1<<6
#define RS_ATMOSPHERE	1<<7 // skybox, clouds, ...
// - visualization
#define RS_VCLUSTERS	1<<16
#define RS_VENTITIES	1<<17
// - all
#define RS_ALL_GAME		RS_BSP | RS_MODELS | RS_ENTITIES | RS_DYNLIGHTS | RS_SHADOWS | RS_POSTPROCESS | RS_PARTICLES | RS_ATMOSPHERE

class IP3DEngine : public IP3DBaseInterface
{
public:
	/** Initialize engine. Must be called at the host app (launcher, editor) as first method. */
	virtual bool InitEngine(const char* cmdLine, bool bInEditor=false, HWND hWnd=0)=0;
	virtual eEngineState RenderScene(HWND hwndViewport=NULL, unsigned int RSflags=RS_ALL_GAME, unsigned long clrColor=0)=0;
	virtual void UpdateScene()=0;
	virtual float GetDeltaTime()=0;
	virtual unsigned int GetBuildNumber()const=0;
	/** Request engine shutdown. */
	virtual void Shutdown()=0;
	virtual void PlaySplashVideo(const char* szMovieFilename)=0; // nazev souboru vcetne .avi! Zatim nepouziva filesystem!
	virtual void SetCamera(P3DXVector3D &eyePos, P3DXVector3D &viewVector, P3DXVector3D &upVector)=0;
	virtual bool InEditorMode()const=0;
	// profiler - internal methods
	virtual Prof_Zone_Stack **GetProf_stack()=0;
	virtual Prof_Zone_Stack *GetProf_dummy()=0;
	virtual Prof_StackAppendFn GetProf_StackAppendFn()=0;
};

#define IP3DENGINE_ENGINE "P3DEngine_6" // nazev ifacu