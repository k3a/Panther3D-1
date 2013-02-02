#include "frmMain.h"
#include "dlgAbout.h"
#include "common.h"
#include "panMain.h"

frmMain::frmMain( wxWindow* parent )
:
frmMainGui( parent )
{
	g_pFrmMain = this;
	SetIcon(wxIcon("data/editor/app.ico", wxBITMAP_TYPE_ICO));

	m_mgr.AddPane(new panMain(this), wxAuiPaneInfo().
		Name(wxT("main_window")).Caption(wxT("Main Window")).CloseButton(false)
		.Center());

	wxTextCtrl *tx = new wxTextCtrl(this, wxID_ANY);
	m_mgr.AddPane(tx, wxAuiPaneInfo().
		Name(wxT("console")).Caption(wxT("Console"))
		.Bottom());


	wxNotebook *nb = new wxNotebook(this, wxID_ANY);
	wxPanel *pa = new wxPanel(nb, wxID_ANY);
	nb->AddPage(pa, "Create");
	nb->AddPage(pa, "Edit");
	nb->AddPage(pa, "Groups");
	nb->AddPage(pa, "Display");
	m_mgr.AddPane(nb, wxAuiPaneInfo().
		Name(wxT("rollup")).Caption(wxT("Rollup bar"))
		.Right());

	m_mgr.Update();


	// create four viewports
/*	viewLT = new panRender( panTop, LEFT_TOP );
	viewLB = new panRender( panTop, LEFT_BOTTOM );
	viewRT = new panRender( panTop, RIGHT_TOP );
	viewRB = new panRender( panTop, RIGHT_BOTTOM );

	// BASIC 4-VIEWPORT ARRANGEMENT BEGIN
	sizerMainFrame = new wxBoxSizer( wxVERTICAL );

	// 4 viewports arrangement
	sizerFourViewports = new wxFlexGridSizer( 2, 2, 0, 0 );
	sizerFourViewports->AddGrowableCol( 0 );
	sizerFourViewports->AddGrowableCol( 1 );
	sizerFourViewports->AddGrowableRow( 0 );
	sizerFourViewports->AddGrowableRow( 1 );
	sizerFourViewports->SetFlexibleDirection( wxBOTH );
	sizerFourViewports->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	sizerFourViewports->Add( viewLT, 0, wxALL|wxEXPAND, 5 );
	sizerFourViewports->Add( viewRT, 0, wxALL|wxEXPAND, 5 );
	sizerFourViewports->Add( viewLB, 0, wxALL|wxEXPAND, 5 );
	sizerFourViewports->Add( viewRB, 0, wxALL|wxEXPAND, 5 );
	mlastvtype = (eVT)-1;

	// BASIC 4-VIEWPORT ARRANGEMENT END
	sizerMainFrame->Add( sizerFourViewports, 1, wxEXPAND, 3 );
	panTop->SetSizer( sizerMainFrame );

	// LOAD SAVED ARRANGEMENT
	int aid = Settings::GetInt(LAST_FULLSCR_VIEWPORT);
	if (aid>=0) SetFullscreenViewport((eVT)aid);

	// SETTINGS
	SetPosition( wxPoint(Settings::GetInt(WIN_POS_X), Settings::GetInt(WIN_POS_Y)) );
	SetSize( wxSize(Settings::GetInt(WIN_SIZE_X), Settings::GetInt(WIN_SIZE_Y)) );
	if (Settings::GetBool(WIN_MAXIMIZED)) { Maximize(); Refresh(); }
	// menu
	mbDontSwitch = Settings::GetBool(DONT_SWITCH);
	menu->Check(wxID_DONT_SWITCH, mbDontSwitch);
	// toolbar
	miToolType = Settings::GetInt(T_TOOL);
	switch(miToolType)
	{
	case 0: toolbar->ToggleTool(wxID_SELECT, true); break;
	case 1: toolbar->ToggleTool(wxID_MOVE, true); break;
	case 2: toolbar->ToggleTool(wxID_ROTATE, true); break;
	case 3: toolbar->ToggleTool(wxID_SCALE, true); break;
	case 4: toolbar->ToggleTool(wxID_SCALE_SIDES, true); break;
	}

	mbLocal = Settings::GetBool(T_LOCAL);
	mbPhysInEditor = Settings::GetBool(T_PHYS_IN_EDITOR);
	mbOrbit = Settings::GetBool(T_ORBIT);
	mbGrid = Settings::GetBool(T_GRID);
	miGridSize = Settings::GetInt(T_GRIDSIZE);
	miFarPlane = Settings::GetInt(T_FAR_PLANE);
	miCameraSpeed = Settings::GetInt(T_CAMERA_SPEED);
	// set gui
	toolbar->ToggleTool(wxID_LOCAL, mbLocal);
	toolbar->ToggleTool(wxID_PHYS_IN_EDITOR, mbPhysInEditor);
	toolbar->ToggleTool(wxID_ORBIT, mbOrbit);
	toolbar->ToggleTool(wxID_SHOW_GRID, mbGrid);
	toolFarPlane->SetValue(miFarPlane);
	toolCameraSpeed->SetValue(miCameraSpeed);*/
}

void frmMain::OnClose( wxCloseEvent& event )
{
	Hide(); // hide vindow ;)

	// save settings
	wxPoint pnt; wxSize sz;

	pnt = GetPosition(); Settings::Set(WIN_POS_X, pnt.x); Settings::Set(WIN_POS_Y, pnt.y);
	if (!IsMaximized())
	{
		sz = GetSize(); Settings::Set(WIN_SIZE_X, sz.x); Settings::Set(WIN_SIZE_Y, sz.y);
	}
	Settings::Set(WIN_MAXIMIZED, IsMaximized());
	// toolbars
	Settings::Set(T_TOOL, miToolType);
	Settings::Set(T_LOCAL, mbLocal);
	Settings::Set(T_PHYS_IN_EDITOR, mbPhysInEditor);
	Settings::Set(T_ORBIT, mbOrbit);
	Settings::Set(T_GRID, mbGrid);
	Settings::Set(T_GRIDSIZE, miGridSize);
	Settings::Set(T_FAR_PLANE, miFarPlane);
	Settings::Set(T_CAMERA_SPEED, miCameraSpeed);

	viewLT->Destroying();
	viewRT->Destroying();
	viewLB->Destroying();
	viewRB->Destroying();

	// if there was fullscreen viewport, switch to 4viewports mode to make correct hierarchy to free memory
	if (mlastvtype>=0) 
	{
		eVT tmp = mlastvtype;
		SetFourViewportsArrangement( mlastvtype );
		Settings::Set(LAST_FULLSCR_VIEWPORT, tmp);
	}

	// exit app
	wxTheApp->Exit();
}

static int sash=0;
void frmMain::OnUpdateUI( wxUpdateUIEvent& event )
{
}

void frmMain::SetFullscreenViewport( eVT vtype )
{
	mlastvtype = vtype;
	Settings::Set(LAST_FULLSCR_VIEWPORT, mlastvtype);

	// hide all first
	viewLT->Hide();
	viewRT->Hide();
	viewLB->Hide();
	viewRB->Hide();

	panRender* pRen;
	switch(vtype)
	{
	case LEFT_TOP: pRen = viewLT; break;
	case LEFT_BOTTOM: pRen = viewLB; break;
	case RIGHT_TOP: pRen = viewRT; break;
	default: pRen = viewRB; break;
	}
	pRen->Show();
	pRen->CheckToolbar(wxID_MAXIMIZE, true);

	sizerMainFrame->Detach(0);
	sizerMainFrame->Add( pRen, 1, wxEXPAND, 5 );
	sizerMainFrame->Layout();

	SetActiveViewport(vtype);
}

void frmMain::SetFourViewportsArrangement( eVT vtype )
{
	mlastvtype = (eVT)-1;
	Settings::Set(LAST_FULLSCR_VIEWPORT, mlastvtype);
	// show all first
	viewLT->Show(); viewLT->CheckToolbar(wxID_MAXIMIZE, false);
	viewRT->Show(); viewRT->CheckToolbar(wxID_MAXIMIZE, false);
	viewLB->Show(); viewLB->CheckToolbar(wxID_MAXIMIZE, false);
	viewRB->Show(); viewRB->CheckToolbar(wxID_MAXIMIZE, false);

	panRender* pRen;
	switch(vtype)
	{
	case LEFT_TOP: pRen = viewLT; break;
	case LEFT_BOTTOM: pRen = viewLB; break;
	case RIGHT_TOP: pRen = viewRT; break;
	default: pRen = viewRB; break;
	}

	sizerMainFrame->Detach(0);
	sizerMainFrame->Add( sizerFourViewports, 1, wxEXPAND, 3 );
	pRen->SetContainingSizer(sizerFourViewports);

	sizerMainFrame->Layout();
}

void frmMain::SetActiveViewport(eVT vtype)
{
	viewLT->SetActive(false);
	viewRT->SetActive(false);
	viewLB->SetActive(false);
	viewRB->SetActive(false);
	panRender* pRen;
	switch(vtype)
	{
	case LEFT_TOP: pRen = viewLT; break;
	case LEFT_BOTTOM: pRen = viewLB; break;
	case RIGHT_TOP: pRen = viewRT; break;
	default: pRen = viewRB; break;
	}
	pRen->SetActive(true);
}

void frmMain::btnFindEntityPressed( wxCommandEvent& event )
{
	// TODO: Implement btnFindEntityPressed
}

void frmMain::txtPositionEnterPressed( wxCommandEvent& event )
{
	// TODO: Implement txtPositionEnterPressed
}

void frmMain::tabEventsPickToggled( wxCommandEvent& event )
{
	// TODO: Implement tabEventsPickToggled
}

void frmMain::tabEventsParamLeaveFocus( wxFocusEvent& event )
{
	// TODO: Implement tabEventsParamLeaveFocus
}

void frmMain::tabEventsParamEnterFocus( wxFocusEvent& event )
{
	// TODO: Implement tabEventsParamEnterFocus
}

void frmMain::tabEventsParamOnEnter( wxCommandEvent& event )
{
	// TODO: Implement tabEventsParamOnEnter
}

void frmMain::tabEventsDelayOnEnter( wxCommandEvent& event )
{
	// TODO: Implement tabEventsDelayOnEnter
}

void frmMain::tabEventsAddPressed( wxCommandEvent& event )
{
	// TODO: Implement tabEventsAddPressed
}

void frmMain::tabEventsCopyPressed( wxCommandEvent& event )
{
	// TODO: Implement tabEventsCopyPressed
}

void frmMain::tabEventsPastePressed( wxCommandEvent& event )
{
	// TODO: Implement tabEventsPastePressed
}

void frmMain::tabEventsDeletePressed( wxCommandEvent& event )
{
	// TODO: Implement tabEventsDeletePressed
}

void frmMain::tabCreatePickPosPressed( wxCommandEvent& event )
{
	// TODO: Implement tabCreatePickPosPressed
}

void frmMain::tabCreateEntitiesDblClick( wxListEvent& event )
{
	// TODO: Implement tabCreateEntitiesDblClick
}

void frmMain::tabCreateFindOnTexChanged( wxCommandEvent& event )
{
	// TODO: Implement tabCreateFindOnTexChanged
}

void frmMain::tabCreateFindEnterPressed( wxCommandEvent& event )
{
	// TODO: Implement tabCreateFindEnterPressed
}

void frmMain::tabCreateCreateBtnPressed( wxCommandEvent& event )
{
	// TODO: Implement tabCreateCreateBtnPressed
}

void frmMain::tabGroupsSelectedOnCombo( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsSelectedOnCombo
}

void frmMain::tabGroupsSelectedOnKeyUp( wxKeyEvent& event )
{
	// TODO: Implement tabGroupsSelectedOnKeyUp
}

void frmMain::tabGroupsSelectedOnText( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsSelectedOnText
}

void frmMain::tabGroupsSelectedOnEnterCreate( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsSelectedOnEnterCreate
}

void frmMain::tabGroupsCreateNewPressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsCreateNewPressed
}

void frmMain::tabGroupsDeletePressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsDeletePressed
}

void frmMain::tabGroupsAddSelectionToGroupPressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsAddSelectionToGroupPressed
}

void frmMain::tabGroupsSelectAllPressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsSelectAllPressed
}

void frmMain::tabGroupsListDblClick( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsListDblClick
}

void frmMain::tabGroupsMakeSelectionFromHighlightedPressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsMakeSelectionFromHighlightedPressed
}

void frmMain::tabGroupsRemoveHighlightedPressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsRemoveHighlightedPressed
}

void frmMain::tabGroupsClearListPressed( wxCommandEvent& event )
{
	// TODO: Implement tabGroupsClearListPressed
}

void frmMain::toolNewClicked( wxCommandEvent& event )
{
	// TODO: Implement toolNewClicked
}

void frmMain::toolOpenClicked( wxCommandEvent& event )
{
	// TODO: Implement toolOpenClicked
}

void frmMain::toolSaveClicked( wxCommandEvent& event )
{
	// TODO: Implement toolSaveClicked
}

void frmMain::toolSelectClicked( wxCommandEvent& event )
{
	miToolType=0;
}

void frmMain::toolMoveClicked( wxCommandEvent& event )
{
	miToolType=1;
}

void frmMain::toolRotateClicked( wxCommandEvent& event )
{
	miToolType=2;
}

void frmMain::toolScaleClicked( wxCommandEvent& event )
{
	miToolType=3;
}

void frmMain::toolScaleSidesClicked( wxCommandEvent& event )
{
	miToolType=4;
}

void frmMain::toolLocalClicked( wxCommandEvent& event )
{
	mbLocal = event.IsChecked();
}

void frmMain::toolPhysInEditorClicked( wxCommandEvent& event )
{
	mbPhysInEditor = event.IsChecked();
}

void frmMain::toolOrbitClicked( wxCommandEvent& event )
{
	mbOrbit = event.IsChecked();
}

void frmMain::toolShowGridClicked( wxCommandEvent& event )
{
	mbGrid = event.IsChecked();
}

void frmMain::toolGridMinusClicked( wxCommandEvent& event )
{
	miGridSize /= 2;
	if (miGridSize<1) miGridSize=1;
}

void frmMain::toolGridPlusClicked( wxCommandEvent& event )
{
	miGridSize *= 2;
	if (miGridSize>512) miGridSize=512;
}


void frmMain::toolFarPlaneThumbScrollReleased( wxScrollEvent& event )
{
	miFarPlane = toolFarPlane->GetValue();
}

void frmMain::toolCameraSpeedScrollReleased( wxScrollEvent& event )
{
	miCameraSpeed = toolCameraSpeed->GetValue();
}

void frmMain::toolPlayClicked( wxCommandEvent& event )
{
	// TODO: Implement toolPlayClicked
}

void frmMain::toolAssetBrowserClicked( wxCommandEvent& event )
{
	// TODO: Implement toolAssetBrowserClicked
}

void frmMain::toolMaterialEditorClicked( wxCommandEvent& event )
{
	// TODO: Implement toolMaterialEditorClicked
}

void frmMain::toolModelEditorClicked( wxCommandEvent& event )
{
	// TODO: Implement toolModelEditorClicked
}

void frmMain::toolParticleEditorClicked( wxCommandEvent& event )
{
	// TODO: Implement toolParticleEditorClicked
}

void frmMain::toolConsoleEnterPressed( wxCommandEvent& event )
{
	if (g_pConsole) g_pConsole->Command(toolConsole->GetLineText(0));
	toolConsole->Clear();
	this->SetFocus();
}

void frmMain::mnuFileNewClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuFileNewClicked
}

void frmMain::mnuFileOpenClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuFileOpenClicked
}

void frmMain::mnuFileSaveClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuFileSaveClicked
}

void frmMain::mnuFileSaveAsClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuFileSaveAsClicked
}

void frmMain::mnuFileExitClicked( wxCommandEvent& event )
{
	OnClose(wxCloseEvent());
}

void frmMain::mnuEditUndoClicked( wxCommandEvent& event )
{
	//
}

void frmMain::mnuEditRedoClicked( wxCommandEvent& event )
{
	//
}

void frmMain::mnuEditCopyClicked( wxCommandEvent& event )
{
	//
}

void frmMain::mnuEditPasteClicked( wxCommandEvent& event )
{
	//
}

void frmMain::mnuEditDeleteClicked( wxCommandEvent& event )
{
	//
}


void frmMain::mnuViewHideSelectedClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuViewHideSelectedClicked
}

void frmMain::mnuViewShowHiddenClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuViewShowHiddenClicked
}

void frmMain::mnuDontSwitchToPropertiesClicked( wxCommandEvent& event )
{
	mbDontSwitch = event.IsChecked();
	Settings::Set(DONT_SWITCH, mbDontSwitch);
}

void frmMain::mnuMapPropertiesClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuMapPropertiesClicked
}

void frmMain::mnuMapPlayClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuMapPlayClicked
}

void frmMain::mnuToolsAssetBrowserClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuToolsAssetBrowserClicked
}

void frmMain::mnuToolsMaterialEditorClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuToolsMaterialEditorClicked
}

void frmMain::mnuToolsModelViewerClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuToolsModelViewerClicked
}

void frmMain::mnuParticleEditorClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuParticleEditorClicked
}

void frmMain::mnuHelpOnlineClicked( wxCommandEvent& event )
{
	// TODO: Implement mnuHelpOnlineClicked
}

void frmMain::mnuHelpAboutClicked( wxCommandEvent& event )
{
	dlgAbout *pDlg = new dlgAbout(this);
	pDlg->ShowModal();
}

panRender* frmMain::GetViewport( eVT vtype )
{
	switch(vtype)
	{
	case LEFT_TOP: return viewLT;
	case LEFT_BOTTOM: return viewLB;
	case RIGHT_TOP: return viewRT;
	default: return viewRB;
	}
}

void frmMain::AddConsoleMsg(const char* text, const char* scopeName)
{
	/*txtConsole->AppendText(wxString(text)+wxString("\r\n"));
	txtConsole->SetSelection(txtConsole->GetLastPosition(), txtConsole->GetLastPosition());*/
}