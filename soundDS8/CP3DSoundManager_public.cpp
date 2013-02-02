
//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
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


bool		g_bLastConVarValue;

//////////////////////////////////////////////////////////////////////////
//										CP3DSoundManager
//////////////////////////////////////////////////////////////////////////

REGISTER_SINGLE_CLASS(CP3DSoundManager, IP3DSOUND_SOUNDMANAGER); // zaregistruj tuto tøídu

CP3DSoundManager	*g_pSoundMan;

//---------------------------------
CP3DSoundManager::CP3DSoundManager()
{
	CoInitialize(NULL);	// potrebne pre pracu s efektmi
	m_pDS = NULL;
	m_p3DListener = NULL;
	m_ListenerPosition = P3DXVector3D(0,0,0);
	m_bInitialized = false;
	m_bInCachingState = false;
	memset (m_MasterEffects, 0, sizeof(m_MasterEffects));
	m_dwMasterEffectsNum = 0;
	m_lMasterVolume = DSBVOLUME_MAX;
	m_NotifyCallbackFunc = NULL;
	m_dwHashTableItemsCount = 0;
	m_pXmlSoundDefs = NULL;

	g_pSoundMan = this;
}

//---------------------------------
CP3DSoundManager::~CP3DSoundManager()
{
	UnloadAll ();
	SAFE_RELEASE (m_p3DListener)
	SAFE_RELEASE (m_pDS)
	CoUninitialize();
	if (m_pXmlSoundDefs && g_pXML)
		g_pXML->Free (m_pXmlSoundDefs);

	g_pSoundMan = NULL;
}

//---------------------------------
bool CP3DSoundManager::Init ()
{
	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);
	if (g_pFS == NULL) return false;
	g_pXML = (IP3DXML*)I_GetClass(IP3DENGINE_XML);
	if (g_pXML == NULL) return false;

	g_bLastConVarValue = CVs_soundfx.GetBool ();
	CVs_soundfx.SetChangeCallback (SoundFX_Change_Callback, false);

	return true;
}

//---------------------------------
bool CP3DSoundManager::Initialize (HWND hMainWnd, SoundEventNotifyCallback notifyCallbackFunc)
{
	I_RegisterModule("SoundDS8");
	// memory leaks detection
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(76);

	LPDIRECTSOUNDBUFFER		pPrimaryBuffer;  // Cannot be IDirectSoundBuffer8.
	DSBUFFERDESC				buffer_desc = {0};

	// inicializacia konzoly
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	g_pConsole->RegisterLastConVar(g_pLastConVar); // zaregistrovat ConVary tohoto dll projektu - NUTNÉ!

#ifdef _DEBUG
	CON(MSG_CON_INFO, "= DirectSound 8 Module (%s, %s, DEBUG) initialization =", __DATE__, __TIME__);
#else
	CON(MSG_CON_INFO, "= DirectSound 8 Module (%s, %s) initialization =", __DATE__, __TIME__);
#endif

	if ((m_pDS != NULL) || (hMainWnd == NULL))
	{
		CON(MSG_CON_ERR, "Sound: DirectSound object = NULL!", __DATE__, __TIME__);
		return false;
	}

	// vytvorenie top level objektu DirectSound
	if (FAILED (DirectSoundCreate8 (NULL, &m_pDS, NULL)))
		goto err_quit;
	// nastavenie koop. levelu
	if (FAILED (m_pDS->SetCooperativeLevel (hMainWnd, DSSCL_PRIORITY)))
		goto err_quit;

	// inicializacia 3D listenera
	buffer_desc.dwSize = sizeof(DSBUFFERDESC);
	buffer_desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

	if (SUCCEEDED (m_pDS->CreateSoundBuffer (&buffer_desc, &pPrimaryBuffer, NULL)))
	{
		pPrimaryBuffer->QueryInterface (IID_IDirectSound3DListener8, (LPVOID*)&m_p3DListener);
		pPrimaryBuffer->Release();
	}
	// distance factor
	if (m_p3DListener == NULL)
		goto err_quit;
	m_p3DListener->SetDistanceFactor (ONE_METER_DIV, DS3D_IMMEDIATE);
	m_p3DListener->SetRolloffFactor (DEF_ROLLOFF_FACTOR, DS3D_IMMEDIATE);

	// parsing xml suboru s popisom virtualnych zvukov
	m_pXmlSoundDefs = g_pXML->Parse_file ("sound_def.xml");
	if (m_pXmlSoundDefs == NULL)
	{
		CON(MSG_CON_ERR, "Sound: sound_def.xml file have incorrect format or doesn't exist");
		goto err_quit;
	}

	m_NotifyCallbackFunc = notifyCallbackFunc;
	m_bInitialized = true;

	CON(MSG_CON_INFO, "Sound: Sound subsystem successfuly initialized...", __DATE__, __TIME__);

	return true;
err_quit:
	SAFE_RELEASE (m_p3DListener)
	SAFE_RELEASE (m_pDS)
	return false;
}

// PrimaryChannels - (1)mono, (2)stereo, ine hodnoty sa neodporucaju
// BitsPerSample - 8 bit, 16 bit - pocet bitov na vzorku, ine hodnoty sa neodporucaju
// SamplesPerSec - ..., 8000 kH, 11025 kH, 22050 kH, 32000 kH, 44100 kH, ... - pocet vzoriek za sekundu
//---------------------------------
bool CP3DSoundManager::SetPrimaryBufferFormat (SOUND_CHANNELS PrimaryChannels, SOUND_BITS_PER_SAMPLE BitsPerSample, SOUND_SAMPLES_PER_SECOND SamplesPerSec)
{
	LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

	if (m_pDS == NULL) return false;
	// Get the primary buffer 
	DSBUFFERDESC dsbd = {0};
	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
 
	if (FAILED (m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL)))
	  return false;

	WAVEFORMATEX wfx = {0};
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.nChannels       = PrimaryChannels;
	wfx.nSamplesPerSec  = SamplesPerSec;
	wfx.wBitsPerSample  = BitsPerSample;
	wfx.nBlockAlign     = (WORD) ((wfx.wBitsPerSample / 8) * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	bool bRet = SUCCEEDED (pDSBPrimary->SetFormat(&wfx));
	SAFE_RELEASE (pDSBPrimary);
	return bRet;
}

//---------------------------------
bool CP3DSoundManager::UnloadSound (const char *strSoundName)
{
	DWORD dwHashTableIdx;
	if (!FindSoundByName (strSoundName, dwHashTableIdx))
		return false;
	return UnloadSound (dwHashTableIdx);
}

//---------------------------------
bool CP3DSoundManager::UnloadSound (unsigned int dwSoundID)
{
	DWORD dwHashTableIdx;
	if (!FindSoundByID (dwSoundID, dwHashTableIdx))
		return false;
	return UnloadSound (dwHashTableIdx);
}

//---------------------------------
void CP3DSoundManager::UnloadAll ()
{
	DWORD i;
	for (i=0; i<ACTIVE_LIST_SIZE; i++)
		RemoveFromActiveList (i);

	for (i=0; i<HASH_TABLE_SIZE; i++)
		m_SoundHashTable[i].Unload ();
	m_dwHashTableItemsCount = 0;
}

//---------------------------------
bool CP3DSoundManager::SoundName2SoundID (IN const char *strSoundName, OUT unsigned int &dwSoudID)
{
	DWORD	dwHashTableIdx;
	if (FindSoundByName (strSoundName, dwHashTableIdx))
		{dwSoudID = m_SoundHashTable[dwHashTableIdx].dwSoundID; return true;}
	return false;
}

//---------------------------------
bool CP3DSoundManager::SoundID2SoundName (IN unsigned int dwSoudID, OUT const char *strSoundName)
{
	DWORD	dwHashTableIdx;
	if (FindSoundByID (dwSoudID, dwHashTableIdx))
		{strSoundName = m_SoundHashTable[dwHashTableIdx].strSoundName; return true;}
	return false;
}

// fVolume - hodnota od 0.0f do 1.0f
//---------------------------------
inline LONG	Float2Decibels (float fVolume)
{
	LONG lVolume;
	// uprava linearnej hodnoty na decibely
	if (fVolume <= 0.0f)
		lVolume = DSBVOLUME_MIN;
	else if (fVolume >= 1.0f)
		lVolume = DSBVOLUME_MAX;
	else
	{
		lVolume = LONG(2000.0f * log10f(fVolume));
		if (lVolume < DSBVOLUME_MIN) lVolume = DSBVOLUME_MIN;
	}
	return lVolume;
}

// ma vplyv iba na bufre vytvorene s flagom P3DBCAPS_CTRLVOLUME
// je mozne nastavit iba utlm zvuku, nie zosilnenie
// fVolume - hodnota od 0.0f do 1.0f
//---------------------------------
bool CP3DSoundManager::SetSoundVolume (P3D_SOUND_HANDLE soundHandle, float fVolume)
{
	int intActiveListIdx;
	LONG lVolume;
	SOUND_BUFFER *pSoundBufferPointer;

	if ((intActiveListIdx = FindSoundByHandleInActiveList (soundHandle)) < 0)
		return false;

	lVolume = Float2Decibels (fVolume);

	SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[intActiveListIdx].intHashTableIdx];
	if (pSoundItem.dwCreationFlags & P3DBCAPS_CTRLVOLUME)
	{
		if (m_ActiveList[intActiveListIdx].bDuplicatedBuffer)
			pSoundBufferPointer = &m_ActiveList[intActiveListIdx].DuplicatedBuffer;
		else
			pSoundBufferPointer = &pSoundItem.pSoundBufferList[m_ActiveList[intActiveListIdx].wSoundBufferIdx];

		SOUND_BUFFER &pSndBuffer = *pSoundBufferPointer;
		if (pSndBuffer.pSoundBuffer)
			if (SUCCEEDED (pSndBuffer.pSoundBuffer->SetVolume (lVolume)))
			{
				if (lVolume == DSBVOLUME_MAX) pSndBuffer.bDefVolumeSet = true;
				return true;
			}
	}
	return false;
}

// ma vplyv iba na bufre vytvorene s flagom P3DBCAPS_CTRLVOLUME
// je mozne nastavit iba utlm zvuku, nie zosilnenie
// fVolume - hodnota od 0.0f do 1.0f
//---------------------------------
void CP3DSoundManager::SetMasterVolume (float fVolume)
{
	SOUND_BUFFER *pSoundBufferPointer;

	m_lMasterVolume = Float2Decibels (fVolume);

	// nastavit zmenu hlasitosti na uz prehravane zvuky
	for (DWORD i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].intHashTableIdx >= 0)
		{
			SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[i].intHashTableIdx];
			if (pSoundItem.dwCreationFlags & P3DBCAPS_CTRLVOLUME)
			{
				if (m_ActiveList[i].bDuplicatedBuffer)
					pSoundBufferPointer = &m_ActiveList[i].DuplicatedBuffer;
				else
					pSoundBufferPointer = &pSoundItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx];

				SOUND_BUFFER &pSndBuffer = *pSoundBufferPointer;
				if (pSndBuffer.pSoundBuffer)
					if (SUCCEEDED (pSndBuffer.pSoundBuffer->SetVolume (m_lMasterVolume)))
						if (m_lMasterVolume == DSBVOLUME_MAX)
							pSndBuffer.bDefVolumeSet = true;
			}
		}
}

// nastavi efekt(y) na prehravany zvuk, tieto efekty sa "pridaju" k uz nastavenym master efektom
// efekty pre zvuk je najlepsie nastavovat hned po volani funkcie PlaySound()
// ma vplyv iba na bufre vytvorene s flagom P3DBCAPS_CTRLFX
// bufre vlastnosti v pEffects musia mat spravny format podla druhu efektu, pozri P3DFX_* v IP3DSoundEngine.h
// ak dwEffectsNum == 0 tak sa zrusia vsetky efekty nastavene fciou SetSoundFXs(), master efekty ale zostavaju
//---------------------------------
bool CP3DSoundManager::SetSoundFXs (P3D_SOUND_HANDLE soundHandle, DWORD dwEffectsNum, P3D_SFX_DESC *pEffects)
{
	int intIdx;
	if ((intIdx = FindSoundByHandleInActiveList(soundHandle)) < 0)
		return false;

	if ((dwEffectsNum + m_dwMasterEffectsNum) > P3D_SOUNDFX_COUNT) return false;

	if ((dwEffectsNum > 0) && (pEffects == NULL))
		return false;
	if ((pEffects != NULL) && (pEffects[0].effectID == SFX_ID_NONE))
		{dwEffectsNum = 0; pEffects = NULL;}

	if (m_ActiveList[intIdx].intHashTableIdx >= 0)
	{
		SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[intIdx].intHashTableIdx];
		if ((pSoundItem.dwCreationFlags & P3DBCAPS_CTRLFX) == 0)
			return false;
		if (pSoundItem.pSoundBufferList && (m_ActiveList[intIdx].wSoundBufferIdx < pSoundItem.ucSoundCount))
		{
			SOUND_BUFFER &pSndBuffer = pSoundItem.pSoundBufferList[m_ActiveList[intIdx].wSoundBufferIdx];
			if ((dwEffectsNum == 0) && pSndBuffer.bNoneEffectSet)
				return true;
			bool bRet = SetEffectOnBuffer (pSndBuffer.pSoundBuffer, false, dwEffectsNum, pEffects);
			pSndBuffer.bNoneEffectSet = ((dwEffectsNum + m_dwMasterEffectsNum) == 0) ? true : false;
			return bRet;
		}
	}

	// chyba
	RemoveFromActiveList (intIdx);
	return false;
}

// ma vplyv iba na bufre vytvorene s flagom P3DBCAPS_CTRLFX
// bufre vlastnosti v pEffects musia mat spravny format podla druhu efektu, pozri P3DFX_* v IP3DSoundEngine.h
// odstrani efekty nastavene jednotlivo cez funkciu SetSoundFXs() !
// ak dwEffectsNum == 0 tak sa vypnu vsetky efekty
//---------------------------------
bool CP3DSoundManager::SetMasterFXs (DWORD dwEffectsNum, P3D_SFX_DESC *pEffects)
{
	if (dwEffectsNum > P3D_SOUNDFX_COUNT) return false;

	if ((dwEffectsNum > 0) && (pEffects == NULL))
		return false;
	if ((pEffects != NULL) && (pEffects[0].effectID == SFX_ID_NONE))
			{dwEffectsNum = 0; pEffects = NULL;}

	if ((dwEffectsNum == 0) && (m_dwMasterEffectsNum == 0))		// ziadna zmena
		return true;

	m_dwMasterEffectsNum = dwEffectsNum;
	if (dwEffectsNum)
		memcpy (m_MasterEffects, pEffects, sizeof(P3D_SFX_DESC) * dwEffectsNum);

	// uplatnit zmenu efektu na uz prehravane zvuky
	for (DWORD i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].intHashTableIdx >= 0)
		{
			SOUND_ITEM	&pSoudItem = m_SoundHashTable[m_ActiveList[i].intHashTableIdx];
			if ((pSoudItem.dwCreationFlags & P3DBCAPS_CTRLFX) && (pSoudItem.pSoundBufferList != NULL))
			{
				SOUND_BUFFER &pSndBuffer = pSoudItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx];
				if ((m_dwMasterEffectsNum != 0) || (!pSndBuffer.bNoneEffectSet))
				{
					SetEffectOnBuffer (pSndBuffer.pSoundBuffer);
					pSndBuffer.bNoneEffectSet = (m_dwMasterEffectsNum == 0) ? true : false;
				}
			}
		}
	return true;
}

// "uplatni" vsetky zmeny v nastaveni 3D bufrov a Listenera, tu istu funkciu vola Update()
// moze byt casovo narocna, volat iba ak je potrebne uplatnit 3D nastavenia okamzite
//---------------------------------
bool CP3DSoundManager::Apply3DChanges ()
{
	return SUCCEEDED (m_p3DListener->CommitDeferredSettings());
}

// spravi update active listu, obnovuje stream bufre, uplatni vsetky "deferred" zmeny, spusta zvuky
// fciu je najlepsie volat jeden krat na kazdy frame
//---------------------------------
void CP3DSoundManager::Update ()
{
	DWORD	dwStatus, i;

	// update active listu
	for (i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].intHashTableIdx >= 0)
		{
			SOUND_BUFFER *pSoundBufferPointer = NULL;
			SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[i].intHashTableIdx];

			if (m_ActiveList[i].bDuplicatedBuffer)
				pSoundBufferPointer = &m_ActiveList[i].DuplicatedBuffer;
			else
			{
				if ((m_ActiveList[i].wSoundBufferIdx >= pSoundItem.ucSoundCount) || \
													(pSoundItem.pSoundBufferList == NULL))
					{RemoveFromActiveList (i); continue;}		// chyba
				pSoundBufferPointer = &pSoundItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx];
			}


			SOUND_BUFFER &pSoundBufferItem = *pSoundBufferPointer;

			if (pSoundBufferItem.pSoundBuffer == NULL)
				{RemoveFromActiveList (i); continue;}		// chyba

			// ak bol zvuk prehrany a skoncil, tak odstranenie z active-listu
			if ((m_ActiveList[i].wFlags & CLEARANCE_CANDIDATE) && \
					(m_ActiveList[i].wFlags & MUTED_AT_MAX_DIST) == 0)
				if (SUCCEEDED (pSoundBufferItem.pSoundBuffer->GetStatus (&dwStatus)))
					if ((dwStatus & DSBSTATUS_PLAYING) == 0)
					{
						// spustit notifikaciu o ukonceni zvuku
						if (m_ActiveList[i].pOwner && m_NotifyCallbackFunc)
							m_ActiveList[i].pOwner->FireNotifyCallback (m_ActiveList[i].dwSoundHandle);
						// TODO: v notify callback funkcii sa mozu spustat dalsie zvuky...
						// ... nejak tomu prisposobit kod

						RemoveFromActiveList (i);
						continue;
					}

			// pozastavenie/spustenie 3D zvukov za/pred max. dosahom
			if (pSoundItem.bMuteAtMaxDist && (pSoundBufferItem.pSound3DBuffer != NULL))
			{
				P3DXVector3D pointSoundPos;
				float fMaxDist = DEF_SOUND_MAX_DISTANCE;
				P3DXVector3D vecDist (m_ListenerPosition);
				pSoundBufferItem.pSound3DBuffer->GetPosition ((D3DVECTOR*)&pointSoundPos.x);
				pSoundBufferItem.pSound3DBuffer->GetMaxDistance (&fMaxDist);
				vecDist -= pointSoundPos;
				if (fMaxDist < vecDist.Length())		// prekroceny rozsah
				{
					if ((m_ActiveList[i].wFlags & MUTED_AT_MAX_DIST) == 0)
					{
						m_ActiveList[i].wFlags |= MUTED_AT_MAX_DIST;
						pSoundBufferItem.pSoundBuffer->Stop ();
					}
				}
				else if (m_ActiveList[i].wFlags & MUTED_AT_MAX_DIST)	// v platnom rozsahu
				{
					m_ActiveList[i].wFlags &= ~MUTED_AT_MAX_DIST;
					pSoundBufferItem.pSoundBuffer->Play (0, 0, (m_ActiveList[i].wFlags & PLAY_IN_LOOP) ? DSBPLAY_LOOPING : 0);
				}
			}

			// update 3D pozicie, pripadne rychlosti
			if (pSoundItem.dwCreationFlags & P3DBCAPS_CTRL3D)
				if (m_ActiveList[i].pOwner && pSoundBufferItem.pSound3DBuffer)
				{
					P3DXVector3D	&vec3DPos = m_ActiveList[i].pOwner->m_3DPos;
					pSoundBufferItem.pSound3DBuffer->SetPosition (vec3DPos.x, vec3DPos.y, vec3DPos.z, DS3D_DEFERRED);
					if (m_ActiveList[i].pOwner->m_bUseVelocity)
					{
						P3DXVector3D &vec3DVel = m_ActiveList[i].pOwner->m_3DVelocity;
						pSoundBufferItem.pSound3DBuffer->SetVelocity (vec3DVel.x, vec3DVel.y, vec3DVel.z, DS3D_DEFERRED);
					}
				}

			// update stream bufrov, ak je to potrebne
			if (pSoundItem.bStream)
			{
				if (WaitForSingleObject (pSoundBufferItem.hUpdateEvents[0], 0) == WAIT_OBJECT_0)
					if (pSoundItem.ucType == SOUND_TYPE_WAV)
						UpdateStreamWaveBuffer (pSoundItem, m_ActiveList[i].wSoundBufferIdx, true);
					else if (pSoundItem.ucType == SOUND_TYPE_OGG)
						UpdateStreamOggBuffer (pSoundItem, m_ActiveList[i].wSoundBufferIdx, true);

				if (WaitForSingleObject (pSoundBufferItem.hUpdateEvents[1], 0) == WAIT_OBJECT_0)
					if (pSoundItem.ucType == SOUND_TYPE_WAV)
						UpdateStreamWaveBuffer (pSoundItem, m_ActiveList[i].wSoundBufferIdx, false);
					else if (pSoundItem.ucType == SOUND_TYPE_OGG)
						UpdateStreamOggBuffer (pSoundItem, m_ActiveList[i].wSoundBufferIdx, false);
			}
		}

	// "uplatni" vsetky 3D zmeny
	m_p3DListener->CommitDeferredSettings ();

	// spusti pozadovane 3D zvuky
	for (i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].wFlags & PLAY_ON_UPDATE)
		{
			if (m_ActiveList[i].intHashTableIdx >= 0)
			{
				LPDIRECTSOUNDBUFFER8	pSoundBuffer8 = NULL;
				SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[i].intHashTableIdx];

				if (m_ActiveList[i].bDuplicatedBuffer)
					pSoundBuffer8 = m_ActiveList[i].DuplicatedBuffer.pSoundBuffer;
				else if (pSoundItem.pSoundBufferList)
					pSoundBuffer8 = pSoundItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx].pSoundBuffer;

				if (pSoundBuffer8 == NULL)
					{RemoveFromActiveList (i); continue;}		// chyba
				if (pSoundItem.bStream)
				{
					if (m_ActiveList[i].wFlags & PLAY_IN_LOOP)
						pSoundItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx].loopFlag = STREAMLOOP_PLAYLOOP;
					if (FAILED (pSoundBuffer8->Play (0, 0, DSBPLAY_LOOPING)))	// stream bufre musia mat vzdy loop flag
						{RemoveFromActiveList (i); continue;}
				}
				else if (FAILED (pSoundBuffer8->Play (0, 0, (m_ActiveList[i].wFlags & PLAY_IN_LOOP) ? DSBPLAY_LOOPING : 0)))
					{RemoveFromActiveList (i); continue;}
			}
			m_ActiveList[i].wFlags |= CLEARANCE_CANDIDATE;
			m_ActiveList[i].wFlags &= ~PLAY_ON_UPDATE;
		}
}


//########################################################################
//										Play functions
//########################################################################

// funkcie PlaySound() vratia pseudo_handle, ten sa da pouzit na manipulaciu so spustenym zvukom
// handle je platny len pocas doby prehravania zvuku
// hned po volani PlaySound() je vhodne nastavit pre zvuk efekty (ak je to potrebne)
// zvuk sa nezacne prehravat okamzite, ale spusti sa az pri nasledujucom volani Update()
// v pripade chyby PlaySound() vrati 0

// tymito dvoma funkciami sa spusta zvuk, ktory nie je 3D, moze mat stereo format
// napr. dialogy, hudba a pod.
//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySound (const char *strSoundName, bool bLoop)
{
	return PlayFreeSample (strSoundName, NULL, NULL, bLoop);
}
//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySound (unsigned int dwSoundID, bool bLoop)
{
	return PlayFreeSample (dwSoundID, NULL, NULL, bLoop);
}

// nasledujuce styri funkcie PlaySound() spustaju iba 3D zvuky, zvuk musi mat mono format
// je potrebne definovat bud kontajner vlastnika (pOwner), alebo priamo zadat poziciu zvuku (pSoundPos)
//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySound (const char *strSoundName, P3DXVector3D &pSoundPos, bool bLoop)
{
	return PlayFreeSample (strSoundName, NULL, &pSoundPos, bLoop);
}
//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySound (unsigned int dwSoundID, P3DXVector3D &pSoundPos, bool bLoop)
{
	return PlayFreeSample (dwSoundID, NULL, &pSoundPos, bLoop);
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySound (const char *strSoundName, IP3DSoundContainer *pOwner, bool bLoop)
{
	int intIdx;

	// ak sa pozadovany zvuk prave nachadza v active liste
	if (pOwner == NULL) return 0;
	if ((intIdx = FindSoundByNameInActiveList (strSoundName, pOwner)) >= 0)
	{
		P3D_SOUND_HANDLE handleRet = PlaySoundInternal ((DWORD)intIdx, pOwner, bLoop);
		if (handleRet) return handleRet;
	}

	return PlayFreeSample (strSoundName, pOwner, NULL, bLoop);
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundManager::PlaySound (unsigned int dwSoundID, IP3DSoundContainer *pOwner, bool bLoop)
{
	int intIdx;

	// ak sa pozadovany zvuk prave nachadza v active liste
	if (pOwner == NULL) return 0;
	if ((intIdx = FindSoundByIDInActiveList (dwSoundID, pOwner)) >= 0)
	{
		P3D_SOUND_HANDLE handleRet = PlaySoundInternal ((DWORD)intIdx, pOwner, bLoop);
		if (handleRet) return handleRet;
	}

	return PlayFreeSample (dwSoundID, pOwner, NULL, bLoop);
}

//---------------------------------
bool CP3DSoundManager::StopSound (P3D_SOUND_HANDLE dwSoundHandle)
{
	int intIdx;
	if ((dwSoundHandle == 0) || ((intIdx = FindSoundByHandleInActiveList (dwSoundHandle)) < 0))
		return false;
	RemoveFromActiveList (intIdx);
	return true;
}

// ak bPauseOnly == true tak sa zvuky len pozastavia a je mozne ich znovu spustit
// funkciou ResumeAll(), alebo PlaySound()
//---------------------------------
void CP3DSoundManager::StopAll (bool bPauseOnly)
{
	DWORD i;
	if (bPauseOnly)
	{
		for (i=0; i<ACTIVE_LIST_SIZE; i++)
			if (m_ActiveList[i].intHashTableIdx >= 0)
			{
				SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[i].intHashTableIdx];
				if (pSoundItem.pSoundBufferList && (m_ActiveList[i].wSoundBufferIdx < pSoundItem.ucSoundCount))
				{
					LPDIRECTSOUNDBUFFER8	pSoundBuffer8;
					pSoundBuffer8 = pSoundItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx].pSoundBuffer;
					if (pSoundBuffer8 == NULL)
						{RemoveFromActiveList (i); continue;}
					if (SUCCEEDED (pSoundBuffer8->Stop ()))
					{
						m_ActiveList[i].wFlags |= STOP_ALL_FLAG;
						m_ActiveList[i].wFlags &= ~CLEARANCE_CANDIDATE;
					}
				}
			}
	}
	else for (i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].dwSoundID)
			RemoveFromActiveList (i);
}

// spusti vsetky zvuky pozastavene fciou StopAll(true);
//---------------------------------
void CP3DSoundManager::ResumeAll ()
{
	for (DWORD i=0; i<ACTIVE_LIST_SIZE; i++)
		if ((m_ActiveList[i].intHashTableIdx >= 0) && (m_ActiveList[i].wFlags & STOP_ALL_FLAG))
		{
			SOUND_ITEM &pSoundItem = m_SoundHashTable[m_ActiveList[i].intHashTableIdx];
			if ((pSoundItem.pSoundBufferList) && (m_ActiveList[i].wSoundBufferIdx < pSoundItem.ucSoundCount))
			{
				LPDIRECTSOUNDBUFFER8	pSoundBuffer8;
				pSoundBuffer8 = pSoundItem.pSoundBufferList[m_ActiveList[i].wSoundBufferIdx].pSoundBuffer;
				if (pSoundBuffer8 == NULL)
					{RemoveFromActiveList (i); continue;}
				if (SUCCEEDED(pSoundBuffer8->Play (0, 0, (m_ActiveList[i].wFlags & PLAY_IN_LOOP) ? DSBPLAY_LOOPING : 0)))
				{
					m_ActiveList[i].wFlags &= ~STOP_ALL_FLAG;
					m_ActiveList[i].wFlags |= CLEARANCE_CANDIDATE;
				}
			}
		}
}

// pri destrukcii vlastnika je treba uvolnit z active-listu
// vsetky zvuky, ktore patria kontajneru vlastnika
//---------------------------------
void CP3DSoundManager::ContainerDestroyAlert (IP3DSoundContainer *pOwner)
{
	for (register DWORD i=0; i<ACTIVE_LIST_SIZE; i++)
		if (m_ActiveList[i].pOwner == pOwner)
			RemoveFromActiveList (i);
}

//########################################################################
//										Listener functions
//########################################################################

//---------------------------------
bool CP3DSoundManager::Listener_SetAllParameters (P3DLISTENER	&pListenerParams)
{
	pListenerParams.dwSize = sizeof(P3DLISTENER);
	if (SUCCEEDED (m_p3DListener->SetAllParameters ((DS3DLISTENER*)&pListenerParams, DS3D_DEFERRED)))
		{m_ListenerPosition = pListenerParams.vPosition; return true;}
	return false;
}

//---------------------------------
bool CP3DSoundManager::Listener_GetAllParameters (P3DLISTENER	&pListenerParams)
{
	pListenerParams.dwSize = sizeof(P3DLISTENER);
	return SUCCEEDED (m_p3DListener->GetAllParameters ((DS3DLISTENER*)&pListenerParams));
}

// vecNewOrientFront - smerovy vektor pohladu kamery
//---------------------------------
bool CP3DSoundManager::Listener_SetPositionOrientation (P3DXVector3D &pointNewPosition, P3DXVector3D &vecNewOrientFront)
{
	m_p3DListener->SetPosition (pointNewPosition.x, pointNewPosition.y, pointNewPosition.z, DS3D_DEFERRED);
	m_ListenerPosition = pointNewPosition;
	m_p3DListener->SetOrientation (vecNewOrientFront.x, vecNewOrientFront.y, vecNewOrientFront.z, 0.0f, 1.0f, 0.0f, DS3D_DEFERRED);
	return true;
}

// Nastavenie rychlosti, pouziva sa iba pri dopplerovom efekte.
// Pri malej rychlosti (chodza, beh) netreba nastavovat
//---------------------------------
bool CP3DSoundManager::Listener_SetVelocity (P3DXVector3D &vecNewVelocity)
{
	return SUCCEEDED (m_p3DListener->SetVelocity (vecNewVelocity.x, vecNewVelocity.y, vecNewVelocity.z, DS3D_DEFERRED));
}

// nastavi nasobitel pre Doppler effect, plati globalne pre vsetky 3D zvuky
// fNewDopplerFactor - hodnota od 0 do 10, defaultne 1
//---------------------------------
bool CP3DSoundManager::Listener_SetDopplerFactor (float fNewDopplerFactor)
{
	return SUCCEEDED (m_p3DListener->SetDopplerFactor (fNewDopplerFactor, DS3D_DEFERRED));
}

// Rolloff faktor udava mnozstvo utlmu zvuku v zavislosti na vzdialenosti od listenera
// plati globalne pre vsetky 3D zvuky
// fNewRolloffFactor - hodnota od 0 do 10, defaultne 1
//---------------------------------
bool CP3DSoundManager::Listener_SetRolloffFactor (float fNewRolloffFactor)
{
	return SUCCEEDED (m_p3DListener->SetRolloffFactor (fNewRolloffFactor, DS3D_DEFERRED));
}