//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Engine class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "ip3dengine.h"
#include "ip3drenderer.h"
#include "IP3DWaterManager.h"

extern ConVar CVe_editor;

class CP3DEngine : public IP3DEngine
{
public:
	CP3DEngine();
	~CP3DEngine();
	bool InitEngine(const char* cmdLine, bool bInEditor=false, HWND hWnd=0);
	eEngineState RenderScene(HWND hwndViewport, unsigned int RSflags, unsigned long clrColor);
	void UpdateScene();
	float GetDeltaTime(){return m_deltaTime;};
	unsigned int GetBuildNumber()const{return m_nBuild;};
	void Shutdown();
	void PlaySplashVideo(const char* szMovieFilename); // nazev souboru vcetne .avi! Zatim nepouziva filesystem!
	void SetCamera(P3DXVector3D &eyePos, P3DXVector3D &viewVector, P3DXVector3D &upVector); // nastavovat kameru je potreba touto metodou (engine uz sam provede dle potreb novy render pro streoskopicky vzhled
	bool InEditorMode()const{return CVe_editor.GetBool();};
	// profiler - internal methods
	Prof_Zone_Stack **GetProf_stack();
	Prof_Zone_Stack *GetProf_dummy();
	Prof_StackAppendFn GetProf_StackAppendFn();
private:
	void ParseCommandLine(const char* lpCmdLine);
	HWND CreateWnd();
	void DestroyWnd();
	void RenderSceneInternal(unsigned int RSflags);
	void RenderProfiler();
	inline void RenderAxis();
	
	//
	HWND m_hWnd; // main window handle
	bool m_bOwnWindow; // window was created by engine
	int m_nBuild;
	float m_deltaTime; // time of one frame in seconds
	eEngineState currState; // current engine state

	IP3DWaterManager *m_pWaterMan; // DEBUG: for debugging purposes

	// camera information
	P3DXVector3D m_eyePos; 
	P3DXVector3D m_viewVec; 
	P3DXVector3D m_upVector;
};