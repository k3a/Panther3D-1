#ifndef __frmMain__
#define __frmMain__

/**
@file
Subclass of frmMainGui, which is generated by wxFormBuilder.
*/

#include "gui.h"
#include "panRender.h"

/** Implementing frmMainGui */
class frmMain : public frmMainGui
{
private:
	wxBoxSizer* sizerMainFrame;
	wxFlexGridSizer* sizerFourViewports;
	panRender* viewLT,  *viewLB, *viewRT, *viewRB;

protected:
	// Handlers for frmMainGui events.
	void OnClose( wxCloseEvent& event );
	void OnUpdateUI( wxUpdateUIEvent& event );
	void btnFindEntityPressed( wxCommandEvent& event );
	void txtPositionEnterPressed( wxCommandEvent& event );
	void tabEventsPickToggled( wxCommandEvent& event );
	void tabEventsParamLeaveFocus( wxFocusEvent& event );
	void tabEventsParamEnterFocus( wxFocusEvent& event );
	void tabEventsParamOnEnter( wxCommandEvent& event );
	void tabEventsDelayOnEnter( wxCommandEvent& event );
	void tabEventsAddPressed( wxCommandEvent& event );
	void tabEventsCopyPressed( wxCommandEvent& event );
	void tabEventsPastePressed( wxCommandEvent& event );
	void tabEventsDeletePressed( wxCommandEvent& event );
	void tabCreatePickPosPressed( wxCommandEvent& event );
	void tabCreateEntitiesDblClick( wxListEvent& event );
	void tabCreateFindOnTexChanged( wxCommandEvent& event );
	void tabCreateFindEnterPressed( wxCommandEvent& event );
	void tabCreateCreateBtnPressed( wxCommandEvent& event );
	void tabGroupsSelectedOnCombo( wxCommandEvent& event );
	void tabGroupsSelectedOnKeyUp( wxKeyEvent& event );
	void tabGroupsSelectedOnText( wxCommandEvent& event );
	void tabGroupsSelectedOnEnterCreate( wxCommandEvent& event );
	void tabGroupsCreateNewPressed( wxCommandEvent& event );
	void tabGroupsDeletePressed( wxCommandEvent& event );
	void tabGroupsAddSelectionToGroupPressed( wxCommandEvent& event );
	void tabGroupsSelectAllPressed( wxCommandEvent& event );
	void tabGroupsListDblClick( wxCommandEvent& event );
	void tabGroupsMakeSelectionFromHighlightedPressed( wxCommandEvent& event );
	void tabGroupsRemoveHighlightedPressed( wxCommandEvent& event );
	void tabGroupsClearListPressed( wxCommandEvent& event );
	void toolNewClicked( wxCommandEvent& event );
	void toolOpenClicked( wxCommandEvent& event );
	void toolSaveClicked( wxCommandEvent& event );
	void toolSelectClicked( wxCommandEvent& event );
	void toolMoveClicked( wxCommandEvent& event );
	void toolRotateClicked( wxCommandEvent& event );
	void toolScaleClicked( wxCommandEvent& event );
	void toolScaleSidesClicked( wxCommandEvent& event );
	void toolLocalClicked( wxCommandEvent& event );
	void toolPhysInEditorClicked( wxCommandEvent& event );
	void toolOrbitClicked( wxCommandEvent& event );
	void toolShowGridClicked( wxCommandEvent& event );
	void toolGridMinusClicked( wxCommandEvent& event );
	void toolGridPlusClicked( wxCommandEvent& event );
	void toolFarPlaneThumbScrollReleased( wxScrollEvent& event );
	void toolCameraSpeedScrollReleased( wxScrollEvent& event );
	void toolPlayClicked( wxCommandEvent& event );
	void toolAssetBrowserClicked( wxCommandEvent& event );
	void toolMaterialEditorClicked( wxCommandEvent& event );
	void toolModelEditorClicked( wxCommandEvent& event );
	void toolParticleEditorClicked( wxCommandEvent& event );
	void toolConsoleEnterPressed( wxCommandEvent& event );
	void mnuFileNewClicked( wxCommandEvent& event );
	void mnuFileOpenClicked( wxCommandEvent& event );
	void mnuFileSaveClicked( wxCommandEvent& event );
	void mnuFileSaveAsClicked( wxCommandEvent& event );
	void mnuFileExitClicked( wxCommandEvent& event );
	void mnuEditUndoClicked( wxCommandEvent& event );
	void mnuEditRedoClicked( wxCommandEvent& event );
	void mnuEditCopyClicked( wxCommandEvent& event );
	void mnuEditPasteClicked( wxCommandEvent& event );
	void mnuEditDeleteClicked( wxCommandEvent& event );
	void mnuViewHideSelectedClicked( wxCommandEvent& event );
	void mnuViewShowHiddenClicked( wxCommandEvent& event );
	void mnuDontSwitchToPropertiesClicked( wxCommandEvent& event );
	void mnuMapPropertiesClicked( wxCommandEvent& event );
	void mnuMapPlayClicked( wxCommandEvent& event );
	void mnuToolsAssetBrowserClicked( wxCommandEvent& event );
	void mnuToolsMaterialEditorClicked( wxCommandEvent& event );
	void mnuToolsModelViewerClicked( wxCommandEvent& event );
	void mnuParticleEditorClicked( wxCommandEvent& event );
	void mnuHelpOnlineClicked( wxCommandEvent& event );
	void mnuHelpAboutClicked( wxCommandEvent& event );
	
public:
	/** Constructor */
	frmMain( wxWindow* parent );
	void SetFullscreenViewport( eVT vtype );
	void SetFourViewportsArrangement( eVT vtype );
	panRender* GetViewport( eVT vtype );
	void AddConsoleMsg(const char* text, const char* scopeName);
	void SetActiveViewport(eVT vtype);
private:
	eVT mlastvtype; // last fullscreen viewport, -1 = 4viewport mode
	int miToolType;
	bool mbLocal;
	bool mbPhysInEditor;
	bool mbOrbit;
	int miFarPlane;
	int miCameraSpeed;
	bool mbGrid;
	int miGridSize;
	bool mbDontSwitch; // don't switch to properties on Create entity
};

#endif // __frmMain__
