#ifndef __panRender__
#define __panRender__

#include "HP3DVector.h"

/**
@file
Subclass of panRenderGui, which is generated by wxFormBuilder.
*/

#include "gui.h"

enum eVT
{
	LEFT_TOP,
	LEFT_BOTTOM,
	RIGHT_TOP,
	RIGHT_BOTTOM
};

/** Implementing panRenderGui */
class panRender : public panRenderGui
{
protected:
	// Handlers for panRenderGui events.
	void toolMaximizeClicked( wxCommandEvent& event );
	void toolRightClicked( wxCommandEvent& event );
	void toolTopClicked( wxCommandEvent& event );
	void toolFrontClicked( wxCommandEvent& event );
	void toolPerspectClicked( wxCommandEvent& event );
	void toolWireClicked( wxCommandEvent& event );
	void toolTextClicked( wxCommandEvent& event );
	void toolLitClicked( wxCommandEvent& event );
	void toolRealtimeClicked( wxCommandEvent& event );
	void toolRenderClicked( wxCommandEvent& event );
	void OnMouse( wxMouseEvent& event );
	
public:
	/** Constructor */
	panRender( wxWindow* parent, eVT vtype );
	void OnIdle( wxIdleEvent& event );
	void CheckToolbar(int id, bool checked);
	void Destroying();
	HWND GetViewportHWND(){return (HWND)this->panViewport->GetHWND();};
	void SetActive(bool bActive=true){ 	
		m_bActive = bActive;
		if(bActive)
			this->SetBackgroundColour(wxColour(255, 0, 0));
		else
			this->SetBackgroundColour(wxColour(0, 0, 0));
		this->ClearBackground(); 
	};
private:
	int miViewType;
	int miRenderType;
	bool mbRealtime;
	eVT mvtype;
	DWORD notRealtimeCnt; // number of non-realtime ticks
	bool m_bActive;
	//
	float m_fOrthoZoom;
	P3DXVector3D m_vPos;
	P3DXVector3D m_vView;
};

#endif // __panRender__