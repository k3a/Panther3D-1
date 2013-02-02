
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	IP3DLightManager interface
//-----------------------------------------------------------------------------

#pragma once

#include "interface.h"
#include "HP3DMatrix.h"

#define	MAX_DYN_LIGHTS				24
#define	MAX_DYN_LIGHTS_PER_FACE		4		// rovnaka hodnota musi byt definovana v shaderoch
#define	MAX_FLICKER_SAMPLES			1024	// max pocet znakov vo flicker_stringu (po rozbaleni)

//////////////////////////////////////////////////////////////////////////

struct DYN_LIGHT_INFO_USER
{
	P3DXMatrix	pos_rot;
	char*		sz_light_texture;
	float		fRadius;
	P3DColorValueRGB	color;
	char*		szFlicker_str;
	float		fFlicker_cycle_time;
	bool		bFlicker_smooth;
	bool		bCastShadows;
};

struct SHADER_DYNLIGHT_INPUT
{
	P3DXMatrix			light_pos_rot[MAX_DYN_LIGHTS_PER_FACE];
	void*				light_textures[MAX_DYN_LIGHTS_PER_FACE];	// svetelne cube textury - LPDIRECT3DCUBETEXTURE9
	float				light_radius[MAX_DYN_LIGHTS_PER_FACE];
	P3DColorValueRGB	light_color[MAX_DYN_LIGHTS_PER_FACE];
	int					light_count;
};



//////////////////////////////////////////////////////////////////////////

class IP3DLightManager : public IP3DBaseInterface
{
public:
	virtual DWORD AddDynamicLight (DYN_LIGHT_INFO_USER &lightInfo, bool bStartFlickering = true) = 0;
	virtual void SetLightPosition (DWORD dwLightID, P3DMatrix	&light_pos_rot) = 0;
	virtual void SetLightRadius (DWORD dwLightID, float fRadius) = 0;
	virtual void Loop () = 0;
	virtual void SetMapFaceNum (int intFaceNum) = 0;
	virtual bool GetModelLightInfo (P3DSphere modelBoundSphere, SHADER_DYNLIGHT_INPUT &shader_lights) = 0;
	virtual bool GetFaceLightInfo (int intBSPFace, SHADER_DYNLIGHT_INPUT	&shader_lights) = 0;
	virtual void RemoveLight (DWORD dwLightID) = 0;
	virtual void SwitchLight (DWORD dwLightID, bool bSwitchOnOff) = 0;
	virtual void SwitchFlickering (DWORD dwLightID, bool bFlickerOnOff) = 0;
};


#define IP3DRENDERER_LIGHTMANAGER "P3DLightManager_1" // nazev ifacu
