//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Loading dialog
//-----------------------------------------------------------------------------
#pragma once
#include "GUI_Loading.h"
#include "common.h"

class CP3DGUI_Loading : public IP3DGUI_Loading, public CWindow
{
public:
	CP3DGUI_Loading()
	{
		INIT_ELEMENT(btnTest);
		INIT_ELEMENT(btnFrameTest);
	}
	~CP3DGUI_Loading();
	bool Init();
	void SetProgress(float fValue);
	void Update();
private:
	float m_fProgress;
	//
	CButton btnTest;
	CButton btnFrameTest;
	CFrame frmTest;
	CSlider sldTest;
	CProgressBar pbTest;
	CLabel lblTest;
	CLabel lblTestB;
	CLabel lblTestCenter;
	CCheckBox chkTest;
	CTabStrip tsTest;
	 CFrame frm_tsTest_1; // prvni zalozka
	  CLabel lbl_Test1;
	 CFrame frm_tsTest_2; //druha zalozka
	  CLabel lbl_Test2;
    CTextBox txtTest;
	COption optTest1;
	COption optTest2;
	CImage imgTest;
};