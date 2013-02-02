///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 20 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "gui.h"

///////////////////////////////////////////////////////////////////////////

frmMainGui::frmMainGui( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	m_mgr.SetManagedWindow(this);

	this->SetMinSize(wxSize(400,300));
	
	// Toolbar - Standard ------------------
	toolbarStandard = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	toolbarStandard->AddTool( wxID_NEW, wxT("Create new map"), wxBitmap( wxT("data/editor/new.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("New Map"), wxT("New Map") );
	toolbarStandard->AddTool( wxID_OPEN, wxT("Open map..."), wxBitmap( wxT("data/editor/open.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Open Map..."), wxT("Open Map...") );
	toolbarStandard->AddTool( wxID_SAVE, wxT("Save current map"), wxBitmap( wxT("data/editor/save.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Save Current Map"), wxT("Save Current Map") );
	toolbarStandard->AddSeparator();
	toolbarStandard->AddTool( wxID_UNDO, wxT("tool"), wxBitmap( wxT("data/editor/undo.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Undo changes"), wxT("Undo changes") );
	toolbarStandard->AddTool( wxID_REDO, wxT("tool"), wxBitmap( wxT("data/editor/redo.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Redo changes"), wxT("Redo changes") );
	toolbarStandard->Realize();
	m_mgr.AddPane(toolbarStandard, wxAuiPaneInfo().
		Name(wxT("toolbar_standard")).Caption(wxT("Standard")).
		ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

	// Toolbar - Manipulators ------------------
	toolbarManipulators = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	toolbarManipulators->AddTool( wxID_SELECT, wxT("Selection tool"), wxBitmap( wxT("data/editor/select.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Selection tool"), wxT("Selection tool") );
	toolbarManipulators->AddTool( wxID_MOVE, wxT("Move tool"), wxBitmap( wxT("data/editor/move.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Move tool"), wxT("Move tool") );
	toolbarManipulators->AddTool( wxID_ROTATE, wxT("Rotation tool"), wxBitmap( wxT("data/editor/rotate.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Rotation tool"), wxT("Rotation tool") );
	toolbarManipulators->AddTool( wxID_SCALE, wxT("Scale tool"), wxBitmap( wxT("data/editor/scale.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Scale tool"), wxT("Scale tool") );
	toolbarManipulators->AddTool( wxID_SCALE_SIDES, wxT("ScaleSides tool"), wxBitmap( wxT("data/editor/scalesides.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("ScaleSides tool"), wxT("ScaleSides tool") );
	toolbarManipulators->AddTool( wxID_LOCAL, wxT("tool"), wxBitmap( wxT("data/editor/local.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Use local (object) space for manipulation"), wxT("Use local (object) space for manipulation") );
	toolbarManipulators->Realize();
	m_mgr.AddPane(toolbarManipulators, wxAuiPaneInfo().
		Name(wxT("toolbar_manipulators")).Caption(wxT("Manipulators")).
		ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));


	// Toolbar - Additional ------------------
	toolbarAdditional = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	toolbarAdditional->AddTool( wxID_PHYS_IN_EDITOR, wxT("tool"), wxBitmap( wxT("data/editor/physInEditor.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Enable collisions in editor"), wxT("Enable collisions in editor") );
	toolbarAdditional->AddTool( wxID_ORBIT, wxT("tool"), wxBitmap( wxT("data/editor/orbit.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Toggle orbit view"), wxT("Toggle orbit view") );
	toolbarAdditional->AddTool( wxID_SHOW_GRID, wxT("tool"), wxBitmap( wxT("data/editor/grid.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Show grid"), wxT("Show grid") );
	toolbarAdditional->AddTool( wxID_GRID_MINUS, wxT("tool"), wxBitmap( wxT("data/editor/gridminus.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Smaller grid cells"), wxT("Smaller grid cells") );
	toolbarAdditional->AddTool( wxID_GRID_PLUS, wxT("tool"), wxBitmap( wxT("data/editor/gridplus.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Bigger grid cells"), wxT("Bigger grid cells") );
	toolFarPlane = new wxSlider( toolbarAdditional, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( 60,-1 ), wxSL_HORIZONTAL );
	toolFarPlane->SetToolTip( wxT("Far plane distance") );
	toolbarAdditional->AddControl( toolFarPlane );
	toolCameraSpeed = new wxSlider( toolbarAdditional, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( 60,-1 ), wxSL_HORIZONTAL );
	toolCameraSpeed->SetToolTip( wxT("Camera speed") );
	toolbarAdditional->AddControl( toolCameraSpeed );
	toolbarAdditional->AddSeparator();
	toolbarAdditional->AddTool( wxID_PLAY, wxT("Play!"), wxBitmap( wxT("data/editor/play.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Play!"), wxT("Play!") );
	toolbarAdditional->Realize();
	m_mgr.AddPane(toolbarAdditional, wxAuiPaneInfo().
		Name(wxT("toolbar_additional")).Caption(wxT("Additional")).
		ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

	// Toolbar - Tools ------------------
	toolbarTools = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	toolbarTools->AddTool( wxID_ASSET_BROWSER, wxT("tool"), wxBitmap( wxT("data/editor/assetbrowser.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Asset Browser"), wxT("Show Asset Browser") );
	toolbarTools->AddTool( wxID_MATERIAL_BROWSER, wxT("tool"), wxBitmap( wxT("data/editor/materialeditor.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Material Editor"), wxT("Show Material Editor") );
	toolbarTools->AddTool( wxID_MODEL_EDITOR, wxT("tool"), wxBitmap( wxT("data/editor/modeleditor.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Model Editor"), wxT("Show Model Editor") );
	toolbarTools->AddTool( wxID_PARTICLE_EDITOR, wxT("tool"), wxBitmap( wxT("data/editor/particleeditor.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Particle Editor"), wxT("Show Particle Editor") );
	toolbarTools->Realize();
	m_mgr.AddPane(toolbarTools, wxAuiPaneInfo().
		Name(wxT("toolbar_tools")).Caption(wxT("Tools")).
		ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));

	// Toolbar - Console ------------------
	toolbarConsole = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
	m_staticText314 = new wxStaticText( toolbarConsole, wxID_ANY, wxT("Console:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText314->Wrap( -1 );
	toolbarConsole->AddControl( m_staticText314 );
	toolConsole = new wxTextCtrl( toolbarConsole, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 98,-1 ), wxTE_PROCESS_ENTER );
	toolbarConsole->AddControl( toolConsole );
	toolbarConsole->Realize();
	m_mgr.AddPane(toolbarConsole, wxAuiPaneInfo().
		Name(wxT("toolbar_console")).Caption(wxT("Console")).
		ToolbarPane().Top().
		LeftDockable(false).RightDockable(false));







	
	menu = new wxMenuBar( 0 );
	mnuFile = new wxMenu();
	wxMenuItem* mnuFileNew;
	mnuFileNew = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("&New") ) + wxT('\t') + wxT("CTRL+N"), wxT("Create new map"), wxITEM_NORMAL );
	mnuFile->Append( mnuFileNew );
	
	wxMenuItem* mnuFileOpen;
	mnuFileOpen = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("&Open...") ) + wxT('\t') + wxT("CTRL+O"), wxT("Open Map File..."), wxITEM_NORMAL );
	mnuFile->Append( mnuFileOpen );
	
	wxMenuItem* mnuFileSave;
	mnuFileSave = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Save") ) + wxT('\t') + wxT("CTRL+S"), wxT("Save current map"), wxITEM_NORMAL );
	mnuFile->Append( mnuFileSave );
	
	wxMenuItem* mnuFileSaveAs;
	mnuFileSaveAs = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Save &As...") ) + wxT('\t') + wxT("CTRL+SHIFT+S"), wxT("Save current map as..."), wxITEM_NORMAL );
	mnuFile->Append( mnuFileSaveAs );
	
	mnuFile->AppendSeparator();
	
	wxMenuItem* mnuFileExit;
	mnuFileExit = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("&Exit") ) , wxT("Exit P3DCrane"), wxITEM_NORMAL );
	mnuFile->Append( mnuFileExit );
	
	menu->Append( mnuFile, wxT("&File") );
	
	mnuEdit = new wxMenu();
	wxMenuItem* mnuEditUndo;
	mnuEditUndo = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("&Undo") ) + wxT('\t') + wxT("CTRL+Z"), wxT("Undo changes"), wxITEM_NORMAL );
	mnuEdit->Append( mnuEditUndo );
	
	wxMenuItem* mnuEditRedo;
	mnuEditRedo = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("&Redo") ) + wxT('\t') + wxT("CTRL+Y"), wxT("Redo changes"), wxITEM_NORMAL );
	mnuEdit->Append( mnuEditRedo );
	
	mnuEdit->AppendSeparator();
	
	wxMenuItem* mnuEditCopy;
	mnuEditCopy = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("&Copy") ) + wxT('\t') + wxT("CTRL+C"), wxT("Copy selected"), wxITEM_NORMAL );
	mnuEdit->Append( mnuEditCopy );
	
	wxMenuItem* mnuEditPaste;
	mnuEditPaste = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("&Paste") ) + wxT('\t') + wxT("CTRL+V"), wxT("Paste copied"), wxITEM_NORMAL );
	mnuEdit->Append( mnuEditPaste );
	
	wxMenuItem* mnuEditDelete;
	mnuEditDelete = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("&Delete") ) + wxT('\t') + wxT("CTRL+D"), wxT("Delete selected"), wxITEM_NORMAL );
	mnuEdit->Append( mnuEditDelete );
	
	menu->Append( mnuEdit, wxT("&Edit") );
	
	mnuView = new wxMenu();
	wxMenuItem* mnuViewHideSelected;
	mnuViewHideSelected = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Hide Selected Entities") ) + wxT('\t') + wxT("H"), wxT("Hide current selection"), wxITEM_NORMAL );
	mnuView->Append( mnuViewHideSelected );
	
	wxMenuItem* mnuViewShowHidden;
	mnuViewShowHidden = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Show Hidden Entities") ) + wxT('\t') + wxT("SHIFT+H"), wxT("Show all hidden entities"), wxITEM_NORMAL );
	mnuView->Append( mnuViewShowHidden );
	
	mnuView->AppendSeparator();
	
	wxMenuItem* mnuDontSwitchToProperties;
	mnuDontSwitchToProperties = new wxMenuItem( mnuView, wxID_DONT_SWITCH, wxString( wxT("Don't Switch to Properties") ) , wxT("Don't Switch to Properties after entity creation"), wxITEM_CHECK );
	mnuView->Append( mnuDontSwitchToProperties );
	
	menu->Append( mnuView, wxT("&View") );
	
	mnuMap = new wxMenu();
	wxMenuItem* mnuMapProperties;
	mnuMapProperties = new wxMenuItem( mnuMap, wxID_ANY, wxString( wxT("Edit &Properties...") ) , wxT("Edit Map Properties..."), wxITEM_NORMAL );
	mnuMap->Append( mnuMapProperties );
	
	wxMenuItem* mnuMapPlay;
	mnuMapPlay = new wxMenuItem( mnuMap, wxID_ANY, wxString( wxT("Play!") ) , wxT("Play This Map!"), wxITEM_NORMAL );
	mnuMap->Append( mnuMapPlay );
	
	menu->Append( mnuMap, wxT("&Map") );
	
	mnuTools = new wxMenu();
	wxMenuItem* mnuToolsAssetBrowser;
	mnuToolsAssetBrowser = new wxMenuItem( mnuTools, wxID_ANY, wxString( wxT("&Asset Browser...") ) + wxT('\t') + wxT("CTRL+A"), wxT("Browse game assets..."), wxITEM_NORMAL );
	mnuTools->Append( mnuToolsAssetBrowser );
	
	wxMenuItem* mnuToolsMaterialEditor;
	mnuToolsMaterialEditor = new wxMenuItem( mnuTools, wxID_ANY, wxString( wxT("&Material Editor...") ) , wxT("Create and edit materials, assign textures, ..."), wxITEM_NORMAL );
	mnuTools->Append( mnuToolsMaterialEditor );
	
	wxMenuItem* mnuToolsModelViewer;
	mnuToolsModelViewer = new wxMenuItem( mnuTools, wxID_ANY, wxString( wxT("Model &Viewer...") ) , wxT("Edit game models, create physics shape, events, ..."), wxITEM_NORMAL );
	mnuTools->Append( mnuToolsModelViewer );
	
	wxMenuItem* mnuToolsParticleEditor;
	mnuToolsParticleEditor = new wxMenuItem( mnuTools, wxID_ANY, wxString( wxT("&Particle Editor...") ) , wxT("Create and edit particle effects..."), wxITEM_NORMAL );
	mnuTools->Append( mnuToolsParticleEditor );
	
	menu->Append( mnuTools, wxT("&Tools") );
	
	mnuHelp = new wxMenu();
	wxMenuItem* mnuHelpOnline;
	mnuHelpOnline = new wxMenuItem( mnuHelp, wxID_ANY, wxString( wxT("Online &Help...") ) + wxT('\t') + wxT("F1"), wxT("Open browser window with online help..."), wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpOnline );
	
	wxMenuItem* mnuHelpAbout;
	mnuHelpAbout = new wxMenuItem( mnuHelp, wxID_ANY, wxString( wxT("&About P3DCrane...") ) , wxT("Show information about editor..."), wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpAbout );
	
	menu->Append( mnuHelp, wxT("&Help") );
	
	this->SetMenuBar( menu );
	
	
	this->Centre( wxBOTH );

	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( frmMainGui::OnClose ) );
	this->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( frmMainGui::OnUpdateUI ) );
	this->Connect( wxID_NEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolNewClicked ) );
	this->Connect( wxID_OPEN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolOpenClicked ) );
	this->Connect( wxID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolSaveClicked ) );
	this->Connect( wxID_SELECT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolSelectClicked ) );
	this->Connect( wxID_MOVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolMoveClicked ) );
	this->Connect( wxID_ROTATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolRotateClicked ) );
	this->Connect( wxID_SCALE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolScaleClicked ) );
	this->Connect( wxID_SCALE_SIDES, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolScaleSidesClicked ) );
	this->Connect( wxID_LOCAL, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolLocalClicked ) );
	this->Connect( wxID_PHYS_IN_EDITOR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolPhysInEditorClicked ) );
	this->Connect( wxID_ORBIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolOrbitClicked ) );
	this->Connect( wxID_SHOW_GRID, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolShowGridClicked ) );
	this->Connect( wxID_GRID_MINUS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolGridMinusClicked ) );
	this->Connect( wxID_GRID_PLUS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolGridPlusClicked ) );
	toolFarPlane->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( frmMainGui::toolFarPlaneThumbScrollReleased ), NULL, this );
	toolCameraSpeed->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( frmMainGui::toolCameraSpeedScrollReleased ), NULL, this );
	this->Connect( wxID_PLAY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolPlayClicked ) );
	this->Connect( wxID_ASSET_BROWSER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolAssetBrowserClicked ) );
	this->Connect( wxID_MATERIAL_BROWSER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolMaterialEditorClicked ) );
	this->Connect( wxID_MODEL_EDITOR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolModelEditorClicked ) );
	this->Connect( wxID_PARTICLE_EDITOR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolParticleEditorClicked ) );
	toolConsole->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( frmMainGui::toolConsoleEnterPressed ), NULL, this );
	this->Connect( mnuFileNew->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileNewClicked ) );
	this->Connect( mnuFileOpen->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileOpenClicked ) );
	this->Connect( mnuFileSave->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileSaveClicked ) );
	this->Connect( mnuFileSaveAs->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileSaveAsClicked ) );
	this->Connect( mnuFileExit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileExitClicked ) );
	this->Connect( mnuEditUndo->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditUndoClicked ) );
	this->Connect( mnuEditRedo->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditRedoClicked ) );
	this->Connect( mnuEditCopy->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditCopyClicked ) );
	this->Connect( mnuEditPaste->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditPasteClicked ) );
	this->Connect( mnuEditDelete->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditDeleteClicked ) );
	this->Connect( mnuViewHideSelected->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuViewHideSelectedClicked ) );
	this->Connect( mnuViewShowHidden->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuViewShowHiddenClicked ) );
	this->Connect( mnuDontSwitchToProperties->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuDontSwitchToPropertiesClicked ) );
	this->Connect( mnuMapProperties->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuMapPropertiesClicked ) );
	this->Connect( mnuMapPlay->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuMapPlayClicked ) );
	this->Connect( mnuToolsAssetBrowser->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuToolsAssetBrowserClicked ) );
	this->Connect( mnuToolsMaterialEditor->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuToolsMaterialEditorClicked ) );
	this->Connect( mnuToolsModelViewer->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuToolsModelViewerClicked ) );
	this->Connect( mnuToolsParticleEditor->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuParticleEditorClicked ) );
	this->Connect( mnuHelpOnline->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuHelpOnlineClicked ) );
	this->Connect( mnuHelpAbout->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuHelpAboutClicked ) );
}

frmMainGui::~frmMainGui()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( frmMainGui::OnClose ) );
	this->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( frmMainGui::OnUpdateUI ) );
	this->Disconnect( wxID_NEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolNewClicked ) );
	this->Disconnect( wxID_OPEN, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolOpenClicked ) );
	this->Disconnect( wxID_SAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolSaveClicked ) );
	this->Disconnect( wxID_SELECT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolSelectClicked ) );
	this->Disconnect( wxID_MOVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolMoveClicked ) );
	this->Disconnect( wxID_ROTATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolRotateClicked ) );
	this->Disconnect( wxID_SCALE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolScaleClicked ) );
	this->Disconnect( wxID_SCALE_SIDES, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolScaleSidesClicked ) );
	this->Disconnect( wxID_LOCAL, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolLocalClicked ) );
	this->Disconnect( wxID_PHYS_IN_EDITOR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolPhysInEditorClicked ) );
	this->Disconnect( wxID_ORBIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolOrbitClicked ) );
	this->Disconnect( wxID_SHOW_GRID, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolShowGridClicked ) );
	this->Disconnect( wxID_GRID_MINUS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolGridMinusClicked ) );
	this->Disconnect( wxID_GRID_PLUS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolGridPlusClicked ) );
	toolFarPlane->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( frmMainGui::toolFarPlaneThumbScrollReleased ), NULL, this );
	toolCameraSpeed->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( frmMainGui::toolCameraSpeedScrollReleased ), NULL, this );
	this->Disconnect( wxID_PLAY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolPlayClicked ) );
	this->Disconnect( wxID_ASSET_BROWSER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolAssetBrowserClicked ) );
	this->Disconnect( wxID_MATERIAL_BROWSER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolMaterialEditorClicked ) );
	this->Disconnect( wxID_MODEL_EDITOR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolModelEditorClicked ) );
	this->Disconnect( wxID_PARTICLE_EDITOR, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( frmMainGui::toolParticleEditorClicked ) );
	toolConsole->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( frmMainGui::toolConsoleEnterPressed ), NULL, this );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileNewClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileOpenClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileSaveClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileSaveAsClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuFileExitClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditUndoClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditRedoClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditCopyClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditPasteClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuEditDeleteClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuViewHideSelectedClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuViewShowHiddenClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuDontSwitchToPropertiesClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuMapPropertiesClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuMapPlayClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuToolsAssetBrowserClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuToolsMaterialEditorClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuToolsModelViewerClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuParticleEditorClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuHelpOnlineClicked ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( frmMainGui::mnuHelpAboutClicked ) );
}

dlgSplashGui::dlgSplashGui( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer277;
	bSizer277 = new wxBoxSizer( wxVERTICAL );
	
	m_bitmap8 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("data/editor/splash.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer277->Add( m_bitmap8, 0, wxALL, 0 );
	
	txtInfo = new wxStaticText( this, wxID_ANY, wxT(" Panther3D Engine 1.1.1 | Game"), wxPoint( -1,-1 ), wxSize( 441,-1 ), 0|wxDOUBLE_BORDER );
	txtInfo->Wrap( -1 );
	bSizer277->Add( txtInfo, 0, wxALL, 4 );
	
	this->SetSizer( bSizer277 );
	this->Layout();
}

dlgSplashGui::~dlgSplashGui()
{
}

panRenderGui::panRenderGui( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	
	wxBoxSizer* bSizer279;
	bSizer279 = new wxBoxSizer( wxVERTICAL );
	
	m_toolBar2 = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL ); 
	m_toolBar2->AddTool( wxID_MAXIMIZE, wxT("Maximize this viewport"), wxBitmap( wxT("data/editor/maximize.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Maximize this viewport"), wxT("Maximize this viewport") );
	m_toolBar2->AddTool( wxID_RIGHT, wxT("Right view"), wxBitmap( wxT("data/editor/right.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Right view"), wxT("Right view") );
	m_toolBar2->AddTool( wxID_TOP, wxT("Top view"), wxBitmap( wxT("data/editor/top.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Top view"), wxT("Top view") );
	m_toolBar2->AddTool( wxID_FRONT, wxT("Front view"), wxBitmap( wxT("data/editor/front.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Front view"), wxT("Front view") );
	m_toolBar2->AddTool( wxID_PERSPECT, wxT("Perspective view"), wxBitmap( wxT("data/editor/perspective.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Perspective view"), wxT("Perspective view") );
	m_toolBar2->AddSeparator();
	m_toolBar2->AddTool( wxID_WIRE, wxT("Wireframe mode"), wxBitmap( wxT("data/editor/wireframe.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Wireframe mode"), wxT("Wireframe mode") );
	m_toolBar2->AddTool( wxID_TEXT, wxT("Textured mode"), wxBitmap( wxT("data/editor/textured.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Textured mode"), wxT("Textured mode") );
	m_toolBar2->AddTool( wxID_LIT, wxT("Lit mode"), wxBitmap( wxT("data/editor/lit.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_RADIO, wxT("Lit mode"), wxT("Lit mode") );
	m_toolBar2->AddSeparator();
	m_toolBar2->AddTool( wxID_REALTIME, wxT("Toggle realtime rendering"), wxBitmap( wxT("data/editor/realtime.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_CHECK, wxT("Toggle realtime rendering"), wxT("Toggle realtime rendering") );
	m_toolBar2->AddTool( wxID_RENDER, wxT("tool"), wxBitmap( wxT("data/editor/more.ico"), wxBITMAP_TYPE_ANY ), wxNullBitmap, wxITEM_NORMAL, wxT("Render parameters"), wxT("Render parameters") );
	m_toolBar2->Realize();
	
	bSizer279->Add( m_toolBar2, 0, wxEXPAND, 5 );
	
	panViewport = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	panViewport->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	
	bSizer279->Add( panViewport, 1, wxEXPAND | wxALL, 2 );
	
	this->SetSizer( bSizer279 );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_IDLE, wxIdleEventHandler( panRenderGui::OnIdle ) );
	this->Connect( wxID_MAXIMIZE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolMaximizeClicked ) );
	this->Connect( wxID_RIGHT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolRightClicked ) );
	this->Connect( wxID_TOP, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolTopClicked ) );
	this->Connect( wxID_FRONT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolFrontClicked ) );
	this->Connect( wxID_PERSPECT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolPerspectClicked ) );
	this->Connect( wxID_WIRE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolWireClicked ) );
	this->Connect( wxID_TEXT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolTextClicked ) );
	this->Connect( wxID_LIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolLitClicked ) );
	this->Connect( wxID_REALTIME, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolRealtimeClicked ) );
	this->Connect( wxID_RENDER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolRenderClicked ) );
	panViewport->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( panRenderGui::OnMouse ), NULL, this );
	panViewport->Connect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( panRenderGui::OnMouse ), NULL, this );
	panViewport->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( panRenderGui::OnMouse ), NULL, this );
}

panRenderGui::~panRenderGui()
{
	// Disconnect Events
	this->Disconnect( wxEVT_IDLE, wxIdleEventHandler( panRenderGui::OnIdle ) );
	this->Disconnect( wxID_MAXIMIZE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolMaximizeClicked ) );
	this->Disconnect( wxID_RIGHT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolRightClicked ) );
	this->Disconnect( wxID_TOP, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolTopClicked ) );
	this->Disconnect( wxID_FRONT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolFrontClicked ) );
	this->Disconnect( wxID_PERSPECT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolPerspectClicked ) );
	this->Disconnect( wxID_WIRE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolWireClicked ) );
	this->Disconnect( wxID_TEXT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolTextClicked ) );
	this->Disconnect( wxID_LIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolLitClicked ) );
	this->Disconnect( wxID_REALTIME, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolRealtimeClicked ) );
	this->Disconnect( wxID_RENDER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( panRenderGui::toolRenderClicked ) );
	panViewport->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( panRenderGui::OnMouse ), NULL, this );
	panViewport->Disconnect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( panRenderGui::OnMouse ), NULL, this );
	panViewport->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( panRenderGui::OnMouse ), NULL, this );
}

dlgAboutGui::dlgAboutGui( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer91;
	fgSizer91 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer91->AddGrowableCol( 0 );
	fgSizer91->AddGrowableRow( 0 );
	fgSizer91->SetFlexibleDirection( wxBOTH );
	fgSizer91->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer94;
	sbSizer94 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer93;
	fgSizer93 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer93->AddGrowableCol( 1 );
	fgSizer93->AddGrowableRow( 0 );
	fgSizer93->SetFlexibleDirection( wxBOTH );
	fgSizer93->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_bitmap9 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("data/editor/about.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer93->Add( m_bitmap9, 0, wxALL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer94;
	fgSizer94 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer94->AddGrowableCol( 0 );
	fgSizer94->AddGrowableRow( 1 );
	fgSizer94->SetFlexibleDirection( wxVERTICAL );
	fgSizer94->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer331;
	bSizer331 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText323 = new wxStaticText( this, wxID_ANY, wxT("P3DCrane2\nPanther3D World Editor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText323->Wrap( -1 );
	m_staticText323->SetFont( wxFont( 8, 74, 90, 92, false, wxT("Tahoma") ) );
	
	bSizer331->Add( m_staticText323, 0, wxALL|wxEXPAND, 5 );
	
	m_staticline11 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer331->Add( m_staticline11, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer332;
	bSizer332 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText324 = new wxStaticText( this, wxID_ANY, wxT("Panther3D Engine:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText324->Wrap( -1 );
	bSizer332->Add( m_staticText324, 0, wxLEFT, 5 );
	
	m_staticText326 = new wxStaticText( this, wxID_ANY, wxT("version string"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText326->Wrap( -1 );
	bSizer332->Add( m_staticText326, 0, 0, 2 );
	
	bSizer331->Add( bSizer332, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer333;
	bSizer333 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText327 = new wxStaticText( this, wxID_ANY, wxT("Loaded Modules:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText327->Wrap( -1 );
	bSizer333->Add( m_staticText327, 0, wxLEFT, 5 );
	
	m_staticText325 = new wxStaticText( this, wxID_ANY, wxT("Module.dll, Module.dll"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText325->Wrap( -1 );
	m_staticText325->SetMaxSize( wxSize( -1,12 ) );
	
	bSizer333->Add( m_staticText325, 0, 0, 3 );
	
	bSizer331->Add( bSizer333, 1, wxEXPAND, 5 );
	
	m_staticline12 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer331->Add( m_staticline12, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText330 = new wxStaticText( this, wxID_ANY, wxT("Sysyem information:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText330->Wrap( -1 );
	bSizer331->Add( m_staticText330, 0, wxALL, 5 );
	
	fgSizer94->Add( bSizer331, 1, wxEXPAND, 5 );
	
	m_staticText328 = new wxStaticText( this, wxID_ANY, wxT("Procesor: Intel Core 2 Duo 2,4 GHz\nCapabilities: MMX, SSE, SSE2\nSystem RAM: 2048 MB\nGraphic Card: nVidia GeForce 8800 GTS\nCapabilities: MULTIPLE_RENDER_TARGETS, ..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText328->Wrap( -1 );
	fgSizer94->Add( m_staticText328, 0, wxALL, 5 );
	
	fgSizer93->Add( fgSizer94, 1, wxEXPAND, 5 );
	
	sbSizer94->Add( fgSizer93, 1, wxEXPAND, 5 );
	
	fgSizer91->Add( sbSizer94, 1, wxALL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer92;
	fgSizer92 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer92->AddGrowableCol( 0 );
	fgSizer92->AddGrowableRow( 0 );
	fgSizer92->SetFlexibleDirection( wxBOTH );
	fgSizer92->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText322 = new wxStaticText( this, wxID_ANY, wxT(" Copyright (C) 2008 Reversity Studios (www.reversity.org)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText322->Wrap( -1 );
	fgSizer92->Add( m_staticText322, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer328;
	bSizer328 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button175 = new wxButton( this, wxID_ANY, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button175->SetDefault(); 
	bSizer328->Add( m_button175, 0, wxALL, 5 );
	
	fgSizer92->Add( bSizer328, 1, wxEXPAND, 5 );
	
	fgSizer91->Add( fgSizer92, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer91 );
	this->Layout();
	
	// Connect Events
	m_button175->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgAboutGui::OnClose ), NULL, this );
}

dlgAboutGui::~dlgAboutGui()
{
	// Disconnect Events
	m_button175->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( dlgAboutGui::OnClose ), NULL, this );
}
