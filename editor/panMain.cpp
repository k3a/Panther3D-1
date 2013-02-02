#include "panMain.h"

panMain::panMain( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer14->AddGrowableCol( 0 );
	fgSizer14->AddGrowableRow( 0 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	panRenderPlace = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* szMain;
	szMain = new wxBoxSizer( wxVERTICAL );

	panRenderPlace->SetSizer( szMain );
	panRenderPlace->Layout();
	szMain->Fit( panRenderPlace );
	fgSizer14->Add( panRenderPlace, 1, wxEXPAND | wxALL, 1 );

	m_staticline6 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizer14->Add( m_staticline6, 0, wxEXPAND | wxALL, 0 );

	panStatusbar = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	panStatusbar->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_SCROLLBAR ) );

	wxFlexGridSizer* fgSizer151;
	fgSizer151 = new wxFlexGridSizer( 1, 6, 0, 0 );
	fgSizer151->AddGrowableCol( 0 );
	fgSizer151->AddGrowableCol( 2 );
	fgSizer151->AddGrowableCol( 3 );
	fgSizer151->SetFlexibleDirection( wxBOTH );
	fgSizer151->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	txtStatus11 = new wxStaticText( panStatusbar, wxID_ANY, wxT("Ready"), wxDefaultPosition, wxSize( -1,11 ), wxST_NO_AUTORESIZE );
	txtStatus11->Wrap( -1 );
	txtStatus11->SetFont( wxFont( 7, 74, 90, 90, false, wxT("Tahoma") ) );

	fgSizer151->Add( txtStatus11, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4 );

	m_staticline41 = new wxStaticLine( panStatusbar, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	fgSizer151->Add( m_staticline41, 0, wxALL|wxEXPAND, 2 );

	gProgress1 = new wxGauge( panStatusbar, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,17 ), wxGA_HORIZONTAL|wxGA_SMOOTH );
	gProgress1->SetValue( 50 ); 
	gProgress1->Hide();

	fgSizer151->Add( gProgress1, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 2 );

	txtStatus21 = new wxStaticText( panStatusbar, wxID_ANY, wxT(" "), wxDefaultPosition, wxSize( -1,11 ), wxST_NO_AUTORESIZE );
	txtStatus21->Wrap( -1 );
	txtStatus21->SetFont( wxFont( 7, 74, 90, 90, false, wxT("Tahoma") ) );

	fgSizer151->Add( txtStatus21, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4 );

	m_staticline51 = new wxStaticLine( panStatusbar, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	fgSizer151->Add( m_staticline51, 0, wxEXPAND | wxALL, 2 );

	wxBoxSizer* bSizer291;
	bSizer291 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText301 = new wxStaticText( panStatusbar, wxID_ANY, wxT("x:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText301->Wrap( -1 );
	bSizer291->Add( m_staticText301, 0, wxBOTTOM|wxLEFT|wxTOP, 5 );

	txtX = new wxTextCtrl( panStatusbar, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,19 ), wxTE_PROCESS_ENTER );
	txtX->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

	bSizer291->Add( txtX, 0, wxALL, 2 );

	m_staticText311 = new wxStaticText( panStatusbar, wxID_ANY, wxT("y:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText311->Wrap( -1 );
	bSizer291->Add( m_staticText311, 0, wxBOTTOM|wxLEFT|wxTOP, 5 );

	txtY = new wxTextCtrl( panStatusbar, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,19 ), wxTE_PROCESS_ENTER );
	txtY->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

	bSizer291->Add( txtY, 0, wxALL, 2 );

	m_staticText321 = new wxStaticText( panStatusbar, wxID_ANY, wxT("z:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText321->Wrap( -1 );
	bSizer291->Add( m_staticText321, 0, wxBOTTOM|wxLEFT|wxTOP, 5 );

	txtZ = new wxTextCtrl( panStatusbar, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize( 50,19 ), wxTE_PROCESS_ENTER );
	txtZ->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

	bSizer291->Add( txtZ, 0, wxALL, 2 );

	fgSizer151->Add( bSizer291, 1, wxEXPAND, 5 );

	panStatusbar->SetSizer( fgSizer151 );
	panStatusbar->Layout();
	fgSizer151->Fit( panStatusbar );
	fgSizer14->Add( panStatusbar, 1, wxEXPAND | wxALL, 1 );

	this->SetSizer( fgSizer14 );
	this->Layout();

	// Connect Events
	txtX->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( panMain::OnXYZEnter ), NULL, this );
	txtY->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( panMain::OnXYZEnter ), NULL, this );
	txtZ->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( panMain::OnXYZEnter ), NULL, this );
}

panMain::~panMain()
{
	// Disconnect Events
	txtX->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( panMain::OnXYZEnter ), NULL, this );
	txtY->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( panMain::OnXYZEnter ), NULL, this );
	txtZ->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( panMain::OnXYZEnter ), NULL, this );
}