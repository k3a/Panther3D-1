//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Declaration of CFont
//-----------------------------------------------------------------------------

#pragma once
#include "d3dx9.h"
#include "ip3dfont.h"

class CP3DFont : public IP3DFont
{
public:
	CP3DFont();
	~CP3DFont();
	HRESULT CreateFont(const char *pFontName, int size, bool bold, bool italic);
	int GetLineH();
	int GetLineW(const char *pText);
	void GetTextSize(const char *pText, P3DRect2D* rect, OUT int* pWidth, OUT int* pHeight);
	int DrawText(int align, const char *pText, int x1, int y1, int x2, int y2, unsigned long color);
	int DrawText(int align, const char *pText, P3DRect2D rect, unsigned long color);
	int DrawText(const char *pText, P3DScreenPoint2D pos, unsigned long color);
	int DrawText(const char *pText, int x, int y, unsigned long color);
private:
	ID3DXFont *font;
	UINT sizeX;
};