#pragma once

#include "gui.h"

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class panMain
///////////////////////////////////////////////////////////////////////////////
class panMain : public wxPanel 
{
private:

protected:
	wxPanel* panRenderPlace;
	wxStaticLine* m_staticline6;
	wxPanel* panStatusbar;
	wxStaticText* txtStatus11;
	wxStaticLine* m_staticline41;
	wxGauge* gProgress1;
	wxStaticText* txtStatus21;
	wxStaticLine* m_staticline51;
	wxStaticText* m_staticText301;
	wxTextCtrl* txtX;
	wxStaticText* m_staticText311;
	wxTextCtrl* txtY;
	wxStaticText* m_staticText321;
	wxTextCtrl* txtZ;

	// Virtual event handlers, overide them in your derived class
	virtual void OnXYZEnter( wxCommandEvent& event ){ event.Skip(); }


public:
	panMain( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL );
	~panMain();

};
