//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Tøída pro vykreslování 2D objektù jako ètvercù, èar, bodù, … vèetnì textury.
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include "IP3DTexture.h"
#include "HP3DMatrix.h"
#include "types.h"

// sprites
#define SPRITE_DONOTSAVESTATE               (1 << 0)
#define SPRITE_DONOTMODIFY_RENDERSTATE      (1 << 1)
#define SPRITE_OBJECTSPACE                  (1 << 2)
#define SPRITE_BILLBOARD                    (1 << 3)
#define SPRITE_ALPHABLEND                   (1 << 4)
#define SPRITE_SORT_TEXTURE                 (1 << 5)
#define SPRITE_SORT_DEPTH_FRONTTOBACK       (1 << 6)
#define SPRITE_SORT_DEPTH_BACKTOFRONT       (1 << 7)

class IP3DDraw2D : public IP3DBaseInterface
{
public:
	virtual void DrawTexture(IP3DTexture *pTexture, int x1, int y1, int x2, int y2, float tu1=0, float tv1=0, float tu2=1, float tv2=1)=0;
	virtual void DrawSquare(int x1, int y1, int x2, int y2, unsigned long color1)=0;
	virtual void Line_Begin()=0;
	virtual void Line_End()=0;
	virtual void Line_SetWidth(int w)=0;
	virtual void Line_DrawSimple(int x1, int y1, int x2, int y2, unsigned long color)=0;
	virtual void Line_DrawFilledRect(int x, int y, int w, int h, unsigned long color)=0;
	virtual void Line_Draw(P3DVector2D *points, DWORD numPoints, unsigned long color)=0;
	virtual void Line_DrawRect(int x, int y, int w, int h, unsigned long color)=0;
	virtual void Line_Draw3DRect(int x, int y, int w, int h, unsigned long colorLeftTop, unsigned long colorRightBottom)=0;
	virtual void Draw3DLine(P3DXVector3D &start, P3DXVector3D &end, unsigned long color=0)=0;
	virtual void DrawAABB(P3DAABB &aabb, unsigned long color)=0;
	virtual bool Sprite_Begin(DWORD flags)=0;
	virtual bool Sprite_End()=0;
	virtual bool Sprite_Draw(IP3DTexture *pTexture, CONST RECT *pSrcRect, P3DXVector3D *pCenter, P3DXVector3D *pPosition, DWORD color)=0;
	virtual void Sprite_SetTransform(P3DXMatrix &mat)=0;
	virtual void DrawText(const char* text, int x, int y, unsigned long color)=0;
};

#define IP3DRENDERER_DRAW2D "P3DDraw2D_2" // nazev ifacu