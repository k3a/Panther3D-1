//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Input (keyboard, mouse) class definition
//-----------------------------------------------------------------------------

#include "input.h"
#include "common.h"

REGISTER_SINGLE_CLASS(CP3DInput, IP3DRENDERER_INPUT); // zaregistruj tuto tøídu

CP3DInput::CP3DInput()
{
	ZeroMemory(m_pKeyboardDown, 256 * sizeof(bool));
	ZeroMemory(m_pKeyboardUp, 256 * sizeof(bool));
	ZeroMemory(m_pKeyboardCurr, 256 * sizeof(bool));
	// opak.
	m_pTimer = (IP3DCountdownTimer*)I_GetClass(IP3DENGINE_COUNTDOWNTIMER);
	// vytvoreni a nastaveni timeru
	long l=0;
	SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &l, 0);
	m_timDelay = m_pTimer->CreateTimer(256+256*l); // vytvor timer: prodleva pøed opakováním
	SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &l, 0);
	m_timRepeat = m_pTimer->CreateTimer(400/((l+1)/2)); // vytvor timer: rychlost opakování
	m_pTimer->EnableTimer(m_timRepeat, false); // zakázat opakování
	m_pTimer->EnableTimer(m_timDelay, false); // zakázat prodlevu
	m_wheelGranularity = 1;
}

CP3DInput::~CP3DInput()
{
    if( m_pMouse ) 
	{
		m_pMouse->Unacquire();
		SAFE_RELEASE(m_pMouse)
	}
    if( m_pKeyboard ) 
	{
		m_pKeyboard->Unacquire();
		SAFE_RELEASE(m_pKeyboard);
	}
    SAFE_RELEASE(m_pDI);
//	SAFE_DELETE(m_pTimer); // ukonci timer => PROC NEJDE UKONCIT???
}

HRESULT CP3DInput::CreateDevices(HWND hDlg, bool bExclusive/*=true*/)
{
	HRESULT hr;
    DWORD   dwCoopFlags;

	m_nWndViewport = hDlg;

	// ------------------ INIT MOUSE ---------------------------------------------//
    
	dwCoopFlags = bExclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE;
    dwCoopFlags |= DISCL_FOREGROUND;

    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ) )
        return hr;
    
    // Obtain an interface to the system Mouse device.
    if( FAILED( hr = m_pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL ) ) )
        return hr;
    
    // Set the data format to "Mouse format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing a
    // DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
    if( FAILED( hr = m_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
        return hr;
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
	 if( FAILED(hr = m_pMouse->SetCooperativeLevel( hDlg, dwCoopFlags )))
		return hr;

    // Acquire the newly created device
    m_pMouse->Acquire();

	 DIPROPDWORD	propGranular;
	 propGranular.diph.dwSize = sizeof(DIPROPDWORD);
	 propGranular.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	 propGranular.diph.dwObj = DIMOFS_Z;
	 propGranular.diph.dwHow = DIPH_BYOFFSET;
	 if (SUCCEEDED (m_pMouse->GetProperty (DIPROP_GRANULARITY, (LPDIPROPHEADER)&propGranular)))
		m_wheelGranularity = propGranular.dwData;

	// ------------------ INICIALIZACE KLÁVESNICE ---------------------------------------------//

	dwCoopFlags = DISCL_NONEXCLUSIVE;
    dwCoopFlags |= DISCL_BACKGROUND;

    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ) )
        return hr;
    
    // Obtain an interface to the system keyboard device.
    if( FAILED( hr = m_pDI->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, NULL ) ) )
        return hr;
    
    // Set the data format to "Keyboard format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing an array
    // of 256 bytes to IDirectInputDevice::GetDeviceState.
    if( FAILED( hr = m_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
        return hr;
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    hr = m_pKeyboard->SetCooperativeLevel( hDlg, dwCoopFlags );
    if( FAILED(hr) )
        return hr;

	// ------------ BUFFERED DATA IMPORTANT
        DIPROPDWORD dipdw;

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = 8; // Arbitary buffer size

        if( FAILED( hr = m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
            return hr;

    // Acquire the newly created device
	m_pKeyboard->Acquire();

	CON(MSG_CON_INFO, "Renderer: DirectInput input initialized!");

	return S_OK;
}

void CP3DInput::Loop()
{
	HRESULT			hr;
    DIMOUSESTATE2	dims2;		// DirectInput Mouse state structure

	//--------------------------- GET MOUSE DATA --------------------------------------------//

	// cleaar buffers
	memset (&m_pMouseD, 0, sizeof(m_pMouseD));

    if ((NULL == m_pMouse) || (NULL == m_pKeyboard))
        return;

    // Get the input's device state, and put the state in dims
    ZeroMemory( &dims2, sizeof(dims2) );
    hr = m_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
    if( FAILED(hr) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        hr = m_pMouse->Acquire();
        while( hr == DIERR_INPUTLOST ) 
		  {
			  Sleep(10);
			  hr = m_pMouse->Acquire();
		  }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return; 
    }

	memcpy(&m_pMouseDLast, &m_pMouseD, sizeof(m_pMouseD)); // stary stav pro dalsi snimek
    
    m_pMouseD.x = dims2.lX;
	m_pMouseD.y = dims2.lY;
	m_pMouseD.z = long(dims2.lZ / m_wheelGranularity);
	m_pMouseD.b[0] = dims2.rgbButtons[0];
	m_pMouseD.b[1] = dims2.rgbButtons[1];
	m_pMouseD.b[2] = dims2.rgbButtons[2];
	m_pMouseD.b[3] = dims2.rgbButtons[3];

	// pozice mysi v backBUfferSize
	RECT rect;
	POINT cur;

	GetWindowRect(m_nWndViewport, &rect);
	GetCursorPos(&cur);

	// jen v pripade mysi nad viewportem
	if (cur.x < rect.right && cur.x > rect.left && cur.y < rect.bottom && cur.y > rect.top)
	{
		cur.x -= rect.left;
		cur.y -= rect.top;

		m_absX = (int)((float)g_pEngSet.Width / (rect.right-rect.left) * cur.x);
		m_absY = (int)((float)g_pEngSet.Height / (rect.bottom-rect.top) * cur.y);
	}

	//--------------------------- GET KEYBOARD DATA --------------------------------------------//

	// vynuluj stav klaves doel a nahore
	ZeroMemory(m_pKeyboardDown, 256 * sizeof(bool));
	ZeroMemory(m_pKeyboardUp, 256 * sizeof(bool));

	DIDEVICEOBJECTDATA didod[8];
	DWORD              dwElements=8;
	hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);
    if( FAILED(hr) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        hr = m_pKeyboard->Acquire();
        while( hr == DIERR_INPUTLOST ) 
		  {
			  Sleep(10);
			  hr = m_pKeyboard->Acquire();
		  }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return; 
    }

	// ZPRACUJ DATA
	for(DWORD i=0; i < dwElements; i++) // klavesa dole
	{
		if (didod[i].dwData & 0x80)
		{
			m_pTimer->ResetTimer(m_timDelay); // resetovat a povolit odpocet prodlevy
			m_pKeyboardDown[didod[i].dwOfs] = true; // klávesa byla dole
			m_pKeyboardCurr[didod[i].dwOfs] = true; // akt. stav: stisknuta
		}
		else // klavesa nahore
		{
			m_pTimer->EnableTimer(m_timRepeat, false); // zakazat opakovaci timer
			m_pTimer->EnableTimer(m_timDelay, false); // zakazat odpocet prodlevy
			m_pKeyboardUp[didod[i].dwOfs] = true; // klavesa byla nahore
			m_pKeyboardCurr[didod[i].dwOfs] = false; // akt. stav: nestisknuta
		}
	}

	if (m_pTimer->IsTimeElapsed(m_timDelay, true)) // pokud vyprsela prodleva
	{
		m_pTimer->EnableTimer(m_timRepeat, false); // zakazat opakovaci timer
		m_pTimer->EnableTimer(m_timRepeat, true); // povolit opakovaci timer
	}
}

P3DMOUSED CP3DInput::GetMouseData()
{
	return m_pMouseD;
}

bool CP3DInput::IsKeyDown(int key)
{
	return m_pKeyboardCurr[key];
}

bool CP3DInput::IsKeyDownFirstTime(int key)
{
	return m_pKeyboardDown[key];
}

bool CP3DInput::IsKeyUp(int key)
{
	return !m_pKeyboardCurr[key];
}

bool CP3DInput::IsKeyUpFirstTime(int key)
{
	return m_pKeyboardUp[key];
}

unsigned short CP3DInput::GetKeyChar()
{
	bool repeat = m_pTimer->IsTimeElapsed(m_timRepeat, true);

	for (int i=0; i < 256; i++)
	{
		if (i==1) continue; // nepøevádìj esc klávesu
		if (m_pKeyboardDown[i] || (repeat && m_pKeyboardCurr[i] == true)) // je klávesa poprvé dole? Nebo je uz dost dlouho dole(=opakovat)?
		{
			//NUMPAD
			switch(i)
			{
				case DIK_NUMPAD0: return '0'; break;
				case DIK_NUMPAD1: return '1'; break;
				case DIK_NUMPAD2: return '2'; break;
				case DIK_NUMPAD3: return '3'; break;
				case DIK_NUMPAD4: return '4'; break;
				case DIK_NUMPAD5: return '5'; break;
				case DIK_NUMPAD6: return '6'; break;
				case DIK_NUMPAD7: return '7'; break;
				case DIK_NUMPAD8: return '8'; break;
				case DIK_NUMPAD9: return '9'; break;
				case 83: return '.'; break;
				case 181: return '/'; break;
				case DIK_NUMPADCOMMA: return '.'; break;
				case DIK_NUMPADENTER: return 13; break;
				case DIK_NUMPADEQUALS: return '='; break;
			}

			unsigned short result=0; // výsledný znak
			HKL layout=GetKeyboardLayout(0); // získej aktuální rozložení klávesnice
			unsigned char State[256];
			if (GetKeyboardState(State)==FALSE) // získej stav všech virt. kláves
				return 0; // chyba
			unsigned int vk=MapVirtualKeyEx(i,1,layout); // namapuj virtual-code
			ToAsciiEx(vk,i,State,&result, 0,layout); // získej Ascii znak
			//CON(MSG_CON_INFO, "%d => %d", i, result); // DEBUG!
			return result;
		}
	}
	return 0;
}

bool CP3DInput::IsLeftButtonDown()
{
	return !!(m_pMouseD.b[0] & 0x80);
}
bool CP3DInput::IsLeftButtonDownFirstTime()
{
	return IsLeftButtonDown() && !(m_pMouseDLast.b[0] & 0x80);
}
bool CP3DInput::IsRightButtonDown()
{
	return !!(m_pMouseD.b[2] & 0x80); // FIXME: Je to spravny index? Co jak ma user dvojtlacitkovku???
}
bool CP3DInput::IsRightButtonDownFirstTime()
{
	return IsRightButtonDown() && !(m_pMouseDLast.b[2] & 0x80); // FIXME: Je to spravny index? Co jak ma user dvojtlacitkovku???
}
void CP3DInput::GetViewportMousePos(int &x, int &y)  // vrati x a y v pixelech v aktualnim viewportu (+rozliseni)
{
	x = m_absX;
	y = m_absY;
}

bool CP3DInput::IsLeftButtonUp()
{
	return !(m_pMouseD.b[0] & 0x80); 
}
bool CP3DInput::IsLeftButtonUpFirstTime()
{
	return IsLeftButtonUp() && m_pMouseDLast.b[0] & 0x80;
}
bool CP3DInput::IsRightButtonUp()
{
	return !(m_pMouseD.b[2] & 0x80); // FIXME: Je to spravny index? Co jak ma user dvojtlacitkovku???
}
bool CP3DInput::IsRightButtonUpFirstTime()
{
	return IsRightButtonUp() && m_pMouseDLast.b[2] & 0x80; // FIXME: Je to spravny index? Co jak ma user dvojtlacitkovku???
}