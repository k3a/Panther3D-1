//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Input (keyboard, mouse) class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dinput.h"
#include "types.h"
#include "ip3dcountdowntimer.h"
#include "string"

class CP3DInput : public IP3DInput
{
public:
	CP3DInput();
	~CP3DInput();
	HRESULT CreateDevices(HWND hDlg, bool bExclusive=true);
	void Loop();
	P3DMOUSED GetMouseData();
	bool IsKeyDown(int key);
	bool IsKeyDownFirstTime(int key);
	bool IsKeyUp(int key);
	bool IsKeyUpFirstTime(int key);
	unsigned short GetKeyChar();
	bool IsLeftButtonDown();
	bool IsLeftButtonDownFirstTime();
	bool IsRightButtonDown();
	bool IsRightButtonDownFirstTime();
	bool IsLeftButtonUp();
	bool IsLeftButtonUpFirstTime();
	bool IsRightButtonUp();
	bool IsRightButtonUpFirstTime();
	void GetViewportMousePos(int &x, int &y); // vrati x a y v pixelech v aktualnim viewportu (+rozliseni)
private:
	LPDIRECTINPUT8       m_pDI; // DirectInput pointer
	LPDIRECTINPUTDEVICE8 m_pMouse; // The mouse device
	LPDIRECTINPUTDEVICE8 m_pKeyboard; // The keyboard device
	P3DMOUSED m_pMouseD;
	P3DMOUSED m_pMouseDLast;
	bool m_pKeyboardDown[256]; // aktuální snímek - byla klávesa dole
	bool m_pKeyboardUp[256]; // aktuální snímek - byla klávesa nahoøe
	bool m_pKeyboardCurr[256]; // aktuální snímek - byla klávesa nahoøe
	// --- opakovani klaves ---
	bool m_bOpak; // Opakovat stisknutí klávesy?
	IP3DCountdownTimer *m_pTimer;
	unsigned long m_timDelay;
	unsigned long m_timRepeat;
	long		m_wheelGranularity;
	// --
	HWND m_nWndViewport;
	int m_absX;
	int m_absY;
};