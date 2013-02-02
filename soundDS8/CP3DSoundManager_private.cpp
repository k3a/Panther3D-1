
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DSoundManager definition
//-----------------------------------------------------------------------------



#include "CP3DSoundManager.h"

void UpdateStreamWaveBuffer (SOUND_ITEM &pStreamItem, DWORD dwBufferIdx, bool bUpdateLowHalf);
void UpdateStreamOggBuffer (SOUND_ITEM &pStreamItem, DWORD dwBufferIdx, bool bUpdateLowHalf);
void InitFirstStreamSecond (SOUND_ITEM &pStreamItem, DWORD dwBufferIdx);


extern bool		g_bLastConVarValue;


//---------------------------------
void CP3DSoundManager::SoundFX_Change_Callback (ConVar* pCVar)
{
	if (g_bLastConVarValue != CVs_soundfx.GetBool())
	{
		g_bLastConVarValue = CVs_soundfx.GetBool();
		// TODO: spracovat zmenu nastavenia efektov
		// ...
		// pridat prepnutie bNoneEffectSet pre vsetky bufre ???
	}
}

//---------------------------------
bool CP3DSoundManager::UnloadSound (DWORD dwSoundIdx)
{
	if (dwSoundIdx >= m_dwHashTableItemsCount)
		return true;		// polozka je uz neplatna

	// vycistit polozky v active liste ak je to potrebne
	for (DWORD i=0; i<ACTIVE_LIST_SIZE; i++)
		if (DWORD(m_ActiveList[i].intHashTableIdx) == dwSoundIdx)
			RemoveFromActiveList (i);

	m_SoundHashTable[dwSoundIdx].Unload ();

	// posun nasledujucich zvukov pre zachovanie zoradenej postupnosti
	int intItemsToCopy = m_dwHashTableItemsCount - dwSoundIdx - 1;
	if (intItemsToCopy > 0)
	{
		memcpy (&m_SoundHashTable[dwSoundIdx], &m_SoundHashTable[dwSoundIdx+1], intItemsToCopy * sizeof(SOUND_ITEM));
		m_SoundHashTable[m_dwHashTableItemsCount-1].Clear ();
	}

	m_dwHashTableItemsCount--;

	if (intItemsToCopy > 0)
		RecalculateActiveListIndexes ();

	return true;
}

// v parametri dwHashTableIdx vrati index do tabulky m_SoundHashTable
//---------------------------------
inline bool CP3DSoundManager::FindSoundByName (IN const char *strSoundName, OUT DWORD &dwHashTableIdx)
{
	if ((strSoundName == NULL) || (strSoundName[0] == 0)) return false;
	return FindSoundByID (StringHashFunc (strSoundName), dwHashTableIdx);
}

// v parametri dwHashTableIdx vrati index do tabulky m_SoundHashTable
//---------------------------------
inline bool CP3DSoundManager::FindSoundByID (IN unsigned int dwSoundID, OUT DWORD &dwHashTableIdx)
{
	int intHashTableIdx = -1;

	if (m_dwHashTableItemsCount > 15)
	{
		// binary search
		int middle;
		int left = 0;
		int right = int(m_dwHashTableItemsCount) - 1;

		while (left <= right)
		{
			middle = (left + right) / 2;
			if (m_SoundHashTable[middle].dwSoundID < dwSoundID)
				left = middle+1;
			else if (m_SoundHashTable[middle].dwSoundID > dwSoundID)
				right = middle-1;
			else
			{intHashTableIdx = middle; break;}
		}
	}
	else
	{
		// pre maly pocet prvkov sa pouzije rychlejsie vyhladavanie
		for (register int i=0; i<(int)m_dwHashTableItemsCount; i++)
			if (m_SoundHashTable[i].dwSoundID == dwSoundID)
			{intHashTableIdx = i; break;}
	}

	if (intHashTableIdx == -1) return false;

	dwHashTableIdx = (DWORD)intHashTableIdx;
	return true;
}

// TODO: FIXME: linker neberie funkcie FindSoundBy*InActiveList ako inline
// vrati index zvuku v active liste, ak sa zvuk nenajde vrati -1
//---------------------------------
int CP3DSoundManager::FindSoundByNameInActiveList (const char *strSoundName, IP3DSoundContainer *pOwner)
{
	if ((strSoundName == NULL) || (strSoundName[0] == 0))
		return -1;
	for (register int i=0; i<ACTIVE_LIST_SIZE; i++)
		if ((m_ActiveList[i].pOwner == pOwner) && (m_ActiveList[i].intHashTableIdx >= 0))
		{
			char *strName = m_SoundHashTable[m_ActiveList[i].intHashTableIdx].strSoundName;
			for (register DWORD j=0; j<DEF_SOUNDNAME_SIZE; j++)
				if (strName[j] != strSoundName[j])
					break;
				else if ((strSoundName[j] == 0) || (j == DEF_SOUNDNAME_SIZE - 1))
					return i;
		}

		return -1;
}

// vrati index zvuku v active liste, v pripade chyby -1
//---------------------------------
int CP3DSoundManager::FindSoundByIDInActiveList (unsigned int dwSoundID, IP3DSoundContainer *pOwner)
{
	for (register int i=0; i<ACTIVE_LIST_SIZE; i++)
		if ((m_ActiveList[i].pOwner == pOwner) && (m_ActiveList[i].dwSoundID == dwSoundID))
			return i;
	return -1;
}

// vrati index zvuku v active liste, v pripade chyby -1
//---------------------------------
int CP3DSoundManager::FindSoundByHandleInActiveList (P3D_SOUND_HANDLE dwSoundHandle)
{
	for (register int i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].dwSoundHandle == dwSoundHandle)
			return i;
	return -1;
}

// nastavi effekty pre dany DirectSound buffer
// ak bSetMasterEffects == true tak sa nastavia master efekty
// ak bSetMasterEffects == false tak sa k nastavenym master efektom pridaju efekty definovane v pEffects
//---------------------------------
bool CP3DSoundManager::SetEffectOnBuffer (LPDIRECTSOUNDBUFFER8 pSoundBuffer8, bool bSetMasterEffects, \
										  DWORD dwEffectsNum, P3D_SFX_DESC *pEffects)
{
	HRESULT	hr;
	DWORD		i, dwStatus = 0;

	if (!CVs_soundfx.GetBool ()) return false;
	if (pSoundBuffer8 == NULL) return false;

	// urcenie ake efekty je vlastne potrebne nastavit
	//////////////////////////////////////////////////////////////////////////
	P3D_SFX_DESC	effectsToSet[P3D_SOUNDFX_COUNT];
	DWORD				dwEffectsToSetNum = 0;

	if (bSetMasterEffects)		// nastavit len master efekty
	{
		dwEffectsToSetNum = m_dwMasterEffectsNum;
		if (m_dwMasterEffectsNum)
			memcpy (effectsToSet, m_MasterEffects, sizeof(P3D_SFX_DESC) * m_dwMasterEffectsNum);
	}
	else								// k master efektom pridat dane efekty
	{
		if ((dwEffectsNum > 0) && (pEffects == NULL)) return false;

		dwEffectsToSetNum = m_dwMasterEffectsNum + dwEffectsNum;
		if (m_dwMasterEffectsNum)
			memcpy (effectsToSet, m_MasterEffects, sizeof(P3D_SFX_DESC) * m_dwMasterEffectsNum);
		if (dwEffectsNum)
			memcpy (&effectsToSet[m_dwMasterEffectsNum], pEffects, sizeof(P3D_SFX_DESC) * dwEffectsNum);
	}
	//////////////////////////////////////////////////////////////////////////

	// pozastavenie zvuku
	if (SUCCEEDED (pSoundBuffer8->GetStatus (&dwStatus)))
		if (dwStatus & DSBSTATUS_PLAYING)
			pSoundBuffer8->Stop ();

	// nastavenie efektu
	if (dwEffectsToSetNum == 0)
		hr = pSoundBuffer8->SetFX (0, NULL, NULL);
	else
	{
		DWORD				dwResults[P3D_SOUNDFX_COUNT] = {0};
		DSEFFECTDESC	effect_desc[P3D_SOUNDFX_COUNT];
		memset (effect_desc, 0, sizeof(effect_desc));

		for (i=0; i<dwEffectsToSetNum; i++)
		{
			effect_desc[i].dwSize = sizeof(DSEFFECTDESC);

			switch (effectsToSet[i].effectID)
			{
			case SFX_ID_CHORUS:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_CHORUS; break;
			case SFX_ID_COMPRESSOR:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_COMPRESSOR; break;
			case SFX_ID_DISTORTION:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_DISTORTION; break;
			case SFX_ID_ECHO:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_ECHO; break;
			case SFX_ID_FLANGER:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_FLANGER; break;
			case SFX_ID_GARGLE:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_GARGLE; break;
			case SFX_ID_I3DL2REVERB:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_I3DL2REVERB; break;
			case SFX_ID_PARAMEQ:
				effect_desc[i].guidDSFXClass = GUID_DSFX_STANDARD_PARAMEQ; break;
			case SFX_ID_REVERB:
				effect_desc[i].guidDSFXClass = GUID_DSFX_WAVES_REVERB; break;
			default:
				return false;
			}
		}

		hr = pSoundBuffer8->SetFX (dwEffectsToSetNum, effect_desc, dwResults);

#ifdef _DEBUG
		// ak nie je mozne nastavit aspon jeden z efektov potom asi fcia SetFX() nenastavi ziaden
		for (i=0; i<dwEffectsToSetNum; i++)
			if ((dwResults[i] == DSFXR_FAILED) || (dwResults[i] == DSFXR_PRESENT) || (dwResults[i] == DSFXR_UNKNOWN))
			{
				CON (MSG_CON_DEBUG, "Can't set all sound effects - report this to programmers");
				break;
			}
#endif


			// nastavenie parametrov efektu
			if (SUCCEEDED (hr))
			{
				LPDIRECTSOUNDFXCHORUS			IChorus;
				LPDIRECTSOUNDFXCOMPRESSOR		ICompressor;
				LPDIRECTSOUNDFXDISTORTION		IDistortion;
				LPDIRECTSOUNDFXECHO				IEcho;
				LPDIRECTSOUNDFXFLANGER			IFlanger;
				LPDIRECTSOUNDFXGARGLE			IGargle;
				LPDIRECTSOUNDFXI3DL2REVERB		IReverb;
				LPDIRECTSOUNDFXPARAMEQ			IParamQ;
				LPDIRECTSOUNDFXWAVESREVERB		IWaveReverb;

				for (i=0; i<dwEffectsToSetNum; i++)
				{
					switch (effectsToSet[i].effectID)
					{
					case SFX_ID_CHORUS:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_CHORUS, 0, IID_IDirectSoundFXChorus, (LPVOID*)&IChorus)))
						{hr = IChorus->SetAllParameters ((LPCDSFXChorus)&effectsToSet[i].chorusParams); IChorus->Release ();}
						break;
					case SFX_ID_COMPRESSOR:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_COMPRESSOR, 0, IID_IDirectSoundFXCompressor, (LPVOID*)&ICompressor)))
						{hr = ICompressor->SetAllParameters ((LPCDSFXCompressor)&effectsToSet[i].compressorParams); ICompressor->Release ();}
						break;
					case SFX_ID_DISTORTION:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_DISTORTION, 0, IID_IDirectSoundFXDistortion, (LPVOID*)&IDistortion)))
						{hr = IDistortion->SetAllParameters ((LPCDSFXDistortion)&effectsToSet[i].distortionParams); IDistortion->Release ();}
						break;
					case SFX_ID_ECHO:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_ECHO, 0, IID_IDirectSoundFXEcho, (LPVOID*)&IEcho)))
						{hr = IEcho->SetAllParameters ((LPCDSFXEcho)&effectsToSet[i].echoParams); IEcho->Release ();}
						break;
					case SFX_ID_FLANGER:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_FLANGER, 0, IID_IDirectSoundFXFlanger, (LPVOID*)&IFlanger)))
						{hr = IFlanger->SetAllParameters ((LPCDSFXFlanger)&effectsToSet[i].flangerParams); IFlanger->Release ();}
						break;
					case SFX_ID_GARGLE:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_GARGLE, 0, IID_IDirectSoundFXGargle, (LPVOID*)&IGargle)))
						{hr = IGargle->SetAllParameters ((LPCDSFXGargle)&effectsToSet[i].gargleParams); IGargle->Release ();}
						break;
					case SFX_ID_I3DL2REVERB:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_I3DL2REVERB, 0, IID_IDirectSoundFXI3DL2Reverb, (LPVOID*)&IReverb)))
						{hr = IReverb->SetAllParameters ((LPCDSFXI3DL2Reverb)&effectsToSet[i].reverb2Params); IReverb->Release ();}
						break;
					case SFX_ID_PARAMEQ:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_STANDARD_PARAMEQ, 0, IID_IDirectSoundFXParamEq, (LPVOID*)&IParamQ)))
						{hr = IParamQ->SetAllParameters ((LPCDSFXParamEq)&effectsToSet[i].paramqParams); IParamQ->Release ();}
						break;
					case SFX_ID_REVERB:
						if (SUCCEEDED (hr = pSoundBuffer8->GetObjectInPath (GUID_DSFX_WAVES_REVERB, 0, IID_IDirectSoundFXWavesReverb, (LPVOID*)&IWaveReverb)))
						{hr = IWaveReverb->SetAllParameters ((LPCDSFXWavesReverb)&effectsToSet[i].reverbParams); IWaveReverb->Release ();}
					}
				}
			}
	}

	// spustenie zvuku
	if (dwStatus & DSBSTATUS_PLAYING)
		pSoundBuffer8->Play (0, 0, (dwStatus & DSBSTATUS_LOOPING) ? DSBPLAY_LOOPING : 0);

	return SUCCEEDED (hr);
}

// vlozi novy prvok do hash tabulky tak aby bola zachovana zoradena postupnost podla dwSoundID
//---------------------------------
bool CP3DSoundManager::InsertNewSoundItem (const SOUND_ITEM	&newSoundItem)
{
	if (m_dwHashTableItemsCount == HASH_TABLE_SIZE) return false;	// tabulka je plna
	if ((newSoundItem.strSoundName[0] == 0) || (newSoundItem.dwSoundID == 0) || \
		(newSoundItem.ucSoundCount == 0) || (newSoundItem.pSoundBufferList == NULL))
		return false;

	DWORD dwItemsToMove, dwSoundIdx = m_dwHashTableItemsCount;

	for (DWORD i=0; i<m_dwHashTableItemsCount; i++)
		if (m_SoundHashTable[i].dwSoundID > newSoundItem.dwSoundID)
			{dwSoundIdx = i; break;}

	dwItemsToMove = m_dwHashTableItemsCount - dwSoundIdx;
	if (dwItemsToMove)
	{
		// posun prvkov ak je to potrebne
		SOUND_ITEM tempSoundItem1, tempSoundItem2;
		tempSoundItem1 = m_SoundHashTable[dwSoundIdx];
		for (DWORD j=1; j<=dwItemsToMove; j++)
		{
			tempSoundItem2 = m_SoundHashTable[dwSoundIdx + j];
			m_SoundHashTable[dwSoundIdx + j] = tempSoundItem1;
			tempSoundItem1 = tempSoundItem2;
		}
	}

	m_SoundHashTable[dwSoundIdx] = newSoundItem;
	m_dwHashTableItemsCount++;
	if (dwItemsToMove)
		RecalculateActiveListIndexes ();

	return true;
}

// potrebne volat po kazdej zmene poradia prvkov v m_SoundHashTable (pridanie/odobranie prvku)
//---------------------------------
void CP3DSoundManager::RecalculateActiveListIndexes()
{
	for (DWORD i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].dwSoundID)
		{
			if (m_ActiveList[i].intHashTableIdx >= 0)
				if (m_ActiveList[i].dwSoundID == m_SoundHashTable[m_ActiveList[i].intHashTableIdx].dwSoundID)
					continue;	// ak sa index nezmenil tak nevyhladavat

			DWORD dwSoundIdx;
			if (FindSoundByID (m_ActiveList[i].dwSoundID, dwSoundIdx))
				m_ActiveList[i].intHashTableIdx = (int)dwSoundIdx;
			else
				RemoveFromActiveList (i);
		}
}

// odstrani zvuk z active listu - ukonci zvuk
//---------------------------------
void CP3DSoundManager::RemoveFromActiveList (int intIdx)
{
	if ((intIdx < 0) || (intIdx >= ACTIVE_LIST_SIZE))
		return;

	// zastavit prehravany zvuk
	if (m_ActiveList[intIdx].intHashTableIdx >= 0)
	{
		SOUND_ITEM &pListItem = m_SoundHashTable[m_ActiveList[intIdx].intHashTableIdx];

		if (m_ActiveList[intIdx].bDuplicatedBuffer)
		{
			SOUND_BUFFER &pSoundBufferItem = m_ActiveList[intIdx].DuplicatedBuffer;
			if (pSoundBufferItem.pSound3DBuffer)
				pSoundBufferItem.pSound3DBuffer->Release ();
			if (pSoundBufferItem.pSoundBuffer)
				pSoundBufferItem.pSoundBuffer->Release ();
		}
		else if ((m_ActiveList[intIdx].wSoundBufferIdx < pListItem.ucSoundCount) && \
			(pListItem.pSoundBufferList != NULL))
		{
			SOUND_BUFFER &pSoundBufferItem = pListItem.pSoundBufferList[m_ActiveList[intIdx].wSoundBufferIdx];

			if (pListItem.bStream)
				InitFirstStreamSecond (pListItem, m_ActiveList[intIdx].wSoundBufferIdx);
			else if (pSoundBufferItem.pSoundBuffer)
			{
				pSoundBufferItem.pSoundBuffer->Stop ();
				pSoundBufferItem.pSoundBuffer->SetCurrentPosition (0);
			}
			pSoundBufferItem.intActiveListIdx = -1;
		}
	}

	m_ActiveList[intIdx].Clear ();
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySoundInternal (DWORD dwActiveListIdx, IP3DSoundContainer *pOwner, bool bLoop)
{
	SOUND_ITEM	&pSoundItem = m_SoundHashTable[m_ActiveList[dwActiveListIdx].intHashTableIdx];
	if (pSoundItem.pSoundBufferList)
	{
		SOUND_BUFFER &pSndBuffer = pSoundItem.pSoundBufferList[m_ActiveList[dwActiveListIdx].wSoundBufferIdx];

		if (pSoundItem.bStream)
			InitFirstStreamSecond (pSoundItem, m_ActiveList[dwActiveListIdx].wSoundBufferIdx);
		else
		{
			LPDIRECTSOUNDBUFFER8	pSoundBuffer8 = pSndBuffer.pSoundBuffer;
			if (pSoundBuffer8)
			{
				pSoundBuffer8->Stop ();
				pSoundBuffer8->SetCurrentPosition (0);
			}
		}
		// novy sound handle
		m_ActiveList[dwActiveListIdx].dwSoundHandle = m_SoundHandleProvider.Get();

		m_ActiveList[dwActiveListIdx].wFlags |= PLAY_ON_UPDATE;
		m_ActiveList[dwActiveListIdx].wFlags &= ~CLEARANCE_CANDIDATE;
		if (bLoop)
			m_ActiveList[dwActiveListIdx].wFlags |= PLAY_IN_LOOP;
		else
			m_ActiveList[dwActiveListIdx].wFlags &= ~PLAY_IN_LOOP;			

		// nastavenie master volume a master effects
		UpdateMasterVolumeEffects (pSoundItem, pSndBuffer);

		return m_ActiveList[dwActiveListIdx].dwSoundHandle;
	}
	else
		RemoveFromActiveList (dwActiveListIdx);

	return 0;
}

// vytvori docasnu kopiu sound bufru, pouziva sa ak prave ziaden bufer nie je k dispozicii
// TODO: k funkcii pridat profiler!
//---------------------------------
bool CP3DSoundManager::CreateTemporaryBuffer (IN const SOUND_ITEM &SoundItem, OUT SOUND_BUFFER &SoundBuffer)
{
	if ((SoundItem.pSoundBufferList == NULL) || (SoundItem.pSoundBufferList[0].pSoundBuffer == NULL))
		return false;

	SoundBuffer.Clear ();

	SOUND_BUFFER &pSoundBufferOriginal = SoundItem.pSoundBufferList[0];
	if (FAILED (m_pDS->DuplicateSoundBuffer (pSoundBufferOriginal.pSoundBuffer, (LPDIRECTSOUNDBUFFER*)&SoundBuffer.pSoundBuffer)))
		return false;

	// query 3D interface
	if ((SoundItem.dwCreationFlags & DSBCAPS_CTRL3D) && (pSoundBufferOriginal.pSound3DBuffer != NULL))
	{
		if (FAILED (SoundBuffer.pSoundBuffer->QueryInterface (IID_IDirectSound3DBuffer8, \
			(LPVOID*)&SoundBuffer.pSound3DBuffer)))
			goto err_quit;
		// nastavenie min a max dosahu 3D bufru
		float fBuf = DEF_SOUND_MIN_DISTANCE;
		pSoundBufferOriginal.pSound3DBuffer->GetMinDistance (&fBuf);
		SoundBuffer.pSound3DBuffer->SetMinDistance (fBuf, DS3D_DEFERRED);
		fBuf = DEF_SOUND_MAX_DISTANCE;
		pSoundBufferOriginal.pSound3DBuffer->GetMaxDistance (&fBuf);
		SoundBuffer.pSound3DBuffer->SetMaxDistance (fBuf, DS3D_DEFERRED);
	}

	/*
	// zatial sound engine meumoznuje kopirovanie stream bufrov
	if (SoundItem.bStream)
	{
	// TODO: volat InitFirstStreamSecond () ???
	SoundBuffer.dwCurrentFileOffset = SoundItem.dwUpdateBlockSize * 2;
	// vytvorenie notify udalosti - iba pre stream bufre
	DSBPOSITIONNOTIFY		PositionNotify[2];
	LPDIRECTSOUNDNOTIFY8	lpDsNotify;
	PositionNotify[0].dwOffset = SoundItem.dwUpdateBlockSize;
	PositionNotify[0].hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
	PositionNotify[1].dwOffset = SoundItem.dwUpdateBlockSize * 2 - 2;
	PositionNotify[1].hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
	if ((PositionNotify[0].hEventNotify == NULL) || (PositionNotify[1].hEventNotify == NULL))
	goto err_quit;

	if (FAILED (SoundBuffer.pSoundBuffer->QueryInterface (IID_IDirectSoundNotify8, (LPVOID*)&lpDsNotify)))
	goto err_quit;
	if (FAILED (lpDsNotify->SetNotificationPositions (2, (LPCDSBPOSITIONNOTIFY)&PositionNotify)))
	{lpDsNotify->Release (); goto err_quit;}
	lpDsNotify->Release ();
	SoundBuffer.hUpdateEvents[0] = PositionNotify[0].hEventNotify;
	SoundBuffer.hUpdateEvents[1] = PositionNotify[1].hEventNotify;
	}
	*/

	return true;

err_quit:
	SAFE_RELEASE (SoundBuffer.pSoundBuffer)
		SAFE_RELEASE (SoundBuffer.pSound3DBuffer)
		return false;
}

// nastavenie master volume a effects
//---------------------------------
inline void CP3DSoundManager::UpdateMasterVolumeEffects (const SOUND_ITEM &pSoundItem, SOUND_BUFFER &pSoundBuffer)
{
	if ((pSoundItem.dwCreationFlags & P3DBCAPS_CTRLVOLUME) && (pSoundBuffer.pSoundBuffer != NULL))
		if ((m_lMasterVolume != DSBVOLUME_MAX) || (!pSoundBuffer.bDefVolumeSet))
		{
			pSoundBuffer.pSoundBuffer->SetVolume (m_lMasterVolume);
			if (m_lMasterVolume == DSBVOLUME_MAX)
				pSoundBuffer.bDefVolumeSet = true;
		}

		if (pSoundItem.dwCreationFlags & P3DBCAPS_CTRLFX)
		{
			if ((m_dwMasterEffectsNum != 0) || (!pSoundBuffer.bNoneEffectSet))	// nenastavovat "ziaden" efekt ak ziaden nie je nastaveny
				if (SetEffectOnBuffer (pSoundBuffer.pSoundBuffer))
					pSoundBuffer.bNoneEffectSet = (m_dwMasterEffectsNum == 0) ? true : false;
		}
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlayFreeSample (const char *strSoundName, IP3DSoundContainer *pOwner, P3DXVector3D *pSoundPos, bool bLoop)
{
	DWORD dwSoundIdx;
	if (!FindSoundByName (strSoundName, dwSoundIdx))
		return 0;
	return PlayFreeSample (dwSoundIdx, pOwner, pSoundPos, bLoop);
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlayFreeSample (unsigned int dwSoundID, IP3DSoundContainer *pOwner, P3DXVector3D *pSoundPos, bool bLoop)
{
	DWORD dwSoundIdx;
	if (!FindSoundByID (dwSoundID, dwSoundIdx))
		return 0;
	return PlayFreeSample (dwSoundIdx, pOwner, pSoundPos, bLoop);
}

// najde volny sound buffer daneho zvuku, prida ho do active listu a oznaci ako urceny na prehravanie
//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlayFreeSample (DWORD dwSoundIdx, IP3DSoundContainer *pOwner, P3DXVector3D *pSoundPos, bool bLoop)
{
	DWORD i;
	int intFreeBufferIdx = -1, intFreeActiveListIdx = -1;;

	SOUND_ITEM &SoundItem = m_SoundHashTable[dwSoundIdx];

	if (SoundItem.pSoundBufferList == NULL)
		return 0;
	// pre 3D zvuky je treba definovat vlastnika alebo poziciu
	if ((pOwner == NULL) && (pSoundPos == NULL) && (SoundItem.dwCreationFlags & P3DBCAPS_CTRL3D))
		return 0;

	for (i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].dwSoundID == 0)
		{intFreeActiveListIdx = i; break;}		// volna polozka v active liste

		if (intFreeActiveListIdx == -1)
			return 0;							// prave sa prehrava ACTIVE_LIST_SIZE zvukov

		for (i=0; i<SoundItem.ucSoundCount; i++)
			if (SoundItem.pSoundBufferList[i].intActiveListIdx < 0)		// buffer je volny
			{intFreeBufferIdx = i; break;}

			if (intFreeBufferIdx >= 0)			// zaradenie zvuku do volnej polozky active-listu
			{
				m_ActiveList[intFreeActiveListIdx].dwSoundID = SoundItem.dwSoundID;
				m_ActiveList[intFreeActiveListIdx].dwSoundHandle = m_SoundHandleProvider.Get();
				m_ActiveList[intFreeActiveListIdx].intHashTableIdx = dwSoundIdx;
				m_ActiveList[intFreeActiveListIdx].wSoundBufferIdx = intFreeBufferIdx;
				m_ActiveList[intFreeActiveListIdx].wFlags = PLAY_ON_UPDATE;
				if (bLoop) m_ActiveList[intFreeActiveListIdx].wFlags |= PLAY_IN_LOOP;
				m_ActiveList[intFreeActiveListIdx].pOwner = (CP3DSoundContainer*)pOwner;
				m_ActiveList[intFreeActiveListIdx].bDuplicatedBuffer = false;
				if (pSoundPos)
				{
					LPDIRECTSOUND3DBUFFER8	p3DBuffer;
					if (p3DBuffer = SoundItem.pSoundBufferList[intFreeBufferIdx].pSound3DBuffer)
						p3DBuffer->SetPosition (pSoundPos->x, pSoundPos->y, pSoundPos->z, DS3D_DEFERRED);
				}

				SOUND_BUFFER	&pSndBuffer = SoundItem.pSoundBufferList[intFreeBufferIdx];

				pSndBuffer.intActiveListIdx = intFreeActiveListIdx;

				// nastavenie master volume a master effects
				UpdateMasterVolumeEffects (SoundItem, pSndBuffer);

				return m_ActiveList[intFreeActiveListIdx].dwSoundHandle;
			}

			//////////////////////////////////////////////////////////////////////////
			// nebol najdeny ziaden volny buffer
			// je treba vybrat jeden buffer z prave prehravanych - podla danych kriterii

#define P3DSAMPLE_OVER_DUPLICATE					100
#define P3DSAMPLE_OVER_PLAYPOS					101

			DWORD dwOverridePriority;
			if (SoundItem.ucSampleOverPrior == P3DSAMPLE_OVER_DEFAULT)
			{
				if (((SoundItem.dwCreationFlags & P3DBCAPS_CTRLFX) == 0) && (!SoundItem.bStream))
					// TODO: je OVER_DUPLICATE najlepsie riesenie ???
					dwOverridePriority = P3DSAMPLE_OVER_DUPLICATE;	// spravit docasnu kopiu cez DuplicateSoundBuffer()
				else if (SoundItem.dwCreationFlags & P3DBCAPS_CTRL3D)
					dwOverridePriority = P3DSAMPLE_OVER_3DPOS;
				else if ((!SoundItem.bStream) && (!bLoop))
					dwOverridePriority = P3DSAMPLE_OVER_PLAYPOS;
				else
					dwOverridePriority = P3DSAMPLE_OVER_FORCE_FIRST_BUFFER;
			}
			else
				dwOverridePriority = SoundItem.dwCreationFlags;
			//////////////////////////////////////////////////////////////////////////

			if (dwOverridePriority == P3DSAMPLE_OVER_NONE)
				return 0;
			else if (dwOverridePriority == P3DSAMPLE_OVER_FORCE_FIRST_BUFFER)
			{
				// pri nedostatku bufrov sa pouzije prvy buffer
				intFreeBufferIdx = 0;
			}
			// docasna kopia cez DuplicateSoundBuffer()
			else if (dwOverridePriority == P3DSAMPLE_OVER_DUPLICATE)
			{
				if (CreateTemporaryBuffer (SoundItem, m_ActiveList[intFreeActiveListIdx].DuplicatedBuffer))
				{
					m_ActiveList[intFreeActiveListIdx].dwSoundID = SoundItem.dwSoundID;
					m_ActiveList[intFreeActiveListIdx].dwSoundHandle = m_SoundHandleProvider.Get();
					m_ActiveList[intFreeActiveListIdx].intHashTableIdx = dwSoundIdx;
					m_ActiveList[intFreeActiveListIdx].wSoundBufferIdx = 0;
					m_ActiveList[intFreeActiveListIdx].wFlags = PLAY_ON_UPDATE;
					if (bLoop) m_ActiveList[intFreeActiveListIdx].wFlags |= PLAY_IN_LOOP;
					m_ActiveList[intFreeActiveListIdx].pOwner = (CP3DSoundContainer*)pOwner;
					m_ActiveList[intFreeActiveListIdx].bDuplicatedBuffer = true;
					m_ActiveList[intFreeActiveListIdx].DuplicatedBuffer.intActiveListIdx = intFreeActiveListIdx;
					if (pSoundPos)
					{
						LPDIRECTSOUND3DBUFFER8	p3DBuffer;
						if (p3DBuffer = m_ActiveList[intFreeActiveListIdx].DuplicatedBuffer.pSound3DBuffer)
							p3DBuffer->SetPosition (pSoundPos->x, pSoundPos->y, pSoundPos->z, DS3D_DEFERRED);
					}
					// nastavenie master volume a master effects
					UpdateMasterVolumeEffects (SoundItem, m_ActiveList[intFreeActiveListIdx].DuplicatedBuffer);

					return m_ActiveList[intFreeActiveListIdx].dwSoundHandle;
				}
				return 0;
			}
			// vyber podla vzdialenosti od listenera
			else if (dwOverridePriority == P3DSAMPLE_OVER_3DPOS)
			{
				LPDIRECTSOUND3DBUFFER8	p3DBuffer;
				P3DXVector3D				vec3DDistance;
				float							fTemp, fMaxRange = -1.0f;

				for (i=0; i<SoundItem.ucSoundCount; i++)
				{
					p3DBuffer = SoundItem.pSoundBufferList[i].pSound3DBuffer;
					if ((p3DBuffer != NULL) && (SUCCEEDED (p3DBuffer->GetPosition ((D3DVECTOR*)&vec3DDistance))))
					{
						vec3DDistance -= m_ListenerPosition;
						vec3DDistance *= ONE_METER_DIV;
						// dot product = vzdialenost... odmocnina nie je potrebna
						fTemp = vec3DDistance * vec3DDistance;
						if (fTemp > fMaxRange)
						{fMaxRange = fTemp; intFreeBufferIdx = i;}
					}
				}
			}
			// vyber podla najdlhsie prehravaneho zvuku
			// berie v uvahu aktualny stav prehravania zvuku, pre loop zvuky tento vyber nefunguje spravne
			else if (dwOverridePriority == P3DSAMPLE_OVER_PLAYPOS)
			{
				LPDIRECTSOUNDBUFFER8	pSoundBuffer8;
				DWORD						dwCurrentPos, dwMaxPlayTime = 0;

				for (i=0; i<SoundItem.ucSoundCount; i++)
				{
					pSoundBuffer8 = SoundItem.pSoundBufferList[i].pSoundBuffer;
					if ((pSoundBuffer8 != NULL) && (SUCCEEDED (pSoundBuffer8->GetCurrentPosition (&dwCurrentPos, NULL))))
						if (dwCurrentPos >= dwMaxPlayTime)
						{dwMaxPlayTime = dwCurrentPos; intFreeBufferIdx = i;}
				}
			}
			//////////////////////////////////////////////////////////////////////////
			if (intFreeBufferIdx >= 0)			// vymena vlastnikov zvuku v active-liste
			{
				int						intActiveListIndex;
				DWORD						dwStatus;
				LPDIRECTSOUNDBUFFER8	pSoundBuffer8;

				intActiveListIndex = SoundItem.pSoundBufferList[intFreeBufferIdx].intActiveListIdx;
				pSoundBuffer8 = SoundItem.pSoundBufferList[intFreeBufferIdx].pSoundBuffer;

				if ((intActiveListIndex >= 0) && (pSoundBuffer8 != NULL))
				{
					if (SoundItem.bStream)
					{
						if (SUCCEEDED (pSoundBuffer8->GetStatus (&dwStatus)))
							if (dwStatus & DSBSTATUS_PLAYING)
								InitFirstStreamSecond (SoundItem, intFreeBufferIdx);
					}
					else
					{pSoundBuffer8->Stop (); pSoundBuffer8->SetCurrentPosition (0);}

					m_ActiveList[intActiveListIndex].wSoundBufferIdx = intFreeBufferIdx;
					if (m_ActiveList[intActiveListIndex].dwSoundHandle == 0)
						m_ActiveList[intActiveListIndex].dwSoundHandle = m_SoundHandleProvider.Get();

					m_ActiveList[intActiveListIndex].wFlags = PLAY_ON_UPDATE;
					if (bLoop) m_ActiveList[intActiveListIndex].wFlags |= PLAY_IN_LOOP;
					m_ActiveList[intActiveListIndex].pOwner = (CP3DSoundContainer*)pOwner;	// novy majitel zvuku
					if (pSoundPos)
					{
						LPDIRECTSOUND3DBUFFER8	p3DBuffer;
						if (p3DBuffer = SoundItem.pSoundBufferList[intFreeBufferIdx].pSound3DBuffer)
							p3DBuffer->SetPosition (pSoundPos->x, pSoundPos->y, pSoundPos->z, DS3D_DEFERRED);
					}

					return m_ActiveList[intActiveListIndex].dwSoundHandle;
				}
			}

			return 0;
}

//////////////////////////////////////////////////////////////////////////
// inicializuje stream buffer
//////////////////////////////////////////////////////////////////////////
void InitFirstStreamSecond (SOUND_ITEM &pStreamItem, DWORD dwBufferIdx)
{
	BYTE						*pLockedPtr;
	DWORD						dwLockedSize;
	LPDIRECTSOUNDBUFFER8	pSoundBuffer;

	if ((pStreamItem.pSoundBufferList == NULL) || (dwBufferIdx >= pStreamItem.ucSoundCount) || \
		(pStreamItem.pWaveSoundData == NULL) || (pStreamItem.bStream == false))
		return;

	SOUND_BUFFER &pStreamBuffer = pStreamItem.pSoundBufferList[dwBufferIdx];
	if ((pSoundBuffer = pStreamBuffer.pSoundBuffer) == NULL) return;

	pSoundBuffer->Stop ();
	pSoundBuffer->SetCurrentPosition (0);
	pStreamBuffer.loopFlag = STREAMLOOP_NONLOOP;
	pStreamBuffer.dwCurrentFileOffset = pStreamItem.dwUpdateBlockSize * 2;
	if (pStreamBuffer.hUpdateEvents[0])
		ResetEvent (pStreamBuffer.hUpdateEvents[0]);
	if (pStreamBuffer.hUpdateEvents[1])
		ResetEvent (pStreamBuffer.hUpdateEvents[1]);

	// inicializacia - kopirovanie prvej sekundy suboru
	if (SUCCEEDED (pSoundBuffer->Lock (0, 0, (LPVOID*)&pLockedPtr, &dwLockedSize, \
		NULL, NULL, DSBLOCK_ENTIREBUFFER)))
	{
		if (pStreamItem.ucType == SOUND_TYPE_WAV)				// wav
			memcpy (pLockedPtr, pStreamItem.pWaveSoundData, dwLockedSize);
		else if (pStreamItem.ucType == SOUND_TYPE_OGG)		// ogg
		{
			int intSection = 0;
			DWORD dwPos = 0;
			LONG lRead = 1;
			if (ov_raw_seek (pStreamItem.pOggFileObject, 0) == 0)
				while (dwPos < dwLockedSize)		// dekodovanie
				{
					lRead = ov_read (pStreamItem.pOggFileObject, (char*)(pLockedPtr + dwPos), \
						dwLockedSize - dwPos, 0, 2, 1, &intSection);
					if (lRead <= 0) break;	// chyba
					dwPos += lRead;
				}
		}
		else																	// neznamy typ suboru
			memset (pLockedPtr, (int)pStreamItem.intSilenceValue, dwLockedSize);

		pSoundBuffer->Unlock (pLockedPtr, dwLockedSize, NULL, NULL);
	}
}

//////////////////////////////////////////////////////////////////////////
// updatuje wav stream buffer
//////////////////////////////////////////////////////////////////////////
void UpdateStreamWaveBuffer (SOUND_ITEM &pStreamItem, DWORD dwBufferIdx, bool bUpdateLowHalf)
{
	BYTE							*pLockedPtr;
	LPDIRECTSOUNDBUFFER8		pSoundBuffer;
	DWORD							dwLockedSize, dwWriteSize = 0;
	bool							bWrapAround = false;

	if ((pStreamItem.pSoundBufferList == NULL) || (dwBufferIdx >= pStreamItem.ucSoundCount) || \
		(pStreamItem.pWaveSoundData == NULL) || (pStreamItem.bStream == false))
		return;

	SOUND_BUFFER &pStreamBuffer = pStreamItem.pSoundBufferList[dwBufferIdx];
	if ((pSoundBuffer = pStreamBuffer.pSoundBuffer) == NULL) return;

	if (pStreamBuffer.loopFlag == STREAMLOOP_STOPLOOP2)		// ukoncenie non-looping zvuku
	{InitFirstStreamSecond (pStreamItem, dwBufferIdx); return;}

	// obnovenie polovice bufru
	if (SUCCEEDED (pSoundBuffer->Lock (bUpdateLowHalf ? 0 : pStreamItem.dwUpdateBlockSize, \
		pStreamItem.dwUpdateBlockSize, (LPVOID*)&pLockedPtr, &dwLockedSize, NULL, NULL, 0)))
	{
		if (pStreamItem.dwFileRawDataSize - pStreamBuffer.dwCurrentFileOffset < pStreamItem.dwUpdateBlockSize)
		{
			// koniec suboru
			bWrapAround = true;
			dwWriteSize = pStreamItem.dwFileRawDataSize - pStreamBuffer.dwCurrentFileOffset;
		}
		else
			dwWriteSize = dwLockedSize;

		if (pStreamBuffer.loopFlag == STREAMLOOP_STOPLOOP1)		// zvysok vyplnit "tichom"
		{
			memset (pLockedPtr, (int)pStreamItem.intSilenceValue, dwLockedSize);
			pStreamBuffer.loopFlag = STREAMLOOP_STOPLOOP2;
		}
		else
		{
			BYTE *pReadPtr = pStreamItem.pWaveSoundData + pStreamBuffer.dwCurrentFileOffset;
			memcpy (pLockedPtr, pReadPtr, dwWriteSize);
		}

		if (bWrapAround)		// zapis znova od zaciatku suboru
		{
			BYTE *pWritePtr = pLockedPtr + dwWriteSize;
			dwWriteSize = pStreamItem.dwUpdateBlockSize - dwWriteSize;	// pocet bajtov, ktore este treba zapisat

			if (pStreamBuffer.loopFlag == STREAMLOOP_PLAYLOOP)		// ak sa prehrava v loop cykle, zacina sa od zaciatku suboru
			{
				memcpy (pWritePtr, pStreamItem.pWaveSoundData, dwWriteSize);
				pStreamBuffer.dwCurrentFileOffset = dwWriteSize;
			}
			else								// ak sa neprehrava loop, zvysok sa vyplni "tichom"
			{
				memset (pWritePtr, (int)pStreamItem.intSilenceValue, dwWriteSize);
				pStreamBuffer.loopFlag = STREAMLOOP_STOPLOOP1;
				pStreamBuffer.dwCurrentFileOffset = 0;
			}
		}
		else
			pStreamBuffer.dwCurrentFileOffset += dwWriteSize;

		pSoundBuffer->Unlock (pLockedPtr, dwLockedSize, NULL, NULL);
	}
}

//////////////////////////////////////////////////////////////////////////
// updatuje ogg stream buffer
//////////////////////////////////////////////////////////////////////////
void UpdateStreamOggBuffer (SOUND_ITEM &pStreamItem, DWORD dwBufferIdx, bool bUpdateLowHalf)
{
	BYTE						*pLockedPtr;
	int							intSection;
	LONG						lRead;
	LPDIRECTSOUNDBUFFER8		pSoundBuffer;
	DWORD						dwPos, dwLockedSize, dwWriteSize = 0;
	bool						bWrapAround = false;

	if ((pStreamItem.pSoundBufferList == NULL) || (dwBufferIdx >= pStreamItem.ucSoundCount) || 
		(pStreamItem.pOggFileObject == NULL) || (pStreamItem.bStream == false))
		return;

	SOUND_BUFFER &pStreamBuffer = pStreamItem.pSoundBufferList[dwBufferIdx];
	if ((pSoundBuffer = pStreamBuffer.pSoundBuffer) == NULL) return;

	if (pStreamBuffer.loopFlag == STREAMLOOP_STOPLOOP2)		// ukoncenie non-looping zvuku
	{InitFirstStreamSecond (pStreamItem, dwBufferIdx); return;}

	// obnovenie polovice bufru
	if (FAILED (pSoundBuffer->Lock (bUpdateLowHalf ? 0 : pStreamItem.dwUpdateBlockSize, \
		pStreamItem.dwUpdateBlockSize, (LPVOID*)&pLockedPtr, &dwLockedSize, \
		NULL, NULL, 0)))
		return;

	if (pStreamBuffer.loopFlag == STREAMLOOP_STOPLOOP1)		// vyplnuje sa "tichom"
	{
		memset (pLockedPtr, (int)pStreamItem.intSilenceValue, dwLockedSize);
		pStreamBuffer.loopFlag = STREAMLOOP_STOPLOOP2;
	}
	else
	{
		intSection = 0; dwPos = 0;
		// ogg vorbis je vzdy 16 bitovy
		DWORD dwDesPos = pStreamBuffer.dwCurrentFileOffset >> (pStreamBuffer.bStereo ? 2 : 1);	// delenie 2 pre mono, 4 pre stereo

		if (ov_pcm_tell (pStreamItem.pOggFileObject) != dwDesPos)
			ov_pcm_seek (pStreamItem.pOggFileObject, ogg_int64_t(dwDesPos));
		while (dwPos < dwLockedSize)		// dekodovanie
		{
			lRead = ov_read (pStreamItem.pOggFileObject, (char*)(pLockedPtr + dwPos), \
				dwLockedSize - dwPos, 0, 2, 1, &intSection);
			if (lRead < 0)				// chyba
				break;
			else if (lRead == 0)		// koniec suboru
			{bWrapAround = true;  dwWriteSize = dwPos; break;}
			dwPos += lRead;
		}
		pStreamBuffer.dwCurrentFileOffset += dwPos;
	}

	if (bWrapAround)		// zapis znova od zaciatku suboru
	{
		BYTE *pWritePtr = pLockedPtr + dwWriteSize;
		dwWriteSize = pStreamItem.dwUpdateBlockSize - dwWriteSize;	// pocet bajtov, ktore este treba zapisat
		pStreamBuffer.dwCurrentFileOffset = dwWriteSize;

		if (pStreamBuffer.loopFlag == STREAMLOOP_PLAYLOOP)		// ak sa prehrava v loop cykle, zacina sa od zaciatku suboru
		{
			intSection = 0; dwPos = 0;
			if (ov_raw_seek (pStreamItem.pOggFileObject, 0) == 0)
				while (dwPos < dwWriteSize)		// dekodovanie
				{
					lRead = ov_read (pStreamItem.pOggFileObject, (char*)(pWritePtr + dwPos), \
						dwWriteSize - dwPos, 0, 2, 1, &intSection);
					if (lRead <= 0) break;		// chyba
					dwPos += lRead;
				}
		}
		else								// ak sa neprehrava loop, zvysok sa vyplni "tichom"
		{
			memset (pWritePtr, (int)pStreamItem.intSilenceValue, dwWriteSize);
			pStreamBuffer.loopFlag = STREAMLOOP_STOPLOOP1;
		}
	}

	pSoundBuffer->Unlock (pLockedPtr, dwLockedSize, NULL, NULL);
}
