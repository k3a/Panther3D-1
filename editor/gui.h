///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 20 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __gui__
#define __gui__
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>
#include <wx/propgrid/advprops.h>
#ifdef __VISUALC__
#include <wx/link_additions.h>
#endif //__VISUALC__
#include <wx/listctrl.h>
#include <wx/combobox.h>
#include <wx/tglbtn.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include <wx/slider.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/aui/aui.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_SELECT 1000
#define wxID_MOVE 1001
#define wxID_ROTATE 1002
#define wxID_SCALE 1003
#define wxID_SCALE_SIDES 1004
#define wxID_LOCAL 1005
#define wxID_PHYS_IN_EDITOR 1006
#define wxID_ORBIT 1007
#define wxID_SHOW_GRID 1008
#define wxID_GRID_MINUS 1009
#define wxID_GRID_PLUS 1010
#define wxID_PLAY 1011
#define wxID_ASSET_BROWSER 1012
#define wxID_MATERIAL_BROWSER 1013
#define wxID_MODEL_EDITOR 1014
#define wxID_PARTICLE_EDITOR 1015
#define wxID_DONT_SWITCH 1016
#define wxID_MAXIMIZE 1017
#define wxID_RIGHT 1018
#define wxID_TOP 1019
#define wxID_FRONT 1020
#define wxID_PERSPECT 1021
#define wxID_WIRE 1022
#define wxID_TEXT 1023
#define wxID_LIT 1024
#define wxID_REALTIME 1025
#define wxID_RENDER 1026

///////////////////////////////////////////////////////////////////////////////
/// Class frmMainGui
///////////////////////////////////////////////////////////////////////////////
class frmMainGui : public wxFrame 
{
	private:
	
	protected:
		wxAuiManager m_mgr;
		wxToolBar* toolbarStandard;
		wxToolBar* toolbarManipulators;
		wxToolBar* toolbarAdditional;
		wxToolBar* toolbarTools;
		wxToolBar* toolbarConsole;
		wxSlider* toolFarPlane;
		wxSlider* toolCameraSpeed;
		wxStaticText* m_staticText314;
		wxTextCtrl* toolConsole;
		wxMenuBar* menu;
		wxMenu* mnuFile;
		wxMenu* mnuEdit;
		wxMenu* mnuView;
		wxMenu* mnuMap;
		wxMenu* mnuTools;
		wxMenu* mnuHelp;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }
		virtual void OnUpdateUI( wxUpdateUIEvent& event ){ event.Skip(); }
		virtual void toolNewClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolOpenClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolSaveClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolSelectClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolMoveClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolRotateClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolScaleClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolScaleSidesClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolLocalClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolPhysInEditorClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolOrbitClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolShowGridClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolGridMinusClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolGridPlusClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolFarPlaneThumbScrollReleased( wxScrollEvent& event ){ event.Skip(); }
		virtual void toolCameraSpeedScrollReleased( wxScrollEvent& event ){ event.Skip(); }
		virtual void toolPlayClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolAssetBrowserClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolMaterialEditorClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolModelEditorClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolParticleEditorClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolConsoleEnterPressed( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuFileNewClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuFileOpenClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuFileSaveClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuFileSaveAsClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuFileExitClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuEditUndoClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuEditRedoClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuEditCopyClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuEditPasteClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuEditDeleteClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuViewHideSelectedClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuViewShowHiddenClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuDontSwitchToPropertiesClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuMapPropertiesClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuMapPlayClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuToolsAssetBrowserClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuToolsMaterialEditorClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuToolsModelViewerClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuParticleEditorClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuHelpOnlineClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void mnuHelpAboutClicked( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		frmMainGui( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("P3DCrane2"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,600 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~frmMainGui();
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgSplashGui
///////////////////////////////////////////////////////////////////////////////
class dlgSplashGui : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmap8;
	
	public:
		wxStaticText* txtInfo;
		dlgSplashGui( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 451,279 ), long style = 0 );
		~dlgSplashGui();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class panRenderGui
///////////////////////////////////////////////////////////////////////////////
class panRenderGui : public wxPanel 
{
	private:
	
	protected:
		wxToolBar* m_toolBar2;
		wxPanel* panViewport;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnIdle( wxIdleEvent& event ){ event.Skip(); }
		virtual void toolMaximizeClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolRightClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolTopClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolFrontClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolPerspectClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolWireClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolTextClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolLitClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolRealtimeClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void toolRenderClicked( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnMouse( wxMouseEvent& event ){ event.Skip(); }
		
	
	public:
		panRenderGui( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL );
		~panRenderGui();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class dlgAboutGui
///////////////////////////////////////////////////////////////////////////////
class dlgAboutGui : public wxDialog 
{
	private:
	
	protected:
		wxStaticBitmap* m_bitmap9;
		wxStaticText* m_staticText323;
		wxStaticLine* m_staticline11;
		wxStaticText* m_staticText324;
		wxStaticText* m_staticText326;
		wxStaticText* m_staticText327;
		wxStaticText* m_staticText325;
		wxStaticLine* m_staticline12;
		wxStaticText* m_staticText330;
		wxStaticText* m_staticText328;
		wxStaticText* m_staticText322;
		wxButton* m_button175;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		dlgAboutGui( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("About P3DCrane"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 550,315 ), long style = wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU );
		~dlgAboutGui();
	
};

#endif //__gui__
