//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	common 
//-----------------------------------------------------------------------------
#pragma once
#include "IElement.h"
#include <Windows.h>
#include "interface.h"
#include "scheme.h"
#include "types.h"
#include "symbols.h"
#include "IP3DDraw2D.h"
#include "IP3DConsole.h"
#include "IP3DInput.h"
#include "IP3DConsole.h"
#include "ip3drenderer.h"
#include "IP3DLocalization.h"
#include "IP3DEngine.h"
#include "ip3dfilesystem.h"

// components
#include "Window.h"
#include "Button.h"
#include "Frame.h"
#include "Slider.h"
#include "ProgressBar.h"
#include "label.h"
#include "checkbox.h"
#include "TabStrip.h"
#include "TextBox.h"
#include "Option.h"
#include "Image.h"
#include "menu.h"
#include "ComboBox.h"
#include "Scrollbar.h"
#include "ScrollbarVert.h"
#include "ListBox.h"
#include <vector>

// spolecne dialogy
class CP3DGUI_SelectFile;

class CScheme;

extern vector<IGuiElement*> g_pWindows;
extern CScheme g_Scheme;

extern IP3DDraw2D* g_p2D;
extern IP3DConsole* g_pConsole;
extern IP3DRenderer* g_pRenderer;
extern IP3DLocalization* g_pLocaliz;
extern IP3DEngine* g_pEngine;
extern IP3DFileSystem* g_pFS;

// promenne
extern ConVar* CVr_width;
extern ConVar* CVr_height;
extern ConVar CVgui_editor;

// mys + klaveska
extern IP3DInput* g_pInput;
extern int g_nMouseX;
extern int g_nMouseY;
// tlacitka mysi
extern bool g_bLMouseDown;
extern bool g_bLMouseDownFirstTime;
extern bool g_bLMouseUpFirstTime;
extern bool g_bLMouseLastState;

// pomocne funkce
IGuiElement* GetWindowAtPos(int x, int y);
IGuiElement* GetFirstVisibleWindow();
void SetFirstVisibleWindow(IGuiElement* pWin);

// splolecne dialogy
extern CP3DGUI_SelectFile* dia_SelectFile;