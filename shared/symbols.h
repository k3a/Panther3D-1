//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Shared symbols
//-----------------------------------------------------------------------------
/*
MÌsto pro umisùov·nÌ sdÌlen˝ch maker, definic, pripadne funkci
*/
#pragma once

// Memory leak detection
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif

// BezpeËnÈ ukonËov·nÌ t¯Ìd
#define SAFE_NULL(p)			{p = NULL;}
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_SHUTDOWN(p)		{ if(p) { (p)->Shutdown();(p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }

#define ONE_METER 				100.0f  // one meter
#define ONE_METER_DIV			0.01f	// (1 / one meter)
#define UNIVERSAL_GRAVITY		9.8f

#define P3D_PI		((float)3.141592654f)

#define		DEFAULT_FIELD_OF_VIEW	((P3D_PI/180.0f) * 65.0f)
#define		DEFAULT_NEAR_PLANE		10.0f
#define		DEFAULT_FAR_PLANE			40000.0f

#define P3DMAX_PATH 256
#define P3DMAX_FILENAME 64

//#### BARVY
#define P3DCOLOR_ARGB(a,r,g,b) \
	((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define P3DCOLOR_XRGB(r,g,b) P3DCOLOR_ARGB(0xff,r,g,b)
//zakl. barvy
#define P3DCOLOR_RED P3DCOLOR_XRGB(255, 0, 0)
#define P3DCOLOR_GREEN P3DCOLOR_XRGB(0, 255, 0)
#define P3DCOLOR_BLUE P3DCOLOR_XRGB(0, 0, 255)
#define P3DCOLOR_BLACK 0xFF000000 // cerna
#define P3DCOLOR_WHITE 0xFFFFFFFF // bila
#define P3DCOLOR_GRAY 0xFF808080 // seda
#define P3DCOLOR_ORANGE 0xFFFFA500 // oranzova
#define P3DCOLOR_CYAN 0xFF0000FF // modrozelena
#define P3DCOLOR_BROWN 0xFFA52A2A // hneda
#define P3DCOLOR_YELLOW 0xFFFFFF00 // zluta
#define P3DCOLOR_SILVER 0xFFC0C0C0 // stribrna
#define P3DCOLOR_GOLD 0xFFFFD700 // zlata
#define P3DCOLOR_LIME 0xFF00FF00 // razici zelena
#define P3DCOLOR_CYAN 0xFF0000FF // modrozelena
#define P3DCOLOR_MAGENTA 0xFFFF00FF //fialova
#define P3DCOLOR_TOMATO 0xFFFF6347 // rajcatova oranzova
#define P3DCOLOR_ORANGE 0xFFFFA500 // oranzova
#define P3DCOLOR_AQUAMARINE 0xFF7FFFD4 // morska modra
#define P3DCOLOR_BISQUE 0xFFFFE4C4 //susenkova hneda
#define P3DCOLOR_CHOCOLATE 0xFFD2691E //cokoladova hneda
#define P3DCOLOR_DARKBLUE 0xFF00008B //tmave modra
#define P3DCOLOR_DARKGRAY 0xFFA9A9A9 //tmave seda
#define P3DCOLOR_DARKORCHID 0xFF9932CC //tmava orchidej
#define P3DCOLOR_SKYBLUE 0xFF00BFFF //nebeska modra
#define P3DCOLOR_PURPLE 0xFF800080 //purpurova
#define P3DCOLOR_SLATEBLUE 0xFF6A5ACD //jemna modra
#define P3DCOLOR_STEELBLUE 0xFF4682B4 //kovova modra
#define P3DCOLOR_GRASSGREEN 0xFF2E8B57 //travnata zelena
#define P3DCOLOR_OLIVEGREEN 0xFF6B8E23 // olivove zelena

#define P3DToRadian( degree ) ((degree) * (P3D_PI / 180.0f))
#define P3DToDegree( radian ) ((radian) * (180.0f / P3D_PI))

//#### kopie te z Windows.... nutne
#define pmax(a,b)            (((a) > (b)) ? (a) : (b))
#define pmin(a,b)            (((a) < (b)) ? (a) : (b))

int wildcmp(const char *pszWildcard, const char *pszString);

//matematicke funkce
#include <stdlib.h>
inline int randi(int nMin, int nMax){ return rand()%(nMax-nMin)+nMin; };
inline int randi(int nMax){ return rand()%nMax; };
inline float randf(float fMin, float fMax){ return (1.0f/RAND_MAX*rand())*(fMax-fMin)+fMin; };
inline float randf(float fMax){ return (1.0f/RAND_MAX*rand())*fMax; };