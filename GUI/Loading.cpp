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
#include "Loading.h"

REGISTER_SINGLE_CLASS(CP3DGUI_Loading, IP3DGUI_LOADING);

static IP3DTexture* tex=NULL;

bool CP3DGUI_Loading::Init()
{
	CreateWindow(NULL, 100, 100, 250, 250, P3DWS_CAPTION | P3DWS_CLOSEBTN);
	btnTest.CreateButton("Nastav TextBox", 10, 10, 100, 20, this);

	frmTest.CreateFrame(NULL, 10, 50, 100, 50, true, this);
	btnFrameTest.CreateButton("Pokus :)", 20, 15, 50, 30, &frmTest);

	sldTest.CreateSlider(10, 130, 100, 20, this);

	pbTest.CreateProgressBar(10, 160, 100, 20, true, this);

	lblTest.CreateLabel("Normal label", 10, 185, 100, 13, false, LA_LEFT, this);
	lblTestCenter.CreateLabel("Label center", 10, 198, 100, 13, false, LA_CENTER, this);
	lblTestB.CreateLabel("Bold label", 10, 211, 100, 13, true, LA_LEFT, this);

	chkTest.CreateCheckBox(NULL, 10, 226, 100, 13, false, this);

	tsTest.CreateTabStrip(130, 10, 100, 60, this);
	 frm_tsTest_1.CreateFrame(NULL, 135, 32, 90, 33, false, this); // prvni zalozka
	  lbl_Test1.CreateLabel("Obsah 1. záložky", 0, 0, 90, 33, false, LA_CENTER | LA_VCENTER | LA_SINGLELINE, &frm_tsTest_1);
    tsTest.AddTab("První", &frm_tsTest_1);
	 frm_tsTest_2.CreateFrame(NULL, 135, 32, 90, 33, false, this); // druha zalozka
	  lbl_Test2.CreateLabel("Obsah 2. záložky", 0, 0, 90, 33, false, LA_CENTER | LA_VCENTER | LA_SINGLELINE, &frm_tsTest_2);
    tsTest.AddTab("Druhá", &frm_tsTest_2);

	txtTest.CreateTextBox(NULL, 130, 80, 100, 20, this);

	optTest1.CreateOption("Option1", 130, 110, 100, 20, false, this);
	optTest1.SetValue(true); // takto lze prednastavit hodnotu
	optTest2.CreateOption("Option2", 130, 125, 100, 20, false, this);

	imgTest.CreateImage(130, 150, 50, 50, true, NULL, this);

	return true;
}

void CP3DGUI_Loading::SetProgress(float fValue)
{
	m_fProgress = fValue;
}

void CP3DGUI_Loading::Update()
{
	if(btnFrameTest.OnPress())
	{
		//MessageBox(0, "Tlaèítko funguje... co jiného bys èekal?", "???", 0);
		if (tex) delete tex;
		tex = g_pRenderer->TakeScreenShotTexture();
		imgTest.SetTexture(tex);
	}

	if(btnTest.OnPress())
	{
		txtTest.SetText("Trochu delší text");
	}

	pbTest.SetValue(sldTest.GetValue());
}

CP3DGUI_Loading::~CP3DGUI_Loading()
{
	
}