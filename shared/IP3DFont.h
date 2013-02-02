//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Font interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include "types.h"

enum FontAlign_t
{
	FA_LEFT = 0x00000000,
	FA_CENTER = 0x00000001, // zarovnani doprostred v rect
	FA_RIGHT = 0x00000002, // zarovnani napravo v rect
	FA_VCENTER = 0x00000004, // zarovnani doprostred vertikalne @>>pouze pokud je zaroven FA_SINGLELINE!<<@
	FA_BOTTOM = 0x00000008, // zarovnani dolu v rect @>>pouze pokud je zaroven FA_SINGLELINE!<<@
	FA_SINGLELINE = 0x00000020, // viz. vyse
	FA_NOCLIP = 0x00000100, // rect je zbytecny, text se proste vypise nezavisle na rect
	FA_CALCRECT = 0x00000400, // vypocitat rect
	FA_WORDBREAK = 0x00000010 // pokud slovo presahuje rect, bude posunuto na novy radek
};

class IP3DFont : public IP3DBaseInterface
{
public:
	virtual HRESULT CreateFont(const char *pFontName, int size, bool bold, bool italic)=0;
	virtual int GetLineH()=0; // zastarale, pouzit GetTextSize()
	virtual int GetLineW(const char *pText)=0; // zastarale, pouzit GetTextSize()
	virtual void GetTextSize(const char *pText, IN OUT P3DRect2D* rect, OUT int* pWidth, OUT int* pHeight)=0; // kterykoliv out muze byt NULL, pokud je text viceradkovy, bude zarovnan dle rect. Jinak rect vrati hranice textu.
	virtual int DrawText(int align, const char *pText, int x1, int y1, int x2, int y2, unsigned long color)=0;
	virtual int DrawText(int align, const char *pText, P3DRect2D rect, unsigned long color)=0;
	virtual int DrawText(const char *pText, P3DScreenPoint2D pos, unsigned long color)=0;
	virtual int DrawText(const char *pText, int x, int y, unsigned long color)=0;
};

#define IP3DRENDERER_FONT "P3DFont_2" // nazev ifacu