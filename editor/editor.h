#pragma once

#include <wx/wx.h>
#include "ip3dgame.h"

class CEditor : public wxApp
{
public:
	CEditor();
	virtual ~CEditor();
	virtual bool OnInit();
	virtual int OnExit();
private:
	IP3DGame * mpGame;
};

DECLARE_APP(CEditor)