
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DLightManager definition
//-----------------------------------------------------------------------------


#include "light_manager.h"
#include "../shared/IP3DBSPMap.h"
#include "../shared/HP3DQuaternion.h"
#include "../game/eBase.h"
#include "HP3DQuaternion.h"


REGISTER_SINGLE_CLASS (CP3DLightManager, IP3DRENDERER_LIGHTMANAGER); // zaregistruj tuto tøídu

ConVar CVr_dyn_lighting("r_dyn_lighting", "1", CVAR_NOT_STRING | CVAR_ARCHIVE, "Aktivovat dynamicke svetla");
//ConVar CVr_shadow_level("r_shadow_level", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Uroven tienov");

extern CP3DBSPMap *g_pBSPMap;

//---------------------------------
CP3DLightManager::CP3DLightManager()
{
	m_dwActiveLightsNum = 0;
	m_dwNextFreeLightID = 0;
	m_dwFaceNum = 0;
	m_pFrustum = NULL;
}

//---------------------------------
CP3DLightManager::~CP3DLightManager()
{
	for (DWORD i=0; i<MAX_DYN_LIGHTS; i++)
		m_dynLights[i].TurnOff ();

	SAFE_DELETE (m_pFrustum)
}

//---------------------------------
bool CP3DLightManager::Init()
{
	m_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM_PRIVATE);
	if (m_pFrustum == NULL) return false;

	return true;
}

// vrati -1 ak sa svetlo s danym ID nenajde
//---------------------------------
inline int CP3DLightManager::FindLightIdx (DWORD dwLightID)
{
	for (register int i=0; i<MAX_DYN_LIGHTS; ++i)
		if (m_dynLights[i].dwLightID == dwLightID)
			return i;
	return -1;
}

// obnovi premenne m_dwActiveLightsNum a m_dwActiveLightsIdxs
// tiez nastavi bitset m_lightedFaces tak aby oznacoval vsetky facy, ktore 
// su osvetlene (akymkolvek) dyn. svetlom
//---------------------------------
inline void CP3DLightManager::CheckLights ()
{
	DWORD i, j;

	m_dwActiveLightsNum = 0;
	m_lightedFaces.ClearAll ();	// m_lightedFaces sluzi na urychlenie vypoctov

	if (!CVr_dyn_lighting.GetBool ())
		return;

	for (i=0; i<MAX_DYN_LIGHTS; i++)
		if ((m_dynLights[i].dwLightID != 0xFFFFFFFF) && m_dynLights[i].bLightON)
			{m_dwActiveLightsIdxs[m_dwActiveLightsNum] = i; m_dwActiveLightsNum++;}

	for (j=0; j<m_dwActiveLightsNum; j++)
		m_lightedFaces.Or (m_dynLights[m_dwActiveLightsIdxs[j]].lit_faces);
}

// vrati index identifikujuci svetlo, v pripade neuspechu 0xFFFFFFFF
//---------------------------------
DWORD CP3DLightManager::AddDynamicLight (DYN_LIGHT_INFO_USER &lightInfo, bool bStartFlickering)
{
	int intFreeIdx;
	bool bTextureLoaded = false;

	intFreeIdx = FindLightIdx (0xFFFFFFFF);	// najde prvy volny index
	if (intFreeIdx == -1) return 0xFFFFFFFF;

	if (lightInfo.sz_light_texture)
	{
		FSFILE	*pFile;
		BYTE	*pData;
		DWORD	dwSize;

		if (pFile = g_pFS->Load (lightInfo.sz_light_texture, pData, dwSize))
		{
			bTextureLoaded = SUCCEEDED (D3DXCreateCubeTextureFromFileInMemory (g_pD3DDevice, \
				(LPCVOID)pData, dwSize, &m_dynLights[intFreeIdx].pLightTexture));
			g_pFS->UnLoad (pFile, pData);
		}
	}

	P3DSphere	lightBS;
	lightBS.pointOrigin.x = lightInfo.pos_rot._41;
	lightBS.pointOrigin.y = lightInfo.pos_rot._42;
	lightBS.pointOrigin.z = lightInfo.pos_rot._43;
	lightBS.fRadius = lightInfo.fRadius;

	// osvetlene facy sa ulozia do CBitset pola
	g_pBSPMap->FindDynLightClusters (lightBS, m_dynLights[intFreeIdx].lit_faces);

	// inverzia rotacie
	P3DMatrix invMatrix;
	P3DXQuat quatRot;
	quatRot.BuildFromMatrix (lightInfo.pos_rot);
	quatRot.Conjugate ();
	quatRot.GetRotationMatrix (invMatrix);
	invMatrix.m_posit3 = lightInfo.pos_rot.m_posit3;
	m_dynLights[intFreeIdx].pos_rot = invMatrix;

	m_dynLights[intFreeIdx].fRadius = lightInfo.fRadius;
	m_dynLights[intFreeIdx].color = bTextureLoaded ? P3DColorValueRGB (1,1,1) : lightInfo.color;

	// dekodovanie flickering stringu
	if (lightInfo.szFlicker_str)
	{
		char *pStr, szFlickerTempBuf[MAX_FLICKER_SAMPLES] = {0};
		DWORD dwStrLen = 0;

		for (pStr = lightInfo.szFlicker_str; (*pStr!=0) && (dwStrLen<MAX_FLICKER_SAMPLES); pStr++)
		{
			char cSample = *pStr;
			// to lower case
			if ('A' <= cSample && cSample <= 'Z') cSample += 'a' - 'A';

			if (cSample >= 'a' && cSample <= 'z')
				{szFlickerTempBuf[dwStrLen] = cSample; dwStrLen++; continue;}

			if (cSample == '\\')
			{
				if (pStr == lightInfo.szFlicker_str) continue;
				int intRepeatNum = atoi(pStr+1) - 1;
				if (intRepeatNum <= 0) continue;
				cSample = *(pStr-1);
				if ('A' <= cSample && cSample <= 'Z') cSample += 'a' - 'A';
				for (int i=0; (i<intRepeatNum) && (dwStrLen<MAX_FLICKER_SAMPLES); i++)
					{szFlickerTempBuf[dwStrLen] = cSample; dwStrLen++;}
				while (*pStr!=0 && (*pStr<'a' || *pStr>'z') && (*pStr<'A' || *pStr>'Z'))
					pStr++;
				pStr--;
			}
		}
		if (dwStrLen > 0)
		{
			m_dynLights[intFreeIdx].pFlicker_intenzity_cycle = new unsigned char[dwStrLen];
			for (DWORD i=0; i<dwStrLen; i++)
				m_dynLights[intFreeIdx].pFlicker_intenzity_cycle[i] = szFlickerTempBuf[i] - 'a';
			m_dynLights[intFreeIdx].dwFlickerSamplesCount = dwStrLen;
			m_dynLights[intFreeIdx].fFlicker_cycle_time = lightInfo.fFlicker_cycle_time;
			m_dynLights[intFreeIdx].bFlicker_smooth = lightInfo.bFlicker_smooth;
			m_dynLights[intFreeIdx].bCastShadows = lightInfo.bCastShadows;
			m_dynLights[intFreeIdx].bFlickerON = bStartFlickering;
		}
	}

	m_dynLights[intFreeIdx].bLightON = true;

	m_dynLights[intFreeIdx].dwLightID = m_dwNextFreeLightID;
	m_dwNextFreeLightID++;
	if (m_dwNextFreeLightID == 0xFFFFFFFF) m_dwNextFreeLightID = 0;

	// obnovenie zoznamu aktivnych dyn. svetiel
	CheckLights();

	return m_dynLights[intFreeIdx].dwLightID;
};

//---------------------------------
void CP3DLightManager::SetLightPosition (DWORD dwLightID, P3DMatrix &light_pos_rot)
{
	int intIdx = FindLightIdx (dwLightID);
	if (intIdx == -1) return;

	m_dynLights[intIdx].vecPositionChange += light_pos_rot.m_posit3;
	m_dynLights[intIdx].vecPositionChange -= m_dynLights[intIdx].pos_rot.m_posit3;

	// inverzia rotacie
	P3DMatrix invMatrix;
	P3DXQuat quatRot;
	quatRot.BuildFromMatrix (light_pos_rot);
	quatRot.Conjugate ();
	quatRot.GetRotationMatrix (invMatrix);

	invMatrix.m_posit3 = light_pos_rot.m_posit3;
	m_dynLights[intIdx].pos_rot = invMatrix;
}

//---------------------------------
void CP3DLightManager::SetLightRadius (DWORD dwLightID, float fRadius)
{
	int intIdx = FindLightIdx (dwLightID);
	if (intIdx == -1) return;

	m_dynLights[intIdx].fRadiusChange += fRadius - m_dynLights[intIdx].fRadius;
	m_dynLights[intIdx].fRadius = fRadius;
}

//---------------------------------
void CP3DLightManager::Loop ()
{
	if (!CVr_dyn_lighting.GetBool())
		return;

	// prepocitat zoznam facov a modelov ak je to potrebne
	for (DWORD i=0; i<m_dwActiveLightsNum; i++)
	{
		DWORD dwIdx = m_dwActiveLightsIdxs[i];
		P3DXVector3D vecPosChange = m_dynLights[dwIdx].vecPositionChange;
		float fMinChange = min ((m_dynLights[dwIdx].fRadius * 0.05f), 0.2f);

		// ak sa zmenil dosah svetla o viac ako o 1/20 hodnoty radius, alebo o 20cm tak urcit
		// novy zoznam osvetlenych facov a dyn. modelov
		if (vecPosChange.Abs().GetMax() > fMinChange || \
			fabs(m_dynLights[dwIdx].fRadiusChange) > fMinChange)
		{
			// bsp facy
			P3DSphere	boundSphere;
			boundSphere.pointOrigin = m_dynLights[dwIdx].pos_rot.m_posit3;
			boundSphere.fRadius = m_dynLights[dwIdx].fRadius;
			g_pBSPMap->FindDynLightClusters (boundSphere, m_dynLights[dwIdx].lit_faces);

			// dyn modely
			eBase *pEntity = !g_pEntMgr ? NULL : g_pEntMgr->FindEntityByClassname ("misc_model_dynamic");
			for (; pEntity; pEntity=pEntity->GetNext())		// prechadzanie vsetkych dyn. modelov
			{
				P3DSphere	boundSphere;
				// FIXME: nefunguje !!!
//				pEntity->GetActualBoundSphere (boundSphere);
				P3DXVector3D vecLightPos (m_dynLights[dwIdx].pos_rot.m_posit3);
				vecLightPos -= boundSphere.pointOrigin;
				float fDistance = vecLightPos.Abs().GetMax();
				if (fDistance < (m_dynLights[dwIdx].fRadius + boundSphere.fRadius))
				{
					// model je v dosahu dyn svetla
					// TODO: dokoncit ... !!!
					// pEntity->AskForDynlightInfo() ???
				}
			}
			m_dynLights[dwIdx].vecPositionChange = P3DXVector3D(0,0,0);
			m_dynLights[dwIdx].fRadiusChange = 0.0f;
		}

		// flickering management
		float fCurrentTime = g_pTimer->GetTickCount_s ();
		if (m_dynLights[dwIdx].pFlicker_intenzity_cycle && m_dynLights[dwIdx].bFlickerON)
		{
			float fCurrentCyclePos = fmodf (fCurrentTime, m_dynLights[dwIdx].fFlicker_cycle_time) / m_dynLights[dwIdx].fFlicker_cycle_time;
			fCurrentCyclePos *= (float)m_dynLights[dwIdx].dwFlickerSamplesCount;
			DWORD dwCurrentCycleIdx = (DWORD)fCurrentCyclePos;
			if (m_dynLights[dwIdx].bFlicker_smooth)
			{
				DWORD dwNextCycleIdx;
				if (dwCurrentCycleIdx < (m_dynLights[dwIdx].dwFlickerSamplesCount - 1))
					dwNextCycleIdx = dwCurrentCycleIdx + 1;
				else
					dwNextCycleIdx = 0;

				float fFracional = fCurrentCyclePos - float(DWORD(fCurrentCyclePos));
				m_dynLights[dwIdx].fFlickerMultiplier = \
					float(m_dynLights[dwIdx].pFlicker_intenzity_cycle[dwCurrentCycleIdx]) * (1.0f - fFracional) + \
					float(m_dynLights[dwIdx].pFlicker_intenzity_cycle[dwNextCycleIdx]) * fFracional; 
			}
			else
				m_dynLights[dwIdx].fFlickerMultiplier = float(m_dynLights[dwIdx].pFlicker_intenzity_cycle[dwCurrentCycleIdx]);

			m_dynLights[dwIdx].fFlickerMultiplier /= 'z' - 'a';
		}
	}

	/*
	// tymto sposobom sa prechadza zoznam dyn. modelov
	//////////////////////////////////////////////////////////////////////////
	eBase *pEntity = g_pEntMgr->FindEntityByClassname ("misc_model_dynamic");
	for (; pEntity; pEntity=pEntity->GetNext())
	{
	entVal_t *pVal = pEntity->m_Data.GetValue("renderable_obj");
	if (pVal->type == TYPE_POINTER)
	{
	IP3DRenderableObject *pRenderableObject = (IP3DRenderableObject*)*pVal->pValue;
	if (pRenderableObject)
	{
	// ...
	}
	}
	}
	*/

	//////////////////////////////////////////////////////////////////////////
	// FIXME: debug !!!
	static bool bIncrease = true;
	static bool bPosIncrease = true;
	for (DWORD i=0; i<m_dwActiveLightsNum; i++)
	{
		DWORD dwIdx = m_dwActiveLightsIdxs[i];
		/*
		float fNewRadius;
		if (bIncrease)
		fNewRadius = m_dynLights[dwIdx].fRadius + 0.5f;
		else
		fNewRadius = m_dynLights[dwIdx].fRadius - 0.5f;

		if (fNewRadius > 700.0f) bIncrease = false;
		if (fNewRadius < 200.0f) bIncrease = true;
		SetLightRadius (m_dynLights[dwIdx].dwLightID, fNewRadius);
		*/

		if (m_dynLights[dwIdx].pos_rot.m_posit.z > 500)
			bPosIncrease = false;
		else if (m_dynLights[dwIdx].pos_rot.m_posit.z < -1000)
			bPosIncrease = true;

		static float fAngle;
		fAngle += 0.003f;

		P3DXMatrix matLightTransform;
		matLightTransform.SetIdentityMatrix ();
		matLightTransform.RotateRelativeZXY (-fAngle, fAngle, 0);
		matLightTransform.m_posit = m_dynLights[dwIdx].pos_rot.m_posit;
//		matLightTransform.Translate (0.0f, 0.0f, bPosIncrease ? 2.0f : -2.0f);
		//		matLightTransform.Translate (0.0f, 0.0f, bPosIncrease ? 3.0f : -3.0f);
		SetLightPosition (m_dynLights[dwIdx].dwLightID, matLightTransform);
	}
	// FIXME: debug !!!

	CheckLights();
}

// volane pri nacitani mapy - BSPMap::Build()
//---------------------------------
void CP3DLightManager::SetMapFaceNum (int intFaceNum)
{
	for (DWORD i = 0; i < MAX_DYN_LIGHTS; i++)
	{
		m_dynLights[i].TurnOff();
		m_dynLights[i].lit_faces.Resize (intFaceNum);
	}
	m_lightedFaces.Resize (intFaceNum);
	m_dwFaceNum = intFaceNum;
}

//---------------------------------
void CP3DLightManager::RemoveLight (DWORD dwLightID)
{
	int intIdx = FindLightIdx (dwLightID);
	if (intIdx == -1) return;
	m_dynLights[intIdx].TurnOff ();
	// obnovenie zoznamu aktivnych dyn. svetiel
	CheckLights();
}

//---------------------------------
void CP3DLightManager::SwitchLight (DWORD dwLightID, bool bSwitchOnOff)
{
	int intIdx = FindLightIdx (dwLightID);
	if (intIdx == -1) return;
	if (m_dynLights[intIdx].bLightON == bSwitchOnOff)	// ziadna zmena
		return;
	m_dynLights[intIdx].bLightON = bSwitchOnOff;
	// obnovenie zoznamu aktivnych dyn. svetiel
	CheckLights();
}

//---------------------------------
void CP3DLightManager::SwitchFlickering (DWORD dwLightID, bool bFlickerOnOff)
{
	int intIdx = FindLightIdx (dwLightID);
	if (intIdx == -1) return;
	if (m_dynLights[intIdx].pFlicker_intenzity_cycle)
		m_dynLights[intIdx].bFlickerON = bFlickerOnOff;
}

// pre dany dyn. model vrati informacie potrebne pre shader
// vrati true ak je model osvetleny dyn. svetlom
//---------------------------------
bool CP3DLightManager::GetModelLightInfo (P3DSphere modelBoundSphere, SHADER_DYNLIGHT_INPUT &shader_lights)
{
	int intShaderLightCount = 0;

	if (m_dwActiveLightsNum)
	{
		// urcenie zoznamu svetiel, ktore osvetluju dany model
		DWORD i;
		for (i=0; i<m_dwActiveLightsNum; ++i)
		{
			DWORD dwIdx = m_dwActiveLightsIdxs[i];
			P3DXVector3D vecLightPos (m_dynLights[dwIdx].pos_rot.m_posit3);
			vecLightPos -= modelBoundSphere.pointOrigin;
			float fDistance = vecLightPos.Abs().GetMax();
			if (fDistance < (m_dynLights[dwIdx].fRadius + modelBoundSphere.fRadius))
			{
				// model je v dosahu dyn svetla
				shader_lights.light_pos_rot[intShaderLightCount] = m_dynLights[dwIdx].pos_rot;
				shader_lights.light_radius[intShaderLightCount] = m_dynLights[dwIdx].fRadius;

				P3DColorValueRGB shaderColor = m_dynLights[dwIdx].color;
				if (m_dynLights[dwIdx].bFlickerON)
				{
					// flickering modulation
					shaderColor.r *= m_dynLights[dwIdx].fFlickerMultiplier;
					shaderColor.g *= m_dynLights[dwIdx].fFlickerMultiplier;
					shaderColor.b *= m_dynLights[dwIdx].fFlickerMultiplier;
				}
				shader_lights.light_color[intShaderLightCount] = shaderColor;

				if (m_dynLights[dwIdx].pLightTexture)
					shader_lights.light_textures[intShaderLightCount] = m_dynLights[dwIdx].pLightTexture;
				else
					shader_lights.light_textures[intShaderLightCount] = NULL;
				if (++intShaderLightCount >= MAX_DYN_LIGHTS_PER_FACE)
					break;
			}
		}
		// nastavenie zvysnych light textur na NULL
		for (i=intShaderLightCount; i<MAX_DYN_LIGHTS_PER_FACE; ++i)
			shader_lights.light_textures[i] = NULL;

		shader_lights.light_count = intShaderLightCount;
	}
	return (intShaderLightCount !=0);
}

// pre dany bsp face vrati informacie potrebne pre shader
// vrati true ak je dany face osvetleny dyn. svetlom
//---------------------------------
bool CP3DLightManager::GetFaceLightInfo (int intBSPFace, SHADER_DYNLIGHT_INPUT &shader_lights)
{
	if (!m_lightedFaces.On (intBSPFace))
		return false;

	if (m_dwActiveLightsNum)
	{
		// urcenie zoznamu svetiel, ktore osvetluju dany face
		DWORD i;
		int intShaderLightCount = 0;
		for (i=0; i<m_dwActiveLightsNum; ++i)
		{
			DWORD dwIdx = m_dwActiveLightsIdxs[i];
			if (m_dynLights[dwIdx].lit_faces.On(intBSPFace))
			{
				shader_lights.light_pos_rot[intShaderLightCount] = m_dynLights[dwIdx].pos_rot;
				shader_lights.light_radius[intShaderLightCount] = m_dynLights[dwIdx].fRadius;

				P3DColorValueRGB shaderColor = m_dynLights[dwIdx].color;
				if (m_dynLights[dwIdx].bFlickerON)
				{
					// flickering modulation
					shaderColor.r *= m_dynLights[dwIdx].fFlickerMultiplier;
					shaderColor.g *= m_dynLights[dwIdx].fFlickerMultiplier;
					shaderColor.b *= m_dynLights[dwIdx].fFlickerMultiplier;
				}
				shader_lights.light_color[intShaderLightCount] = shaderColor;

				if (m_dynLights[dwIdx].pLightTexture)
					shader_lights.light_textures[intShaderLightCount] = m_dynLights[dwIdx].pLightTexture;
				else
					shader_lights.light_textures[intShaderLightCount] = NULL;
				if (++intShaderLightCount >= MAX_DYN_LIGHTS_PER_FACE)
					break;
			}
		}
		// nastavenie zvysnych light textur na NULL
		for (i=intShaderLightCount; i<MAX_DYN_LIGHTS_PER_FACE; ++i)
			shader_lights.light_textures[i] = NULL;

		shader_lights.light_count = intShaderLightCount;
	}
	return true;
}
