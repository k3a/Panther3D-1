//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI System DLL, TODO: menu pri nactene hre!!!
//-----------------------------------------------------------------------------
#include "GuiSystem.h"
#include "profiler.h"
#include "IP3DUpdateManager.h"

static ConVar CVcl_InMenu("cl_InMenu", "1", CVAR_NOT_STRING | CVAR_READONLY); //FINAL, FIXME hodnota
static IP3DMaterialManager* g_pMaterialManager=NULL;

REGISTER_SINGLE_CLASS(CGUISystem, IP3DGUI_GUISYSTEM);

static bool bDrawGUI=true; //FINAL, FIXME // pocatecni stav menu - zobrazeno po sputeni hry

static IP3DTexture* pBackground=NULL;
static IP3DTexture* pDefBackground=NULL; // standardni pozadi menu

static CMainMenu s_MainMenu;

float mouseX=0.0f, mouseY=0.0f;

//#include "ParticleGraph.h"  //DEBUG!!
//CP3DGUI_ParticleGraph* test;  //DEBUG!!

// update - docasne tady // TODO_UP
IP3DUpdateManager *s_pUpdate=NULL;

// spolecne dialogy
#include "SelectFile.h"

bool CGUISystem::InitGUISystem()
{
	I_RegisterModule("GUI System");
	// memory leaks detection
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(180);

	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
	// profiler
	Prof_stackPTR = g_pEngine->GetProf_stack();
	Prof_dummyPTR = g_pEngine->GetProf_dummy();
	Prof_StackAppendPTR = g_pEngine->GetProf_StackAppendFn();

	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	g_pConsole->RegisterLastConVar(g_pLastConVar);

	g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT); // ziskej input
	g_Scheme.Init(); 	// init scheme

	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);
	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	g_pLocaliz = (IP3DLocalization*)I_GetClass(IP3DENGINE_LOCALIZATION);
	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);

	s_pUpdate = (IP3DUpdateManager *)I_GetClass(IP3DENGINE_UPDATEMANAGER);

	CVr_width = g_pConsole->FindConVar("r_width");
	CVr_height = g_pConsole->FindConVar("r_height");

	pDefBackground = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE);
	if(!pDefBackground->Load("gui/bg.dds", TT_2D))
	{
		SAFE_DELETE(pDefBackground);
		CON(MSG_CON_ERR, "GUI: Can't load bg texture gui/bg!");
	}

	s_MainMenu.Initialize();

	// vytvor polozky hlavniho menu. TODO: mimo hru?
	s_MainMenu.AddItem(g_pLocaliz->Translate_const("#GUI_ResumeGame")); // TODO: resume/new games
	s_MainMenu.AddItem(g_pLocaliz->Translate_const("#GUI_LoadGame"));
	s_MainMenu.AddItem(g_pLocaliz->Translate_const("#GUI_SaveGame"));
	s_MainMenu.AddItem(g_pLocaliz->Translate_const("#GUI_Options"));
	s_MainMenu.AddItem(g_pLocaliz->Translate_const("#GUI_Exit"));

	// ziskat a skryt dialogy
	g_pDia_NewGame = new CP3DGUI_NewGame();
	 g_pDia_NewGame->m_bHidden = true;
	g_pDia_Settings = new CP3DGUI_Settings();
	 g_pDia_Settings->m_bHidden = true;
	 
	// spolecne dialogy
	dia_SelectFile = new CP3DGUI_SelectFile();
	 dia_SelectFile->m_bHidden = true;
	
	//test = new CP3DGUI_ParticleGraph();  //DEBUG!!

//	l.Init();
#ifdef _DEBUG
	CON(MSG_CON_INFO, "= P3D GUI System (%s, %s, DEBUG) initialized =", __DATE__, __TIME__);
#else
	CON(MSG_CON_INFO, "= P3D GUI System (%s, %s) initialized =", __DATE__, __TIME__);
#endif

	return true;
}

/*!
* \brief
* Vykresli vsechny prvky GUI, obcas je v tom zahrnuta i rychla aktualizace nekterych prvku.
* 
* Vykresli taktez pozadi menu a hlavni nabidku menu.
*
* \param deltaTime
* Pocet sekund v tomto snimku.
*/
bool CGUISystem::Render(float deltaTime)
{
	Prof(GUI_CGUISystem__Render);

	if(!bDrawGUI) return true;

	g_pMaterialManager->TurnOffShaders(); // nepouzivame shadery

	// vykresli pozadi
	P3DXVector3D p;
	p.x=p.y=p.z=0.0f;
	//g_p2D->Sprite_Begin(0);
	if(pBackground) 
		g_p2D->DrawTexture(pBackground, 0, 0, CVr_width->GetInt(), CVr_height->GetInt());
		//g_p2D->Sprite_Draw(pBackground, NULL, NULL, &p, P3DCOLOR_XRGB(128, 128, 128));
	else
		g_p2D->DrawTexture(pDefBackground, 0, 0, CVr_width->GetInt(), CVr_height->GetInt());
		//g_p2D->Sprite_Draw(pDefBackground, NULL, NULL, &p, P3DCOLOR_XRGB(128, 128, 128));
	//g_p2D->Sprite_End();

	// vykresli zakladni nabidku a text menu
	s_MainMenu.Render();

	// vykresli okna
	for(vector<IGuiElement*>::iterator ppElement=g_pWindows.begin(); ppElement != g_pWindows.end(); ppElement++)
	{
		(*ppElement)->m_bHasFocus = false; // uprav ohniska

		// vykresli okna - CUSTOM
		if ((*ppElement)->GetElementFlags() & EF_CUSTOM) (*ppElement)->RenderCustom(deltaTime);
		(*ppElement)->RenderChildren(deltaTime, RT_CUSTOM);

		// vykresli okna - LINE
		//g_p2D->Line_Begin();
		if ((*ppElement)->GetElementFlags() & EF_LINE) (*ppElement)->RenderLine(deltaTime);
		(*ppElement)->RenderChildren(deltaTime, RT_LINE);
		//g_p2D->Line_End();

		// vykresli okna - SPRITE
		g_p2D->Sprite_Begin(SPRITE_ALPHABLEND);
		P3DXMatrix mat;
		mat.SetIdentityMatrix();
		g_p2D->Sprite_SetTransform(mat);
		g_pRenderer->SetTexture(NULL,0);
		g_pRenderer->SetTexture(NULL,1);
		if ((*ppElement)->GetElementFlags() & EF_SPRITE) (*ppElement)->RenderSprite(deltaTime);
		(*ppElement)->RenderChildren(deltaTime, RT_SPRITE);
		/*g_p2D->Sprite_End();*/
		
		if (!(*ppElement)->m_bHidden) (*ppElement)->Render();
	}

	if (g_pWindows.back()) g_pWindows.back()->m_bHasFocus = true; // prvnimu dej ohnisko

	g_pMenu.DrawAndUpdate(deltaTime);

	// GUI EDITOR
	if(CVgui_editor.GetBool())
		g_Scheme._pLabelFontBold->DrawText("GUI - Editor Mode", 10, 10, P3DCOLOR_XRGB(255, 255, 255));

	// vykresli jestli probiha stahovani aktualizaci
	float perc = s_pUpdate->GetPercentDownloaded();
	int cur = s_pUpdate->GetNumDownloaded();
	char str[128];
	if (perc>0.0f && perc<100.0f)
	{
		int num = s_pUpdate->GetNumUpdates();

		if (num == 1)
			sprintf(str, "Downloading updates (%d %%)...", (int)perc);
		else
			sprintf(str, "Downloading updates %d/%d (%d %%)...", cur-1, num, (int)perc);

		g_Scheme._pLabelFontBold->DrawText(str, 10, 20, P3DCOLOR_XRGB(255, 128, 128));
	}
	else if (cur==1)
	{
		sprintf(str, "Update is ready to install...", (int)perc);
		g_Scheme._pLabelFontBold->DrawText(str, 10, 20, P3DCOLOR_XRGB(255, 128, 128));
	}
	else if (cur>1)
	{
		sprintf(str, "%d updates are ready to install...", cur);
		g_Scheme._pLabelFontBold->DrawText(str, 10, 20, P3DCOLOR_XRGB(255, 128, 128));
	}

	// vykresli kurzor mysi
	g_pRenderer->EnableBlending(true);
	g_p2D->DrawTexture(g_Scheme._pCursorTex, g_nMouseX, g_nMouseY, g_nMouseX+16, g_nMouseY+32);
	g_pRenderer->EnableBlending(false);
	/*g_p2D->Line_Begin();
	g_p2D->Line_DrawSimple(g_nMouseX, g_nMouseY, g_nMouseX+10, g_nMouseY+10, 0xffffffff);
	g_p2D->Line_End();*/

	// zjisti zda nebyla zmacknuta polozka hlavniho menu, TODO: menu ve hre - ma resume!
	switch(s_MainMenu.GetSelected())
	{
		case 0: break; // nic :)
		case 1: // New game (resume)
			// new game
			//FIXME: Resume/new game
			bDrawGUI=false;
			CVcl_InMenu.SetValue(bDrawGUI);
			//g_pDia_NewGame->Show();
			break;
		case 2: // Load
			break;
		case 3: // Save
			break;
		case 4: // Options
			g_pDia_Settings->Show();
			break;
		case 5: // Exit
			g_pConsole->Command("quit");
			break;
	}

	return false;
}

/*!
 * \brief
 * Provede aktualizaci GUI systemu a vsech oken, zpracuje uzivatelovy vstupy z klavesnice a mysi.
 * 
 * \param deltaTime
 * Pocet sekund v tomto snimku.
 */
void CGUISystem::Update(float deltaTime)
{
	Prof(GUI_CGUISystem__Update);
	
	// klavesy
	if(g_pInput->IsKeyUpFirstTime(DIK_ESCAPE))
	{
		bDrawGUI=!bDrawGUI;
		if(bDrawGUI)
		{
			pBackground = g_pRenderer->TakeScreenShotTexture();
		}
		else
		{
			SAFE_DELETE(pBackground);
		}
	}
	CVcl_InMenu.SetValue(bDrawGUI);
	
	// aktualizace oken
	if(!bDrawGUI) return;
	
	// aktualizuj kurzor mysi
	g_pInput->GetViewportMousePos(g_nMouseX, g_nMouseY);
	P3DMOUSED mdata = g_pInput->GetMouseData();
	/*mouseX+=mdata.x; // TODO: filtrovat
	mouseY+=mdata.y; // TODO: filtrovat
	if (mouseX < 0) mouseX=0;
	if (mouseY < 0) mouseY=0;
	if (mouseX > CVr_width->GetInt()) mouseX=CVr_width->GetFloat();
	if (mouseY > CVr_height->GetInt()) mouseY=CVr_height->GetFloat();
	g_nMouseX=(int)mouseX; g_nMouseY=(int)mouseY;*/

	// aktualizuj tlacitka mysi
	g_bLMouseLastState = g_bLMouseDown;
	g_bLMouseDown = !!(mdata.b[0]&128);
	g_bLMouseDownFirstTime = g_bLMouseDown && !g_bLMouseLastState;
	g_bLMouseUpFirstTime = !g_bLMouseDown && g_bLMouseLastState;

	// zjisti zda nedoslo ke zmene okna s ohniskem (pokud ovsem aktualni neni modalni)
	IGuiElement* pWin = GetWindowAtPos(g_nMouseX, g_nMouseY);
	IGuiElement* pWinCurrent = GetFirstVisibleWindow();
	if (pWin && pWinCurrent && !((CWindow*)pWinCurrent)->IsModal() && g_bLMouseDownFirstTime)
	{
		if (pWin != g_pWindows.back())
		{
			IGuiElement* pOldBack = g_pWindows.back();
			for(vector<IGuiElement*>::iterator ppElement=g_pWindows.begin(); ppElement != g_pWindows.end(); ppElement++)
			{
				if ((*ppElement) == pWin)
				{
					// prohod tento (nove zvoleny) s byvalym prvnim
					g_pWindows[g_pWindows.size()-1] = pWin;
					(*ppElement) = pOldBack;
					// uprav ohniska
					pOldBack->m_bHasFocus = false;
					pWin->m_bHasFocus = true;
					break;
				}
			}
		}
	}

	// aktualizuji jen prvni = aktivni a viditelny
	IGuiElement* pWinFirstActive = GetFirstVisibleWindow();
	if (pWinFirstActive)
	{
		pWinFirstActive->m_bHasFocus = true;
		pWinFirstActive->Update(deltaTime); //aktualizuj komponenty
		pWinFirstActive->UpdateChildren(deltaTime); //aktualizuj komponenty
		pWinFirstActive->Update(); //aktualizuj konkretni okna
	}
}

CGUISystem::~CGUISystem()
{
	// dialogy
	SAFE_DELETE(g_pDia_NewGame);
	SAFE_DELETE(g_pDia_Settings);
	
	//SAFE_DELETE(test); //DEBUG!!

	SAFE_DELETE(pBackground);
	SAFE_DELETE(pDefBackground);
	s_MainMenu.Shutdown();
	
	// spolecne dialogy
	SAFE_DELETE(dia_SelectFile);
}