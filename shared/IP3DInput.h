//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Input (keyboard, mouse) interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

struct P3DMOUSED // mouse data
{
    long x, y, z;
	unsigned char b[4]; // buttons
};

class IP3DInput : public IP3DBaseInterface
{
public:
	virtual long CreateDevices(HWND hDlg, bool bExclusive=true)=0;
	/** Retrieve new keyboard, mouse and other device state. */
	virtual void Loop()=0;
	virtual P3DMOUSED GetMouseData()=0;
	virtual bool IsKeyDown(int key)=0;
	virtual bool IsKeyDownFirstTime(int key)=0;
	virtual bool IsKeyUp(int key)=0;
	virtual bool IsKeyUpFirstTime(int key)=0;
	virtual bool IsLeftButtonDown()=0;
	virtual bool IsLeftButtonDownFirstTime()=0;
	virtual bool IsRightButtonDown()=0;
	virtual bool IsRightButtonDownFirstTime()=0;
	virtual bool IsLeftButtonUp()=0;
	virtual bool IsLeftButtonUpFirstTime()=0;
	virtual bool IsRightButtonUp()=0;
	virtual bool IsRightButtonUpFirstTime()=0;
	/** Returns X and Y coordinates of mouse within actual viewport. */
	virtual void GetViewportMousePos(int &x, int &y)=0;
	virtual unsigned short GetKeyChar()=0;
};

#define IP3DRENDERER_INPUT "P3DInput_2" // nazev ifacu