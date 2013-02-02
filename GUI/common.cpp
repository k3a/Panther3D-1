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
#include "common.h"

// okna
vector<IGuiElement*> g_pWindows;
// barevne schema
CScheme g_Scheme;

// spolecne promenne
IP3DDraw2D* g_p2D=NULL;
IP3DConsole* g_pConsole=NULL;
IP3DRenderer* g_pRenderer=NULL;
IP3DLocalization* g_pLocaliz=NULL;
IP3DEngine* g_pEngine=NULL;
IP3DFileSystem* g_pFS=NULL;

// promenne
ConVar* CVr_width;
ConVar* CVr_height;
ConVar CVgui_editor("gui_editor", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Zapnout mod editovani GUI?");

// mys + klaveska
IP3DInput* g_pInput=NULL;
int g_nMouseX=0;
int g_nMouseY=0;
// tlacitka mysi
bool g_bLMouseDown=false;
bool g_bLMouseDownFirstTime=false;
bool g_bLMouseUpFirstTime=false;
bool g_bLMouseLastState=false;

// spolecne dialogy
CP3DGUI_SelectFile* dia_SelectFile=NULL;

IGuiElement* GetWindowAtPos(int x, int y)
{
	for(vector<IGuiElement*>::reverse_iterator ppElement=g_pWindows.rbegin(); ppElement != g_pWindows.rend(); ppElement++)
	{
		if (x > (*ppElement)->m_nPosX && x < (*ppElement)->m_nPosX + (*ppElement)->m_nWidth &&
			y > (*ppElement)->m_nPosY-(*ppElement)->m_nHeaderHeight && y < (*ppElement)->m_nPosY + (*ppElement)->m_nHeight)
		{
			if ((*ppElement)->m_bHidden == false)
				return (*ppElement);
		}
	}
	return NULL;
}

IGuiElement* GetFirstVisibleWindow()
{
	for(vector<IGuiElement*>::reverse_iterator ppElement=g_pWindows.rbegin(); ppElement != g_pWindows.rend(); ppElement++)
	{
		if ((*ppElement)->m_bHidden == false)
			return (*ppElement);
	}
	return NULL;
}

void SetFirstVisibleWindow(IGuiElement* pWin)
{
	IGuiElement* pWinCurrent = GetFirstVisibleWindow();
	if (pWin && pWinCurrent && !((CWindow*)pWinCurrent)->IsModal())
	{
		if (pWin != g_pWindows.back())
		{
			IGuiElement* pOldBack = g_pWindows.back();
			for(vector<IGuiElement*>::iterator ppElement=g_pWindows.begin(); ppElement != g_pWindows.end(); ppElement++)
			{
				if ((*ppElement) == pWin)
				{
					// prohod tento (nove zvoleny) s byvalym prvnim
					g_pWindows[g_pWindows.size()-1] = pWin;
					(*ppElement) = pOldBack;
					// uprav ohniska
					pOldBack->m_bHasFocus = false;
					pWin->m_bHasFocus = true;
					// uprav viditelnost
					pWin->m_bHidden = false;
					break;
				}
			}
		}
	}
}