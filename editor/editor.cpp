#include "editor.h"
#include "common.h"
#include "frmMain.h"

IMPLEMENT_APP(CEditor)

CEditor::CEditor()
{
}

CEditor::~CEditor()
{ 
}

bool CEditor::OnInit()
{
	//_CrtSetBreakAlloc(14159);
	SetVendorName(wxString("Reversity Studios"));
	SetClassName(wxString("P3DCrane2"));
	SetAppName(wxString("P3DCrane2"));

	// SHOW SPLASH
	dlgSplashGui* pSplash = new dlgSplashGui( (wxWindow*)NULL );
	pSplash->txtInfo->SetWindowStyle(pSplash->txtInfo->GetWindowStyle()|wxST_NO_AUTORESIZE);
	pSplash->Show();
	pSplash->txtInfo->SetLabel(" Initializing...");
	pSplash->Update();

	// init all image handlers for wx (ico, bmp, ...)
	wxInitAllImageHandlers();

	// load settings
	Settings::Load("editor.ini");

	// create main window
	frmMain* mainFrm = new frmMain( (wxWindow*)NULL );

	// LOAD ENGINE AND GAME
	// DEBUG!!!! odkomentuj na konci
	/*pSplash->txtInfo->SetLabel(" Loading engine and modules...");
	char szExePath[MAX_PATH];
	GetModuleFileName(NULL, szExePath, MAX_PATH);
	for(int i=(int)strlen(szExePath); i > 0; i--)
	{
		if (szExePath[i]=='/' || szExePath[i]=='\\') 
		{
			szExePath[i]=0;
			break;
		}
	}
	I_Initialize(szExePath); // initialize dll module system

	pSplash->txtInfo->SetLabel(" Loading game module...");
	I_LoadNewModule("main.game"); // game module

	mpGame = (IP3DGame *)I_GetClass(IP3DGAME_GAME);
	HWND hWnd = (HWND)mainFrm->GetHWND();

	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	if (g_pEngine)
	{ 
		I_GetClass(IP3DSOUND_SOUNDMANAGER); // FIXME: WTF? WHY IT MUST BE THERE?
		g_pEngine->InitEngine(GetCommandLine(), true, hWnd); // initialize engine

		g_pConsole->Command("e_drawaxis 1"); // TODO: Move to engine (if editorMode = true) showAxis

		pSplash->txtInfo->SetLabel(" Loading common singletons...");
		mpGame->InitGame(GetCommandLine(),true);
		Sys_LoadCommon(hWnd); // load common singletons
	}

	// SHOW MAIN APP FRAME
	pSplash->txtInfo->SetLabel(" Loading user interface...");
	pSplash->Update();*/

	mainFrm->Show();

	// HIDE SPLASH
	delete pSplash;
	SetTopWindow( mainFrm );

	return true;
}

int CEditor::OnExit()
{
	Settings::Save("editor.ini");

	// DEBUG!!!
/*	g_pEngine->Shutdown();
	I_Shutdown();*/

	return 0;
}