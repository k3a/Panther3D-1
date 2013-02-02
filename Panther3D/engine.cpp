//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Engine class definition
//-----------------------------------------------------------------------------

#include "engine.h"
#include "common.h"
#include "ip3dprimitive.h"
#include "console.h"
#include "IP3DMesh.h"
#include "IP3DMaterialManager.h"
#include "IP3DUpdateManager.h"
#include "cpu.h"

#include "prof.h"
#pragma warning (disable: 4311)

#include "particle_pointemitor.h"
CP3DPointEmitor pntEmitor; // DEBUG!!!

REGISTER_SINGLE_CLASS(CP3DEngine, IP3DENGINE_ENGINE); // zaregistruj tuto tøídu

static ConVar* CVcl_InMenu=NULL;

static ConVar CVe_drawaxis("e_drawaxis", "0", CVAR_NOT_STRING);
ConVar CVe_editor("e_editor", "0", CVAR_READONLY | CVAR_NOT_STRING);
static ConVar CVe_profiler("e_profiler", "0", CVAR_ARCHIVE | CVAR_NOT_STRING);
static ConVar CVe_prof_graph("e_prof_graph", "1", CVAR_ARCHIVE | CVAR_NOT_STRING);
static ConVar CVe_prof_report_mode("e_prof_report_mode", "1", CVAR_ARCHIVE | CVAR_NOT_STRING, "Mod profileru (sort self, hierarchy, call graph)");
static ConVar CVe_prof_y_spacing("e_prof_y_spacing", "10", CVAR_ARCHIVE | CVAR_NOT_STRING, "Roztazeni grafu na ose y");
static ConVar CVe_stereoscopic("e_stereoscopic", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Vykreslit pro stereoskopicky obraz");
static ConVar CVx_water("x_water", "0", CVAR_ARCHIVE | CVAR_NOT_STRING);

// autopatcher/updater
static IP3DUpdateManager *up=NULL;
static bool bPatcherIsRunning=false;

CP3DEngine::CP3DEngine()
{
	m_deltaTime = 0;
	m_nBuild = 0;
	fntA15 = NULL;
	fntA17 = NULL;
}

CP3DEngine::~CP3DEngine()
{
	DestroyWnd();
	SAFE_DELETE(fntA15);
	SAFE_DELETE(fntA17);
}

bool CP3DEngine::InitEngine(const char* cmdLine, bool bInEditor, HWND hWnd)
{
	I_RegisterModule("Engine");


	g_pEngine = (IP3DEngine*)this;
	CVe_editor.SetValue(bInEditor);

	srand (GetTickCount ());		// inicializacia gen. nahodnych cisel

	// ziskej ent manager
	g_pEntMgr = (IP3DEntityMgr*)I_GetClass(IP3DGAME_ENTITYMGR);
	if (!g_pEntMgr)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't get entity manager from this game module!");
		return false;
	}
	g_pEntMgr->Initialize(); // preloz nazvy a tak...

	// init console
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	g_pConsole->RegisterLastConVar(g_pLastConVar);

	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);
	if (g_pFS == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain file_system interface");
		return false;
	}

	// vytvor build number - pocet dnu od 1. dubna 2005 - priblizne datum, kdy se poprve zacalo premyslet o projektu
	// nepocita se s prestupnym rokem...
	const char* pszDate = __DATE__; // napr. Apr 24 2006
	char *pszMonths[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	BYTE nMonthDays[12] = { 31,     28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };
	// zatim ziskej pocet dnu od 1.1.2005
	UINT nBuild = (atoi(&pszDate[7])-2005)*365;
	for(UINT i=0;i<12;i++)//projdi vsechny mesice az do aktualniho a pricti pocet
	{
		if(!strnicmp(pszDate, pszMonths[i], 3)) break;
		nBuild+=nMonthDays[i];
	}
	nBuild+=atoi(&pszDate[4]); // pricti pocet dnu v aktualnim mesici
	nBuild-=91; // odecti dobu od 1.1.2005 do 1.4.2005 = 91
	m_nBuild = nBuild;

	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	CON(MSG_CON_INFO, "[Panther3D Engine%s - Ver. %d.%d.%d.%d Build: %d] ::: Started %d.%d.%d %d:%d:%d", CVe_editor.GetBool() ? " - Editor Mode" : "", P3DVER_1, P3DVER_2, P3DVER_3, P3DVER_4, nBuild, sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	// write engine information
#ifdef _DEBUG
	CON(MSG_CON_INFO, "= Panther3D Engine (%s, %s, DEBUG) initialization =", __DATE__, __TIME__);
#else
	CON(MSG_CON_INFO, "= Panther3D Engine (%s, %s) initialization =", __DATE__, __TIME__);
#endif

	// if window handle not specified, create new window
	I_GetClass(IP3DENGINE_LOCALIZATION);
	m_bOwnWindow = false;
	m_hWnd = hWnd;
	if (!hWnd) 
	{
		m_bOwnWindow = true;
		m_hWnd = CreateWnd();
	}

	// zjisti info o uzivateli a pc a vypis...
	char cmpName[256]="#"; char usrName[256]="#"; char msgOut[512]="#";
	DWORD strSize=256;
	GetComputerName(cmpName, &strSize);
	strSize=256;
	GetUserName(usrName, &strSize);
	sprintf(msgOut, "Engine: Logged in as %s @ %s", usrName, cmpName);
	CON(MSG_CON_INFO, msgOut);

	// zjisti CPU informace
	char strExt[128]=" ";
	if (g_bCPUInfo.bEXT)
	{
		if (g_bCPUInfo.bMMX) strcat(strExt, "MMX ");
		if (g_bCPUInfo.bMMXEX) strcat(strExt, "MMXEx ");
		if (g_bCPUInfo.b3DNOW) strcat(strExt, "3DNow ");
		if (g_bCPUInfo.b3DNOWEX) strcat(strExt, "3DNowEx ");
		if (g_bCPUInfo.bSSE) strcat(strExt, "SSE ");
		if (g_bCPUInfo.bSSE2) strcat(strExt, "SSE2 ");
	}
	sprintf(msgOut, "Engine: Processor %s %s (%s) - %d %s", 
		    g_bCPUInfo.vendor, g_bCPUInfo.name, strExt, g_bCPUInfo.nCores, g_bCPUInfo.nCores>1?"cores":"core");
	CON(MSG_CON_INFO, msgOut);

	// Naèti globální ukazatele

	// INIT GLOBAL TIMER
	g_pTimer = (IP3DTimer*)I_GetClass(IP3DENGINE_TIMER);
	if (g_pTimer == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain timer class interface");
		return false;
	}

	// INIT RENDERER MODULE
	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);
	if (g_pRenderer == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain renderer class interface");
		return false;
	}

	// set renderer
	g_pRenderer->InitRenderer( m_hWnd );
	g_pEngSet = g_pRenderer->GetSettings(); // get settings

	// SHOW GAME WINDOW
	if (m_bOwnWindow)
	{
		int swid = GetSystemMetrics(SM_CXSCREEN);
		int shei = GetSystemMetrics(SM_CYSCREEN);
		//SetWindowLong(m_hWnd, GWL_STYLE, g_pEngSet.Windowed ? WS_OVERLAPPEDWINDOW | WS_VISIBLE : WS_VISIBLE);
		MoveWindow(m_hWnd, swid/2-g_pEngSet.Width/2, shei/2-g_pEngSet.Height/2, g_pEngSet.Width, g_pEngSet.Height, TRUE);
		UpdateWindow(m_hWnd);
		ShowWindow( m_hWnd, SW_SHOWDEFAULT );
		if(!hWnd) SetForegroundWindow(m_hWnd);
	}

	// nastavit projekciu
	g_pRenderer->SetProjection (DEFAULT_FIELD_OF_VIEW, (float)g_pEngSet.Width/(float)g_pEngSet.Height, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);

	g_pPPManager = (IP3DPostprocessManager*)I_GetClass(IP3DRENDERER_POSTPROCESSMANAGER);
	if (g_pPPManager == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain postprocess_manager interface");
		return false;
	}

	// FIXME: debug na testovanie !!!
	/*
	test_effect
	test_bloom, test_blur, test_sepia, test_greyscale, test_blackandwhite,
	test_edgedetect, test_inversion, test_sobel_filter, test_ascii_art,
	test_radial_blur, test_emboss, test_mosaic, test_shower_glass,
	test_scope_sight, test_spherize, test_twirl, test_oldtv,
	test_pixelize, test_fresh, test_rainbow, test_rainbow_anim, test_motion_blur
	*/

// 	bool bRet = g_pPPManager->PreCache ("test_effect");
// 	bRet = g_pPPManager->AddEffect("test_effect");

	// skybox
	g_pSkyBox = (IP3DSky*)I_GetClass(IP3DRENDERER_SKY);
	if (g_pSkyBox == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain skybox class interface");
		return false;
	}

	// stereoscopic
	g_pStereoscopic = (IP3DStereoscopic*)I_GetClass(IP3DRENDERER_STEREOSCOPIC);
	if (g_pStereoscopic == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain stereoscope class interface");
		return false;
	}

	// mesh loader
	g_pMeshLoader = (IP3DMeshLoader*)I_GetClass(IP3DRENDERER_MESHLOADER);
	if (g_pMeshLoader == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain mesh_loader interface");
		return false;
	}

	// XML_loader
	g_pXML = (IP3DXML*)I_GetClass(IP3DENGINE_XML);
	if (g_pXML == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain xml_class interface");
		return false;
	}

	// ziskej ent manager
	g_pEntMgr = (IP3DEntityMgr*)I_GetClass(IP3DGAME_ENTITYMGR);
	if (g_pEntMgr == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't get entity manager from this game module!");
		return false;
	}

	// bsp_map_loader
	g_pBSPMap = (IP3DBSPMap*)I_GetClass(IP3DRENDERER_BSPMAP);
	if (g_pBSPMap == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain bsp_map interface");
		return false;
	}

	// light manager
	g_pLightManager = (IP3DLightManager*)I_GetClass(IP3DRENDERER_LIGHTMANAGER);
	if (g_pLightManager == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain light_manager interface");
		return false;
	}

	// alpha manager
	g_pAlphaManager = (IP3DAlphaManager*)I_GetClass(IP3DRENDERER_ALPHAMANAGER);
	if (g_pAlphaManager == NULL)
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain alpha_manager interface");
		return false;
	}

	// frustum
	g_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM);
	if (g_pFrustum == NULL) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain frustum class interface");
		return false;
	}

	// get global 2D drawing class
	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);
	if (g_p2D == NULL) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain 2D_draw class interface");
		return false;
	}

	// init fonts
	fntA15 = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	fntA17 = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	if (!fntA15 || !fntA15) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain font class interface");
		return false;
	}
	if(SUCCEEDED(fntA15->CreateFont("MS Sans Serif", 15, true, false))
    && SUCCEEDED(fntA17->CreateFont("MS Sans Serif", 17, false, false)))
			CON(MSG_CON_INFO, "Engine: Global fonts loaded!");

	// INIT PHYSICS MODULE
	g_pPhysEngine = (IP3DPhysEngine*)I_GetClass(IP3DPHYS_PHYSENGINE);
	if (!g_pPhysEngine || !g_pPhysEngine->Initialize()) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't initialize physics engine!");
		return false;
	}

	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);
	if (g_pInput == NULL) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain input class interface");
		return false;
	}
	if (g_pEngSet.Windowed)
		g_pInput->CreateDevices(m_hWnd, false);
	else
		g_pInput->CreateDevices(m_hWnd, !bInEditor);

	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
	if (g_pSoundMan == NULL) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain sound_manager interface");
		return false;
	}
	if (!g_pSoundMan->Initialize(m_hWnd)) 
		CON(MSG_ERR_FATAL, "Engine: Can't initialize Sound Manager!");

	// ziskaj triedu material_manager
	I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	
	// ziskaj triedu model_loader
	I_GetClass(IP3DRENDERER_MESHLOADER);
	
	// ziskej water manager
	m_pWaterMan = (IP3DWaterManager*)I_GetClass(IP3DRENDERER_WATERMANAGER);

	g_pGUISystem = (IP3DGUISystem*)I_GetClass(IP3DGUI_GUISYSTEM);
	if (g_pGUISystem == NULL) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain GUI_System interface");
		return false;
	}
	g_pGUISystem->InitGUISystem(); // Initializuj GUISystem
	CVcl_InMenu = g_pConsole->FindConVar("cl_InMenu");

	// helper class
	g_pHelperObject = (IP3DHelperObject*)I_GetClass(IP3DENGINE_HELPER_OBJECT);
	if (g_pHelperObject == NULL) 
	{
		CON(MSG_ERR_FATAL, "Engine: Can't obtain helper_object interface");
		return false;
	}

	// zkontroluj zda nejsou nove updaty
	up = (IP3DUpdateManager *)I_GetClass(IP3DENGINE_UPDATEMANAGER);
	//if (up->CheckForUpdates("http://localhost/bug"))
	if (up->CheckForUpdates("http://bug.7thsquad.com"))
		up->DownloadUpdates();

	g_pRenderer->EnableLighting(false);
	//g_pRenderer->EnableBlending(true);
	g_pRenderer->BlendFunc(RBLEND_SRCALPHA, RBLEND_INVSRCALPHA);

	// write engine information
	CON(MSG_CON_INFO, "Engine: Engine initialized!", __DATE__, __TIME__);
	
	currState = ES_RUNNING; // normalni beh
	
	// profiler
	Prof_set_report_mode((Prof_Report_Mode)CVe_prof_report_mode.GetInt());

	ParseCommandLine(cmdLine);

	// DEBUG!!! PARTICLE!!!
	pntEmitor.SetTexture("textures/sprites/part.dds");
	pntEmitor.SetEmissionRate(5);
	ParticleEmissionPoint ep;
	ep.vDirVelocity.x = ep.vDirVelocity.y = 0.0f;
	ep.vDirVelocity.z = 40.0;
	pntEmitor.SetEmissionTypePoint(ep);
	pntEmitor.SetEmissionRate(28);
	pntEmitor.SetParticleLifeTime(5);
	pntEmitor.SetMaxParticleNum(150);
	pntEmitor.SetAirResistance(0.0f);
	P3DXVector3D ed(10,10,0);
	pntEmitor.SetEmissionDeviation(ed);
	pntEmitor.CreateEmitor();

	return true; // ok
}

eEngineState CP3DEngine::RenderScene(HWND hwndViewport, unsigned int RSflags, unsigned long clrColor)
{
	Prof(ENGINE_CP3DEngine__RenderScene);
	// BEGIN SCENE
	g_pTimer->Loop(); // loop global timer
	m_deltaTime = g_pTimer->GetTimeDelta_s();
	g_fTimeDelta = m_deltaTime;

	//	g_pRenderer->SaveSceneIntoCubeMap(-760.0f, 88.0f, 168.0f);

	g_pRenderer->BeginScene(clrColor, false, false, true);
	
	// RENDER SCENE
	// gui
	if (CVcl_InMenu->GetBool()){
		g_pGUISystem->Render(m_deltaTime);
	}else{
		// render scene
		if (!CVe_stereoscopic.GetBool()) // vykresli klasicky
		{
			// postprocess manager
			if (RSflags&RS_POSTPROCESS) g_pPPManager->Begin ();

			// render scene
			RenderSceneInternal(RSflags);

			// postprocess manager
			if (RSflags&RS_POSTPROCESS) g_pPPManager->End ();
		}
		else // render anaglyph (2x) / TODO: customize code for stereo rendering or drop it?
		{
			// view vector
			P3DXVector3D viewDirection (m_viewVec);
			viewDirection.Normalize ();

			// find right vector from up vector and view vector
			P3DXVector3D strafeVector = viewDirection % m_upVector;		// cross product
			strafeVector.Normalize();
			// strafe eyes
			strafeVector *= 3.5f;

			P3DXVector3D EyePos;

			for(int oko=0;oko<2;oko++)
			{
				if (oko==0)
					EyePos = strafeVector + m_eyePos;
				else
					EyePos = -strafeVector + m_eyePos;

				g_pRenderer->SetCamera (EyePos, m_viewVec, m_upVector);

				// postprocess manager
				if (RSflags&RS_POSTPROCESS) g_pPPManager->Begin ();

				// render scene
				g_pStereoscopic->Begin(oko);
				RenderSceneInternal(RSflags);
				g_pStereoscopic->End(oko);

				// postprocess manager
				if (RSflags&RS_POSTPROCESS) g_pPPManager->End ();
			}
		}

		// set world matrix t oidentity - really needed here?
		P3DXMatrix matrix2;
		matrix2.SetIdentityMatrix ();
		g_pRenderer->SetWorldTransform (matrix2);

		// - render physics info
		g_pPhysEngine->RenderDebug();

		// Vykresli osy. Velmi uzitecna vec zvlast pro editor ;)
		if (CVe_drawaxis.GetBool()) RenderAxis();

		pntEmitor.Render(); // DEBUG!!!
	}

	// 2D "POST" RENDERING

	// - render profiler
	RenderProfiler();

	// - render console
	g_pConsole->Render();

	// - render engine info
	fntA15->DrawText(FA_CENTER, "Panther3D Engine <DEVELOPMENT STAGE> - Copyright © 2005-2008 Reversity Studios (www.reversity.org)", 0, g_pEngSet.Height - fntA15->GetLineH(), g_pEngSet.Width, g_pEngSet.Height, P3DCOLOR_XRGB(255, 255, 255));

	// END SCENE RENDERING
	g_pRenderer->EndScene();

	// - TODO: what about this place, it's really necessary to do some useful update work here?

	// PRESENT ON VIEWPORT
	g_pRenderer->Present(hwndViewport);

	return currState;
}

void CP3DEngine::RenderSceneInternal(unsigned int RSflags)
{
	// vykresli hru
	if (RSflags & RS_BSP) g_pBSPMap->Render();
	g_pEntMgr->Render(m_deltaTime, !!(RSflags&RS_MODELS), !!(RSflags&RS_ENTITIES), !!(RSflags&RS_VENTITIES));
	if (RSflags & RS_ATMOSPHERE) g_pSkyBox->Render (true, true);
	g_pAlphaManager->Render(); // TODO: make possible to render transparent objects within entity manager in future thus remove this class
}

void CP3DEngine::RenderProfiler()
{
	if(CVe_profiler.GetBool() && g_pInput)
	{
		//g_pMaterialManager->TurnOffShaders();
		Prof_draw_dx(10, 10, 500, 600, 2); // output precision <1-4>
		if (CVe_prof_graph.GetBool()) Prof_draw_graph_dx(520, 10, 2, CVe_prof_y_spacing.GetFloat());

		if (g_pInput->IsKeyDownFirstTime(DIK_UP))
		{
			Prof_move_cursor(-1);
		}
		else if (g_pInput->IsKeyDownFirstTime(DIK_DOWN))
		{
			Prof_move_cursor(1);
		}
		else if (g_pInput->IsKeyDownFirstTime(DIK_LEFT))
		{
			Prof_select_parent();
		}
		else if (g_pInput->IsKeyDownFirstTime(DIK_RIGHT))
		{
			Prof_select();
		}
		else if (g_pInput->IsKeyDownFirstTime(DIK_PGUP))
		{
			int reportMode = CVe_prof_report_mode.GetInt();
			if (reportMode>0) 
			{
				reportMode--;
				CVe_prof_report_mode.SetValue(reportMode);
			}
			Prof_set_report_mode((Prof_Report_Mode)reportMode);
		}
		else if (g_pInput->IsKeyDownFirstTime(DIK_PGDN))
		{
			int reportMode = CVe_prof_report_mode.GetInt();
			if (reportMode<2) 
			{
				reportMode++;
				CVe_prof_report_mode.SetValue(reportMode);
			}
			Prof_set_report_mode((Prof_Report_Mode)reportMode);
		}
	}
}

void CP3DEngine::UpdateScene()
{
	Prof(ENGINE_CP3DEngine__UpdateScene);

	// >>> udìlej vìci nároèné na CPU... >>>
	bool bUpdateGame = !CVcl_InMenu->GetBool() && !g_pConsole->IsShown();

	// >>> SIMULACE FYZIKY
	if(bUpdateGame) g_pPhysEngine->SimulateBegin(m_deltaTime);

	//pntEmitor.Update(); // DEBUG!!!

	// ...KEYBOARD AND MOUSE
	g_pInput->Loop();

	// ...GUI SYSTEM A PROFILER
	g_pGUISystem->Update(m_deltaTime);
	if (CVe_profiler.GetBool()) Prof_update(1);

	// ...ALPHA MANAGER (SORTING)
	g_pAlphaManager->Loop ();

	// ...DYNAMIC LIGHTS
	g_pLightManager->Loop ();

	// ...SOUND MANAGER
	g_pSoundMan->Update();

	// ...HELPER - Messova hracicka :)
	g_pHelperObject->Loop ();

	// >>> KONEC SIMULACE FYZIKY... pockej na dokonceni simulace v druhem vlakne
	if(bUpdateGame) g_pPhysEngine->SimulateEnd();

	// ENTITOVY SYSTEM 
	if(!CVcl_InMenu->GetBool() && !g_pConsole->IsShown()) g_pEntMgr->Think(m_deltaTime);
}

void CP3DEngine::PlaySplashVideo(const char* movieName) // nazev souboru vcetne .avi! Zatim nepouziva filesystem!
{
	char str[255];
	char path[MAX_PATH];
	
	return; // FINAL: !!!zatim jeste neni treba splash
	
	GetCurrentDirectory(MAX_PATH, path);

	sprintf(str, "OPEN \"%s\\data\\splash\\%s\" ALIAS 7vi TYPE AviVideo", path, movieName);
	mciSendString (str, NULL, NULL, NULL);

	sprintf(str, "WINDOW 7vi HANDLE %d", (int)g_pEngSet.hWnd);
	mciSendString (str, NULL, NULL, NULL);

	sprintf(str, "PUT 7vi DESTINATION AT 0 0 %d %d", g_pEngSet.Width, g_pEngSet.Height);
	mciSendString (str, NULL, NULL, NULL);

	mciSendString ("PLAY 7vi", NULL, NULL, NULL);

	// pockej na konec nebo klavesu
	strcpy(str, "playing");
	int KeyResult;
	do
	{
		Sleep(100);

		//klavesy?
		mciSendString ("STATUS 7vi MODE WAIT", str, 255, NULL);
		// esc
		KeyResult = GetAsyncKeyState(27);
		if (KeyResult == -32767) break;
		// space
		KeyResult = GetAsyncKeyState(32);
		if (KeyResult == -32767) break;
		// enter
		KeyResult = GetAsyncKeyState(13);
		if (KeyResult == -32767) break;
	} while (!strcmp(str, "playing"));

	mciSendString ("CLOSE 7vi", NULL, NULL, NULL);
}

void CP3DEngine::Shutdown()
{
	currState = ES_QUITTING;

	g_pEntMgr->KillAll();

	// pokud jsou stazeny vsechny updaty, spust updater s timto id procesu at ceka na jeho ukonceni
	if (up->GetNumUpdates()>0 && up->GetNumUpdates()==up->GetNumDownloaded())
	{
		DWORD dwThread = GetCurrentProcessId();
		char currdir[MAX_PATH]="";
		char tooldir[MAX_PATH]="";
		char params[512]="";

		GetCurrentDirectory(MAX_PATH-1, currdir);
		sprintf(params, "\"%s\\data\\\\\" %u", currdir, dwThread);

		sprintf(tooldir, "%s\\data\\PatchPatcher.exe", currdir);

		bPatcherIsRunning = true;
		ShellExecute(0, "open", tooldir, params, currdir, 1);
	}
}

void CP3DEngine::SetCamera(P3DXVector3D &eyePos, P3DXVector3D &viewVector, P3DXVector3D &upVector)
{
	if (CVcl_InMenu && (CVcl_InMenu->GetBool())) return; // not set camera while in menu

	m_eyePos = eyePos;
	m_viewVec = viewVector;

	g_pRenderer->SetCamera (m_eyePos, m_viewVec, upVector);

	// ...BSP MAP
	g_pBSPMap->Loop();
}

inline void CP3DEngine::RenderAxis()
{
	// Vykresli osy. Velmi uzitecna vec zvlast pro editor ;)
	if (CVe_drawaxis.GetBool())
	{
		P3DXVector3D axisZero;
		P3DXVector3D axis;
		P3DVector2D pnt[2];

		bool tst = true;

		// 0
		axisZero.x = 0.0f; axisZero.y = 0.0f; axisZero.z = 0.0f;
		g_pRenderer->WorldToScreenSpace(axisZero, tst);
		axisZero.x -= g_pEngSet.Width/2.0f-30.0f;
		axisZero.y += g_pEngSet.Height/2.0f-30.0f;

		// X
		axis.x = 1.0f; axis.y = 0.0f; axis.z = 0.0f;
		g_pRenderer->WorldToScreenSpace(axis, tst);
		axis.x -= g_pEngSet.Width/2.0f-30.0f;
		axis.y += g_pEngSet.Height/2.0f-30.0f;

		pnt[0].x = axisZero.x; pnt[0].y = axisZero.y;
		pnt[1].x = axis.x; pnt[1].y = axis.y;
		g_p2D->Line_Draw(pnt, 2, P3DCOLOR_XRGB(255, 0, 0));
		fntA17->DrawText("x", (int)axis.x-2, (int)axis.y-10, P3DCOLOR_XRGB(255, 0, 0));

		// Y
		axis.x = 0.0f; axis.y = 1.0f; axis.z = 0.0f;
		g_pRenderer->WorldToScreenSpace(axis, tst);
		axis.x -= g_pEngSet.Width/2.0f-30.0f;
		axis.y += g_pEngSet.Height/2.0f-30.0f;

		pnt[0].x = axisZero.x; pnt[0].y = axisZero.y;
		pnt[1].x = axis.x; pnt[1].y = axis.y;
		g_p2D->Line_Draw(pnt, 2, P3DCOLOR_XRGB(0, 255, 0));
		fntA17->DrawText("y", (int)axis.x-2, (int)axis.y-10, P3DCOLOR_XRGB(0, 255, 0));

		// Z
		axis.x = 0.0f; axis.y = 0.0f; axis.z = 1.0f;
		g_pRenderer->WorldToScreenSpace(axis, tst);
		axis.x -= g_pEngSet.Width/2.0f-30.0f;
		axis.y += g_pEngSet.Height/2.0f-30.0f;

		pnt[0].x = axisZero.x; pnt[0].y = axisZero.y;
		pnt[1].x = axis.x; pnt[1].y = axis.y;
		g_p2D->Line_Draw(pnt, 2, P3DCOLOR_XRGB(0, 0, 255));
		fntA17->DrawText("z", (int)axis.x-2, (int)axis.y-10, P3DCOLOR_XRGB(0, 0, 255));

		char posText[32]; P3DXVector3D camPos; float fFOV=0;
		g_pRenderer->GetCameraInfo(camPos, fFOV);
		sprintf(posText, "%d %d %d", (int)camPos.x, (int)camPos.y, (int)camPos.z);
		fntA17->DrawText(posText, 75, g_pEngSet.Height-27, P3DCOLOR_XRGB(255, 128, 0));
	}
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_DESTROY:
//		if (g_pConsole) g_pConsole->Command("quit");
		//PostQuitMessage( 0 );
		return 0;
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

//-----------------------------------------------------------------------------
HWND CP3DEngine::CreateWnd()
{
	// Register window class
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), LoadIcon(GetModuleHandle(NULL), "IDI_ICON"), NULL, NULL, NULL,
		"P3DWindow", NULL };
	RegisterClassEx( &wc );

	// Create game widnow
	m_hWnd = CreateWindow( "P3DWindow", g_pLocaliz->Translate_const("#gui_GameTitle"), 
		0, 0, 0, 800, 600,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );

	// Hide game widnow
	ShowWindow( m_hWnd, SW_HIDE );

	if(!m_hWnd)
	{
		CON(MSG_ERR_FATAL, "Can't create game window!");
	}

	ShowCursor(false);

	return m_hWnd;
}

//-----------------------------------------------------------------------------
void CP3DEngine::DestroyWnd()
{
	if (m_bOwnWindow)
	{
		DestroyWindow(m_hWnd);
		UnregisterClass( "P3DWindow", GetModuleHandle(NULL) ); // unregister window class
	}
}
//-----------------------------------------------------------------------------
void CP3DEngine::ParseCommandLine(const char* lpCmdLine)
{
	// ---- ZPRACUJ PRIKAZY
	char szCmdLine[100]; int nStart=0;
	for(unsigned int i=0;i<strlen(lpCmdLine);i++)
	{
		if (lpCmdLine[i] == '-') 
		{
			if (nStart>0)
			{
				// execute command
				strcpy(szCmdLine, &lpCmdLine[nStart]);
				szCmdLine[i-nStart-1]=0;
				g_pConsole->Command(szCmdLine);
			}

			nStart = i+1;
		}

	}
	// execute last command
	if(strlen(lpCmdLine) && lpCmdLine[nStart-1]=='-') 
		g_pConsole->Command(&lpCmdLine[nStart]);
}

// PROFILER 
//-----------------------------------------------------------------------------
Prof_Zone_Stack **CP3DEngine::GetProf_stack()
{
	Prof_stackPTR = &Prof_stack;
	return &Prof_stack;
}
//-----------------------------------------------------------------------------
Prof_Zone_Stack *CP3DEngine::GetProf_dummy()
{
	Prof_dummyPTR = &Prof_dummy;
	return &Prof_dummy;
}
//-----------------------------------------------------------------------------
Prof_StackAppendFn CP3DEngine::GetProf_StackAppendFn()
{
	Prof_StackAppendPTR = Prof_StackAppend;
	return Prof_StackAppend;
}