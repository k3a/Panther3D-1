//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Helper class definition
//-----------------------------------------------------------------------------


#include "common.h"
#include "..\\game\\eBase.h"
#include "helperobject.h"
#include "..\\shared\\IP3DMaterialManager.h"
#include <d3dx9.h>

REGISTER_SINGLE_CLASS(CP3DHelperObject, IP3DENGINE_HELPER_OBJECT); // zaregistruj tuto tøídu


D3DXMATRIX g_matTurret[2];

//---------------------------------
bool CP3DHelperObject::Init ()
{
	D3DXMatrixIdentity (&g_matTurret[0]);
	D3DXMatrixIdentity (&g_matTurret[1]);
	return true;
}

//---------------------------------
void CP3DHelperObject::Loop ()
{
/*
	D3DXMATRIX	matTemp;
	if (g_pInput)
	{
		// turret left
		if (g_pInput->IsKeyDown (DIK_J))
		{
			D3DXMatrixRotationY (&matTemp, -0.005f);
			g_matTurret[0] *= matTemp;
		}
		// turret right
		if (g_pInput->IsKeyDown (DIK_L))
		{
			D3DXMatrixRotationY (&matTemp, 0.005f);
			g_matTurret[0] *= matTemp;
		}
		// gun up
		if (g_pInput->IsKeyDown (DIK_I))
		{
			D3DXMatrixRotationX (&matTemp, 0.005f);
			g_matTurret[1] *= matTemp;
		}
		// gun up
		if (g_pInput->IsKeyDown (DIK_K))
		{
			D3DXMatrixRotationX (&matTemp, -0.005f);
			g_matTurret[1] *= matTemp;
		}

		//////////////////////////////////////////////////////////////////////////
		int intAnimation = 0xFFFFFFFF;
		if (g_pInput->IsKeyDownFirstTime (DIK_1))
			intAnimation = 0;
		if (g_pInput->IsKeyDownFirstTime (DIK_2))
			intAnimation = 1;
		if (g_pInput->IsKeyDownFirstTime (DIK_3))
			intAnimation = 2;

		eBase *pEntity = g_pEntMgr->FindEntityByClassname ("misc_model_dynamic");
		for (; pEntity; pEntity=pEntity->GetNext())
			pEntity->TestFunc (0, intAnimation, g_matTurret);
	}
*/

/*
	//////////////////////////////////////////////////////////////////////////
	// FORCE TO USE THIS MATERIAL
	//////////////////////////////////////////////////////////////////////////

	static DWORD dwNewMaterial = 0xFFFFFFFF;
	static bool bSwitch = true;
	DWORD dwMaterialToSet = 0xFFFFFFFF;

	if (g_pInput)
		if (g_pInput->IsKeyDownFirstTime (DIK_Z))
		{
			if (bSwitch)
			{
				if (dwNewMaterial == 0xFFFFFFFF)
				{
					IP3DMaterialManager *pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
					if (pMaterialManager)
						dwNewMaterial = pMaterialManager->LoadMaterial ("materials/models/dyn_skin_def.7mt");
				}
				dwMaterialToSet = dwNewMaterial;
			}

			bSwitch = !bSwitch;

			// tymto sposobom sa prechadza zoznam dyn. modelov
			//////////////////////////////////////////////////////////////////////////
			eBase *pEntity = g_pEntMgr->FindEntityByClassname ("misc_model_dynamic");
			for (; pEntity; pEntity=pEntity->GetNext())
				pEntity->TestFunc (0, (int)dwMaterialToSet);
		}
*/


/*
	//////////////////////////////////////////////////////////////////////////
	// FINDFIRSTFILE
	//////////////////////////////////////////////////////////////////////////
	static bool bSwitch = false;

	if (g_pInput)
		if (g_pInput->IsKeyDownFirstTime (DIK_Z))
		{
			if (bSwitch) return;
			bSwitch = true;
			// DEBUG FS
			int nValid;
			HANDLE searchHandle=NULL;
			WIN32_FIND_DATA fileData;

			char pathToSearch[MAX_PATH];
			//strcpy(pathToSearch,"shaders/bsplm_*.fx");
			strcpy(pathToSearch,"asd*.txt");

			searchHandle = g_pFS->FindFirstFile(pathToSearch, &fileData);
			nValid = (searchHandle == INVALID_HANDLE_VALUE) ? 0 : 1;

			while (nValid)
			{
				if (fileData.dwFileAttributes & FILE_ATTRIBUTE_PACK)
					MessageBox(0, fileData.cFileName, "PACK", 0);
				else
					MessageBox(0, fileData.cFileName, "SYSTEM", 0);
				// neni - dalsi soubor
				nValid = g_pFS->FindNextFile(searchHandle, &fileData);
			}
			g_pFS->FindClose(searchHandle);
		}
*/

/*
	//////////////////////////////////////////////////////////////////////////
	// SOUND
	//////////////////////////////////////////////////////////////////////////
	static bool bLoaded = false;
	if (!bLoaded)
	{
		unsigned int intSoundID;
		intSoundID = g_pSoundMan->LoadSound ("electro_station");
		intSoundID = g_pSoundMan->LoadSound ("button_on");
		intSoundID = g_pSoundMan->LoadSound ("button_off");

		static IP3DSoundContainer *pSoundContainer = (IP3DSoundContainer*)I_GetClass(IP3DSOUND_SOUNDCONTAINER);
		if (pSoundContainer)
		{
			pSoundContainer->m_3DPos = P3DXVector3D(-1170, 110, -380);
			pSoundContainer->PlaySound ("electro_station", true);
		}
		bLoaded = true;
	}

	if (g_pInput->IsKeyDownFirstTime (DIK_Z))
		g_pSoundMan->PlaySound ("button_on");
	else if (g_pInput->IsKeyDownFirstTime (DIK_X))
		g_pSoundMan->PlaySound ("button_off");
*/
}
