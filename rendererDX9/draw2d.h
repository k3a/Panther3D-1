//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Class for drawing 2d objects (line, texture, sphere, ...)
//-----------------------------------------------------------------------------
#pragma once

#include "IP3DDraw2D.h"
#include <d3dx9.h>

class CP3DDraw2D : public IP3DDraw2D
{
public:
	CP3DDraw2D();
	~CP3DDraw2D();
	void DrawTexture(IP3DTexture *pTexture, int x1, int y1, int x2, int y2, float tu1=0, float tv1=0, float tu2=1, float tv2=1);
	void DrawSquare(int x1, int y1, int x2, int y2, unsigned long color1);
	void Line_Begin();
	void Line_End();
	void Line_SetWidth(int w);
	void Line_DrawSimple(int x1, int y1, int x2, int y2, unsigned long color);
	void Line_DrawFilledRect(int x, int y, int w, int h, unsigned long color);
	void Line_Draw(P3DVector2D *points, DWORD numPoints, unsigned long color);
	void Line_DrawRect(int x, int y, int w, int h, unsigned long color);
	void Line_Draw3DRect(int x, int y, int w, int h, unsigned long colorLeftTop, unsigned long colorRightBottom);
	void Draw3DLine(P3DXVector3D &start, P3DXVector3D &end, unsigned long color=0);
	void DrawAABB(P3DAABB &aabb, unsigned long color);
	bool Sprite_Begin(DWORD flags);
	bool Sprite_End();
	bool Sprite_Draw(IP3DTexture *pTexture, CONST RECT *pSrcRect, P3DXVector3D *pCenter, P3DXVector3D *pPosition, DWORD color);
	void Sprite_SetTransform(P3DXMatrix &mat);
	void DrawText(const char* text, int x, int y, unsigned long color);
private:
	ID3DXLine *m_pLine;
	ID3DXSprite *m_pSprite;
	ID3DXFont *m_pFont;
	//
	bool m_bNeedSetWidth;
	int m_fLineWidth;
};