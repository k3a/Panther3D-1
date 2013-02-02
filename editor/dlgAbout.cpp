#include "dlgAbout.h"

dlgAbout::dlgAbout( wxWindow* parent )
:
dlgAboutGui( parent )
{

}

void dlgAbout::OnClose( wxCommandEvent& event )
{
	this->Destroy();
	//delete this;
}
