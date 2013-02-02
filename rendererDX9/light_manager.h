
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DLightManager declaration
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "bspmap.h"


enum P3DCUBEMAP_FACES
{
	P3DCUBEMAP_FACE_POSITIVE_X = 0,
	P3DCUBEMAP_FACE_NEGATIVE_X = 1,
	P3DCUBEMAP_FACE_POSITIVE_Y = 2,
	P3DCUBEMAP_FACE_NEGATIVE_Y = 3,
	P3DCUBEMAP_FACE_POSITIVE_Z = 4,
	P3DCUBEMAP_FACE_NEGATIVE_Z = 5
};


struct DYN_LIGHT_INFO
{
	LPDIRECT3DCUBETEXTURE9	pLightTexture;
	DWORD				dwLightID;
	CBitset				lit_faces;			// zoznam facov osvetlenych tymto svetlom
	P3DXMatrix			pos_rot;
	float				fRadius;
	P3DColorValueRGB	color;
	float				fFlickerMultiplier;
	unsigned char*		pFlicker_intenzity_cycle;
	DWORD				dwFlickerSamplesCount;		// pocet "samplov" v pFlicker_intenzity_cycle
	float				fFlicker_cycle_time;
	bool				bFlicker_smooth;
	bool				bCastShadows;

	P3DXVector3D		vecPositionChange;	// zmena polohy
	float				fRadiusChange;		// zmena dosahu
	bool				bLightON;			// true ak je svetlo zapnute
	bool				bFlickerON;			// true ak je zapnuty flickering


	DYN_LIGHT_INFO() {pLightTexture = NULL; pFlicker_intenzity_cycle = NULL; TurnOff();}

	void TurnOff ()
	{
		SAFE_RELEASE (pLightTexture)
		dwLightID = 0xFFFFFFFF;
		lit_faces.ClearAll ();
		pos_rot.SetIdentityMatrix ();
		fRadius = 0;
		color = P3DColorValueRGB (0,0,0);
		fFlickerMultiplier = 1.0f;
		SAFE_DELETE_ARRAY (pFlicker_intenzity_cycle)
		dwFlickerSamplesCount = 0;
		fFlicker_cycle_time = 0; bFlicker_smooth = false;
		bCastShadows = false;
		vecPositionChange = P3DXVector3D(0,0,0);
		fRadiusChange = 0.0f;
		bLightON = false;
		bFlickerON = false;
	}
};


//---------------------------------
class CP3DLightManager : public IP3DLightManager
{
private:
	DYN_LIGHT_INFO	m_dynLights[MAX_DYN_LIGHTS];
	DWORD			m_dwActiveLightsNum;
	DWORD			m_dwActiveLightsIdxs[MAX_DYN_LIGHTS];
	DWORD			m_dwNextFreeLightID;
	DWORD			m_dwFaceNum;		// aktualny pocet facov v mape
	CBitset			m_lightedFaces;		// facy ktore su osvetlene akymkolvek dyn. svetlom
	IP3DFrustum		*m_pFrustum;

	int FindLightIdx (DWORD dwLightID);
	void CheckLights ();

public:
	CP3DLightManager();
	~CP3DLightManager();
	bool Init();
	DWORD AddDynamicLight (DYN_LIGHT_INFO_USER &lightInfo, bool bStartFlickering = true);
	void SetLightPosition (DWORD dwLightID, P3DMatrix	&light_pos_rot);
	void SetLightRadius (DWORD dwLightID, float fRadius);
	void Loop ();
	void SetMapFaceNum (int intFaceNum);
	bool GetModelLightInfo (P3DSphere modelBoundSphere, SHADER_DYNLIGHT_INPUT &shader_lights);
	bool GetFaceLightInfo (int intBSPFace, SHADER_DYNLIGHT_INPUT &shader_lights);
	void RemoveLight (DWORD dwLightID);
	void SwitchLight (DWORD dwLightID, bool bSwitchOnOff);
	void SwitchFlickering (DWORD dwLightID, bool bFlickerOnOff);
};
