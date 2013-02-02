//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Class for creating and rendering fonts
//-----------------------------------------------------------------------------

#include "common.h"
#include "font.h"

REGISTER_CLASS(CP3DFont, IP3DRENDERER_FONT); // zaregistruj tuto tøídu

CP3DFont::CP3DFont()
{
	font = NULL;
}

CP3DFont::~CP3DFont()
{
	SAFE_RELEASE(font);
}

HRESULT CP3DFont::CreateFont(const char *pFontName, int size, bool bold, bool italic)
{
	int weight;
	if (bold) weight = FW_BOLD; else weight = FW_NORMAL;

	HRESULT hr = D3DXCreateFont( g_pD3DDevice,            // D3D device
                         size,               // Height
                         0,                     // Width
                         weight,               // Weight
                         1,                     // MipLevels, 0 = autogen mipmaps
                         italic,                 // Italic
                         DEFAULT_CHARSET,       // CharSet
                         OUT_DEFAULT_PRECIS,    // OutputPrecision
                         DEFAULT_QUALITY,       // Quality
                         DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
                         pFontName,              // pFaceName
                         &font);              // ppFont

	font->PreloadCharacters(32, 255); // preload chars to VRAM

	// get size of "x" because we want exact size and size of " " is = 0 without this workaround ;)
	RECT tmpRC;
	SetRect(&tmpRC, 0, 0, 0, 0);
	font->DrawText( NULL, "x", -1, &tmpRC, DT_CALCRECT, 0 );
	sizeX = (tmpRC.right-tmpRC.left)*2;

	return hr;
}

//-----------------------------------------------------------------------------
// Purpose:	Get text line height
//-----------------------------------------------------------------------------
int CP3DFont::GetLineH()
{
	if( font == NULL ) return 0;
	return font->DrawText( NULL, "A", -1, NULL, DT_CALCRECT, 0 ); // oprava, misto 1 jsem nastavil -1, je to ok?
}

int CP3DFont::GetLineW(const char *pText)
{
	if( font == NULL ) return 0;
	RECT rc;
	font->DrawText( NULL, pText, -1, &rc, DT_CALCRECT, 0 ); // oprava, misto 1 jsem nastavil -1, je to ok?
	return rc.right - rc.left;
}

void CP3DFont::GetTextSize(const char *pText, P3DRect2D* rect, OUT int* pWidth, OUT int* pHeight)
{
	Prof(RENDERER_CP3DFont__GetTextSize);
	if( !font || !pText ) return;

	RECT rc; int w; int h;

	if (rect) 
		SetRect(&rc, rect->x1, rect->y1, rect->x2, rect->y2);
	else
		SetRect(&rc, 0, 0, 0, 0);

	// vytvor text s x na konci a zacatku
	char* tmp = new char[strlen(pText)+3];
	strcpy(tmp, "x");
	strcat(tmp, pText);
	strcat(tmp, "x");

	// zjisti velikost textu s x na konci a zacatku
	h = font->DrawText( NULL, tmp, -1, &rc, DT_CALCRECT, 0 );
	w = rc.right - rc.left;

	delete[] tmp;

	// vysledek
	if (pWidth) *pWidth = w-sizeX;
	if (pHeight) *pHeight = h;
	if (rect) {
		rect->x1 = rc.left;
		rect->x2 = rc.right-sizeX;
		rect->y1 = rc.top;
		rect->y2 = rc.bottom;
	}
}

int CP3DFont::DrawText(int align, const char *pText, int x1, int y1, int x2, int y2, unsigned long color)
{
	if( font == NULL ) return 0;
	RECT rc;
	SetRect( &rc, x1, y1, x2, y2 );
	return font->DrawText( NULL, pText, -1, &rc, align, color);
}

int CP3DFont::DrawText(int align, const char *pText, P3DRect2D rect, unsigned long color)
{
	if( font == NULL ) return 0;
	return font->DrawText( NULL, pText, -1, (LPRECT)&rect, align, color);
}

int CP3DFont::DrawText(const char *pText, P3DScreenPoint2D pos, unsigned long color)
{
	if( font == NULL ) return 0;
	return DrawText(FA_NOCLIP, pText, pos.x, pos.y, 0, 0, color);
}

int CP3DFont::DrawText(const char *pText, int x, int y, unsigned long color)
{
	if( font == NULL ) return 0;
	return DrawText(FA_NOCLIP, pText, x, y, 0, 0, color);
}