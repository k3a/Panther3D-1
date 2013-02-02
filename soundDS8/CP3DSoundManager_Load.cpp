//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	GetWaveFileInfo() & LoadSound() function definition
//-----------------------------------------------------------------------------


#include "CP3DSoundManager.h"

#pragma warning (disable : 4311)


// parsing wav suboru
//---------------------------------
bool GetWaveFileInfo (const BYTE *hFileMapView, DWORD dwFileSize, WAVE_SOUNDFILE_DESC &SounfFileDesc)
{
	DWORD	dwChunkSize;
	bool	bFMTdone, bDATAdone;
	WAVE_CHUNK		*pChunk, *pMaxChunkPointer;

	if (hFileMapView == NULL) return false;

	// test wav formatu
	pChunk = (WAVE_CHUNK*)hFileMapView;
	if (pChunk->ck_ID != 0x46464952)					// 'RIFF'
		return false;
	if ((DWORD)(pChunk->ck_Data) != 0x45564157)	// 'WAVE'
		return false;

	// parsing wav suboru - je potrebne vyhladat chunk 'fmt ' a 'data'
	bFMTdone = false;
	bDATAdone = false;

	pMaxChunkPointer = (WAVE_CHUNK*)(hFileMapView + dwFileSize - 24);
	pChunk = (WAVE_CHUNK*)(hFileMapView + 12);

	while ((!bFMTdone || !bDATAdone) && (pChunk < pMaxChunkPointer))
	{
		if (pChunk->ck_ID == 0x20746d66)		// 'fmt '
		{
			// nacitanie formatu suboru
			memcpy (&SounfFileDesc.wfex, &pChunk->ck_Data, sizeof(PCMWAVEFORMAT));
			if (SounfFileDesc.wfex.wFormatTag != WAVE_FORMAT_PCM)		// trieda podporuje iba PCM wave format
				return false;
			SounfFileDesc.wfex.cbSize = 0;
			bFMTdone = true;
		}
		else if (pChunk->ck_ID == 0x61746164)		// 'data'
		{
			// nastavenie smernika na zvukove data
			SounfFileDesc.pSoundData = (BYTE*)&pChunk->ck_Data;
			SounfFileDesc.dwSoundDataSize = pChunk->ck_Size;
			bDATAdone = true;
		}
		// pokracovat dalsim chunk-om
		dwChunkSize = pChunk->ck_Size;
		if (dwChunkSize & 0x01) dwChunkSize++;		// zarovnanie na nasobok 2
		pChunk = (WAVE_CHUNK*) ((BYTE*)(pChunk) + 8 + dwChunkSize);
	}

	if (!bFMTdone || !bDATAdone)
		return false;			// neboli najdene potrebne chunk-y 'fmt ' a 'data'

	return true;
}

// TODO: dopracovat caching !!!
//---------------------------------
void CP3DSoundManager::StartCaching ()
{
	m_bInCachingState = true;
}

//---------------------------------
void CP3DSoundManager::StopCaching ()
{
	m_bInCachingState = false;
}

// strSoundName - nazov zvuku, case-sensitive
// dwMaxSim - maximalny pocet sucasne hrajucich zvukov, ktore bude dana entita pouzivat
// vrati sound ID cislo, 0 ak sa zvuk nepodarilo nahrat
//---------------------------------
unsigned int CP3DSoundManager::LoadSound (const char *strSoundName, DWORD dwMaxSim)
{
	char					*pStr, szBuf[MAX_PATH];
	DWORD					dwSoundID, i;
	SOUND_ITEM				SoundItem;
	OggVorbis_File			*pOvFile = NULL;
	DWORD					dwFileSize = 0;
	DWORD					dwDataSize = 0;				// velkost zvuk. dat
	LPDIRECTSOUNDBUFFER8	pSoundBuffer8 = NULL;
	DSBUFFERDESC			sound_desc = {0};
	bool					bClearOVFileOnErr = false;
	FILE					*ogg_file = NULL;
	FSFILE					*soundFile = NULL;
	BYTE					*pMapView = NULL;
	DWORD					dwCreationFlags = 0;
	float					fMin3DDistance = DEF_SOUND_MIN_DISTANCE;
	float					fMax3DDistance = DEF_SOUND_MAX_DISTANCE;
	bool					bStreaming = false;
	bool					bAIdetect = false;
	bool					bContinualPlay = false;
	P3DSAMPLE_OVERRIDE		OverPriorFlags = P3DSAMPLE_OVER_DEFAULT;
	P3DSOUND_PRIORITY		SoundPriority = P3DSOUND_PRIORITY_MEDIUM;


	if ((!m_bInitialized) || (m_pDS == NULL) || (strSoundName == NULL) || (strSoundName[0] == 0) || \
		(dwMaxSim == 0) || (m_pXmlSoundDefs == NULL) || (m_dwHashTableItemsCount == HASH_TABLE_SIZE))
		return 0;


	// urcenie SoundID
	dwSoundID = StringHashFunc (strSoundName);

	if (dwSoundID == 0)	return 0;		// kolizia

	for (i=0; i<HASH_TABLE_SIZE; i++)
		if (m_SoundHashTable[i].dwSoundID == dwSoundID)
			return dwSoundID;		// zvuk je uz nahrany

	//////////////////////////////////////////////////////////////////////////
	// nacitanie xml definicie zvuku
	ezxml_t	effect_xml_iter, effect_xml_subitem;
	ezxml_t	effect_xml = NULL;

	// vyhladanie definicie zvuku
	for (effect_xml_iter = g_pXML->Child (m_pXmlSoundDefs, "SoundObject"); effect_xml_iter != NULL; effect_xml_iter = effect_xml_iter->next)
	{
		if ((pStr = (char*)g_pXML->Attr (effect_xml_iter, "name")) == NULL)
			continue;
		if (stricmp (pStr, strSoundName) == 0)
		{effect_xml = effect_xml_iter; break;}
	}
	if (effect_xml == NULL)
		return 0;	// zvuk s danym nazvom neexistuje

	// nacitanie atributov

	// flag
	pStr = (char*)g_pXML->Attr (effect_xml, "flags");
	if (pStr) dwCreationFlags = atoi(pStr);
#ifdef _DEBUG
	dwCreationFlags |= DSBCAPS_GLOBALFOCUS;	// pouzivane len pre debug ucely
#else
	dwCreationFlags &= ~DSBCAPS_GLOBALFOCUS;
#endif
	// priority
	pStr = (char*)g_pXML->Attr (effect_xml, "priority");
	if (pStr) SoundPriority = (P3DSOUND_PRIORITY)atoi(pStr);
	// sample override priority
	pStr = (char*)g_pXML->Attr (effect_xml, "over_prior");
	if (pStr) OverPriorFlags = (P3DSAMPLE_OVERRIDE)atoi(pStr);
	// TODO: upravit OverPriorFlags podla dwCreationFlags !!!
	// streaming
	pStr = (char*)g_pXML->Attr (effect_xml, "streaming");
	if (pStr) bStreaming = !!atoi(pStr);
	// ai_detect
	pStr = (char*)g_pXML->Attr (effect_xml, "ai_detect");
	if (pStr) bAIdetect = !!atoi(pStr);
	// continual_play
	if (dwCreationFlags & P3DBCAPS_CTRL3D)
	{
		pStr = (char*)g_pXML->Attr (effect_xml, "continual_play");
		if (pStr) bContinualPlay = !!atoi(pStr);
	}
	// min & max 3D distance
	pStr = (char*)g_pXML->Attr (effect_xml, "max_dist");
	if (pStr)
	{
		fMax3DDistance = (float)atof(pStr);
		fMin3DDistance = fMax3DDistance / MIN_MAX_DISTANCE_RATIO;
	}

	// nacitanie zvukov
	// TODO: pridat wildcards
	// TODO: pridat nacitanie aliasov
	effect_xml_subitem = g_pXML->Child (effect_xml, "wave");
	if ((effect_xml_subitem == NULL) || (effect_xml_subitem->txt == NULL))
		return 0;
	//////////////////////////////////////////////////////////////////////////

	char szFileNameTemp[MAX_PATH] = {0};
	strcpy (szFileNameTemp, effect_xml_subitem->txt);

	// uprava suborovej cesty
	strlwr (szFileNameTemp);
	while (pStr = strchr(szFileNameTemp, '/'))
		*pStr = '\\';

	DWORD dwFileNameLen = (DWORD)strlen (szFileNameTemp);
	if (dwFileNameLen <= 4) return 0;

	char *szPtr = (char*)(szFileNameTemp + dwFileNameLen - 4);


	if (bStreaming)
		dwCreationFlags |= DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;

	if ((OverPriorFlags == P3DSAMPLE_OVER_3DPOS) && ((dwCreationFlags & P3DBCAPS_CTRL3D) == 0))
		OverPriorFlags = P3DSAMPLE_OVER_DEFAULT;

	if ((dwCreationFlags & DSBCAPS_CTRL3D) && (dwCreationFlags & DSBCAPS_LOCSOFTWARE))
		dwCreationFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;	// plati len pre software buffers

	if (dwMaxSim > 255) dwMaxSim = 1;

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	if (stricmp(szPtr, ".wav") == 0)										// spracovanie wav suborov
	{
		WAVE_SOUNDFILE_DESC	sound_file_desc = {0};

		sprintf (szBuf, "sound\\%s", szFileNameTemp);

		// loading suboru do pamate
		soundFile = g_pFS->Load (szBuf, pMapView, dwFileSize);
		if ((soundFile == NULL) || (pMapView == NULL)) return 0;

		if (dwFileSize < 200) goto err_quit1;		// file size minimum 200B

		// getting wave format info
		if (!GetWaveFileInfo (pMapView, dwFileSize, sound_file_desc))
			goto err_quit1;		// neplatny format wave suboru

		if (bStreaming)
			if (sound_file_desc.dwSoundDataSize < sound_file_desc.wfex.nAvgBytesPerSec)
				goto err_quit1;		// streamig zvuk musi trvat min 1 sekundu

		//////////////////////////////////////////////////////////////////////////
#ifdef CONVERT_3DWAVE_TO_MONO
		// 3D zvuk nemoze mat stereo format - konverzia stereo na mono
		if ((sound_file_desc.wfex.nChannels > 1) && (dwCreationFlags & DSBCAPS_CTRL3D))
		{
			unsigned char	*pSampleData8 = (unsigned char*)sound_file_desc.pSoundData;
			signed short	*pSampleData16 = (signed short*)sound_file_desc.pSoundData;
			signed short	*pSampleData16Write = (signed short*)sound_file_desc.pSoundData;
			DWORD				dwLoop;

			// 8 bit PCM
			if (sound_file_desc.wfex.wBitsPerSample == 8)
			{
				dwLoop = sound_file_desc.dwSoundDataSize / 2;
				for (i=0; i<dwLoop; pSampleData8+=2, i++)
					sound_file_desc.pSoundData[i] = ((*pSampleData8) + *(pSampleData8 + 1)) / 2;
				sound_file_desc.wfex.nBlockAlign = 1;
			}
			// 16 bit PCM
			else
			{
				dwLoop = sound_file_desc.dwSoundDataSize / 4;
				for (i=0; i<dwLoop; pSampleData16+=2, pSampleData16Write++, i++)
					*pSampleData16Write = ((*pSampleData16) + *(pSampleData16 + 1)) / 2;
				sound_file_desc.wfex.nBlockAlign = 2;
			}

			sound_file_desc.dwSoundDataSize /= 2;
			sound_file_desc.wfex.nAvgBytesPerSec /= 2;
			sound_file_desc.wfex.nChannels = 1;
		}
#else
		if ((sound_file_desc.wfex.nChannels > 1) && (dwCreationFlags & DSBCAPS_CTRL3D))
		{g_pFS->UnLoad (soundFile, pMapView); return 0;}
#endif
		//////////////////////////////////////////////////////////////////////////
		sound_desc.dwSize = sizeof(DSBUFFERDESC);
		sound_desc.dwFlags = dwCreationFlags;
		if (bStreaming)
			sound_desc.dwBufferBytes = sound_file_desc.wfex.nAvgBytesPerSec;	// 1s pre stream buffer
		else
			sound_desc.dwBufferBytes = sound_file_desc.dwSoundDataSize;
		sound_desc.dwBufferBytes &= 0xFFFFFFFE;	// zarovnanie na nasobok 2
		sound_desc.lpwfxFormat = &sound_file_desc.wfex;
		// guid3DAlgorithm - dalsie mozne flagy s lepsou kvalitou 3D zvuku
		// a tiez s vacsou narocnostou na vypocty su (DS3DALG_NO_VIRTUALIZATION), DS3DALG_HRTF_LIGHT a DS3DALG_HRTF_FULL
		sound_desc.guid3DAlgorithm = (dwCreationFlags & DSBCAPS_CTRL3D) == 0 ? GUID_NULL : DS3DALG_NO_VIRTUALIZATION;

		// vytvorenie DirectSound objektu zvuku
		LPDIRECTSOUNDBUFFER	pSoundBuffer;
		if (SUCCEEDED (m_pDS->CreateSoundBuffer (&sound_desc, &pSoundBuffer, NULL))) 
		{
			// vytvoreny buffer ma typ LPDIRECTSOUNDBUFFER, konverzia na LPDIRECTSOUNDBUFFER8
			pSoundBuffer->QueryInterface (IID_IDirectSoundBuffer8, (LPVOID*) &pSoundBuffer8);
			pSoundBuffer->Release();
		}
		else
			goto err_quit1;

		if (pSoundBuffer8 == NULL) goto err_quit1;

		// naplnenie bufferu datami wav suboru
		LPVOID lpvWrite;
		DWORD  dwBufferLength;
		if (FAILED (pSoundBuffer8->Lock (0, 0, &lpvWrite, &dwBufferLength, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
			goto err_quit1;
		memcpy (lpvWrite, sound_file_desc.pSoundData, min(sound_file_desc.dwSoundDataSize, dwBufferLength));
		pSoundBuffer8->Unlock (lpvWrite, dwBufferLength, NULL, 0);

		//////////////////////////////////////////////////////////////////////////

		SoundItem.pSoundBufferList = new SOUND_BUFFER[dwMaxSim];
		if (SoundItem.pSoundBufferList == NULL)
			goto err_quit1;

		// vyplnenie spolocnych poloziek
		strncpy (SoundItem.strSoundName, strSoundName, DEF_SOUNDNAME_SIZE);
		SoundItem.dwSoundID = dwSoundID;
		SoundItem.dwCreationFlags = dwCreationFlags;
		SoundItem.ucType = SOUND_TYPE_WAV;
		SoundItem.bStream = bStreaming;
		SoundItem.ucSampleOverPrior = (BYTE)OverPriorFlags;
		SoundItem.ucSoundCount = (BYTE)dwMaxSim;
		SoundItem.bMuteAtMaxDist = bContinualPlay;

		// vyplnenie "stream" poloziek
		if (bStreaming)
		{
			SoundItem.intSilenceValue = (sound_file_desc.wfex.wBitsPerSample == 8) ? 128 : 0;
			SoundItem.dwUpdateBlockSize = sound_desc.dwBufferBytes / 2;
			SoundItem.dwFileRawDataSize = sound_file_desc.dwSoundDataSize;
			// alokovanie pamate a kopirovanie zvukovych dat
			SoundItem.pWaveSoundData = new BYTE[sound_file_desc.dwSoundDataSize];
			if (SoundItem.pWaveSoundData == NULL) goto err_quit3;
			memcpy (SoundItem.pWaveSoundData, sound_file_desc.pSoundData, sound_file_desc.dwSoundDataSize);
		}

		for (i=0; i<SoundItem.ucSoundCount; i++)
		{
			SoundItem.pSoundBufferList[i].bStereo = (sound_file_desc.wfex.nChannels == 1) ? false : true;
			if (i == 0)
				SoundItem.pSoundBufferList[i].pSoundBuffer = pSoundBuffer8;
			// duplikovanie sound bufrov, ak je to potrebne
			else if (dwCreationFlags & P3DBCAPS_CTRLFX)	// pre zvuky s efektmi je treba vytvorit novy bufer
			{
				pSoundBuffer8 = NULL;
				if (FAILED (m_pDS->CreateSoundBuffer (&sound_desc, &pSoundBuffer, NULL)))
					goto err_quit3;
				// vytvoreny buffer ma typ LPDIRECTSOUNDBUFFER, konverzia na LPDIRECTSOUNDBUFFER8
				pSoundBuffer->QueryInterface (IID_IDirectSoundBuffer8, (LPVOID*) &pSoundBuffer8);
				pSoundBuffer->Release();
				if (pSoundBuffer8 == NULL) goto err_quit3;
				// naplnenie bufferu datami wav suboru
				if (FAILED (pSoundBuffer8->Lock (0, 0, &lpvWrite, &dwBufferLength, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
				{pSoundBuffer8->Release (); goto err_quit3;}
				memcpy (lpvWrite, sound_file_desc.pSoundData, min(sound_file_desc.dwSoundDataSize, dwBufferLength));
				pSoundBuffer8->Unlock (lpvWrite, dwBufferLength, NULL, 0);
				SoundItem.pSoundBufferList[i].pSoundBuffer = pSoundBuffer8;
			}
			else if (FAILED (m_pDS->DuplicateSoundBuffer (pSoundBuffer8, (LPDIRECTSOUNDBUFFER*)&SoundItem.pSoundBufferList[i].pSoundBuffer)))
				goto err_quit3;

			if (dwCreationFlags & DSBCAPS_CTRL3D)		// query 3D interface
			{
				if (FAILED (SoundItem.pSoundBufferList[i].pSoundBuffer->QueryInterface (IID_IDirectSound3DBuffer8, \
					(LPVOID*)&SoundItem.pSoundBufferList[i].pSound3DBuffer)))
					goto err_quit3;
				// nastavenie min a max dosahu 3D bufru
				SoundItem.pSoundBufferList[i].pSound3DBuffer->SetMinDistance (fMin3DDistance, DS3D_DEFERRED);
				SoundItem.pSoundBufferList[i].pSound3DBuffer->SetMaxDistance (fMax3DDistance, DS3D_DEFERRED);
			}

			if (SoundItem.bStream)
			{
				SoundItem.pSoundBufferList[i].dwCurrentFileOffset = SoundItem.dwUpdateBlockSize * 2;
				// vytvorenie notify udalosti - iba pre stream bufre
				DSBPOSITIONNOTIFY		PositionNotify[2];
				LPDIRECTSOUNDNOTIFY8	lpDsNotify;
				PositionNotify[0].dwOffset = SoundItem.dwUpdateBlockSize;
				PositionNotify[0].hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
				PositionNotify[1].dwOffset = SoundItem.dwUpdateBlockSize * 2 - 2;
				PositionNotify[1].hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
				if ((PositionNotify[0].hEventNotify == NULL) || (PositionNotify[1].hEventNotify == NULL))
					goto err_quit3;

				if (FAILED (SoundItem.pSoundBufferList[i].pSoundBuffer->QueryInterface (IID_IDirectSoundNotify8, (LPVOID*)&lpDsNotify)))
					goto err_quit3;
				if (FAILED (lpDsNotify->SetNotificationPositions (2, (LPCDSBPOSITIONNOTIFY)&PositionNotify)))
				{lpDsNotify->Release (); goto err_quit3;}
				lpDsNotify->Release ();
				SoundItem.pSoundBufferList[i].hUpdateEvents[0] = PositionNotify[0].hEventNotify;
				SoundItem.pSoundBufferList[i].hUpdateEvents[1] = PositionNotify[1].hEventNotify;
			}
		}

		g_pFS->UnLoad (soundFile, pMapView);

		if (!InsertNewSoundItem (SoundItem))
			return 0;
	}

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	else if (stricmp (szPtr, ".ogg") == 0)								// spracovanie ogg suborov
	{
		int intSampleCount = 0;
		DWORD dwBufferSize = 0;

		pOvFile = new OggVorbis_File;
		if (pOvFile == NULL) goto err_quit2;
		memset (pOvFile, 0, sizeof(OggVorbis_File));

		// TODO: FIXME: docasne bez file systemu !!!
		/*
		sprintf (szBuf, "sound\\%s", szFileNameTemp);
		ogg_file = g_pFS->Open (szBuf);

		// vytvorenie objektu OggVorbis_File
		if (ov_open (ogg_file, pOvFile, NULL, 0) != 0)
		{g_pFS->Close (ogg_file); ogg_file = NULL; goto err_quit2;}
		// TODO: skusit !!!
		// g_pFS->Close (ogg_file);
		*/
		sprintf (szBuf, "data/sound/%s", szFileNameTemp);
		ogg_file = fopen (szBuf, "rb");
		if (ogg_file == NULL) goto err_quit2;

		// vytvorenie objektu OggVorbis_File
		if (ov_open (ogg_file, pOvFile, NULL, 0) != 0)
		{fclose(ogg_file); ogg_file = NULL; goto err_quit2;}
		// TODO: skusit ???
		// fclose(ogg_file);

		bClearOVFileOnErr = true;

		if (ov_seekable (pOvFile) == 0) goto err_quit2;		// not seekable

		// the vorbis_info struct keeps the most of the interesting format info
		vorbis_info *vi = ov_info (pOvFile, -1);
		if (vi == NULL) goto err_quit2;

		// TODO: spravit konverziu na mono format, ak to bude mozne ???
		if ((vi->channels > 1) && (dwCreationFlags & DSBCAPS_CTRL3D))
			goto err_quit2;			// stereo zvuky nemozu byt 3D

		// nastavenie wave formatu
		WAVEFORMATEX wfm;
		wfm.cbSize          = sizeof(wfm);
		wfm.nChannels       = vi->channels;
		wfm.wBitsPerSample  = 16;                    // ogg vorbis je vzdy 16 bitovy
		wfm.nSamplesPerSec  = vi->rate;
		wfm.nBlockAlign     = 2 * wfm.nChannels;
		wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;
		wfm.wFormatTag      = WAVE_FORMAT_PCM;

		intSampleCount = (int)ov_pcm_total (pOvFile, -1);
		if (intSampleCount <= 0) goto err_quit2;
		dwBufferSize = dwDataSize = intSampleCount * wfm.nBlockAlign;

		if (bStreaming)
			if (dwDataSize < wfm.nAvgBytesPerSec)
				goto err_quit2;		// streamig zvuk musi trvat min 1 sekundu
			else
				dwBufferSize = wfm.nAvgBytesPerSec & 0xFFFFFFFE;	// zarovnanie na nasobok 2

		// vytvorenie sound bufferu
		sound_desc.dwSize = sizeof(DSBUFFERDESC);
		sound_desc.dwFlags = dwCreationFlags;
		sound_desc.dwBufferBytes = dwBufferSize;
		sound_desc.lpwfxFormat = &wfm;
		// DS3DALG_NO_VIRTUALIZATION, DS3DALG_HRTF_LIGHT, DS3DALG_HRTF_FULL
		sound_desc.guid3DAlgorithm = (dwCreationFlags & DSBCAPS_CTRL3D) == 0 ? GUID_NULL : DS3DALG_NO_VIRTUALIZATION;

		// vytvorenie sound bufferu
		LPDIRECTSOUNDBUFFER	pSoundBuffer;
		if (FAILED (m_pDS->CreateSoundBuffer (&sound_desc, &pSoundBuffer, NULL))) 
			goto err_quit2;
		// vytvoreny buffer ma typ LPDIRECTSOUNDBUFFER, konverzia na LPDIRECTSOUNDBUFFER8
		pSoundBuffer->QueryInterface (IID_IDirectSoundBuffer8, (LPVOID*) &pSoundBuffer8);
		pSoundBuffer->Release();

		if (pSoundBuffer8 == NULL) goto err_quit2;

		// naplnenie bufferu datami ogg suboru
		char *lpvWrite;
		DWORD  dwBufferLength;
		if (SUCCEEDED (pSoundBuffer8->Lock (0, 0, (LPVOID*)&lpvWrite, &dwBufferLength, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
		{
			int intSection = 0;
			DWORD dwPos = 0;
			LONG lRead = 1;
			dwBufferSize = min (dwDataSize, dwBufferLength);

			ov_raw_seek (pOvFile, 0);
			while ((lRead != 0) && (dwPos < dwBufferSize))		// dekodovanie
			{
				lRead = ov_read (pOvFile, lpvWrite + dwPos, dwBufferSize - dwPos, 0, 2, 1, &intSection);
				if (lRead < 0)		// chyba
				{pSoundBuffer8->Unlock (lpvWrite, dwBufferLength, NULL, 0); goto err_quit2;}
				dwPos += lRead;
			}
			pSoundBuffer8->Unlock (lpvWrite, dwBufferLength, NULL, 0);
		}
		else
			goto err_quit2;

		//////////////////////////////////////////////////////////////////////////

		SoundItem.pSoundBufferList = new SOUND_BUFFER[dwMaxSim];
		if (SoundItem.pSoundBufferList == NULL)
			goto err_quit2;

		// vyplnenie spolocnych poloziek
		strncpy (SoundItem.strSoundName, strSoundName, DEF_SOUNDNAME_SIZE);
		SoundItem.dwSoundID = dwSoundID;
		SoundItem.dwCreationFlags = dwCreationFlags;
		SoundItem.ucType = SOUND_TYPE_OGG;
		SoundItem.bStream = bStreaming;
		SoundItem.ucSampleOverPrior = (BYTE)OverPriorFlags;
		SoundItem.ucSoundCount = (BYTE)dwMaxSim;
		SoundItem.bMuteAtMaxDist = bContinualPlay;

		// vyplnenie zvysnych poloziek pre stream bufre
		if (SoundItem.bStream)
		{
			SoundItem.intSilenceValue = 0;
			SoundItem.dwUpdateBlockSize = dwBufferSize / 2;
			SoundItem.dwFileRawDataSize = dwDataSize;
			SoundItem.pOggFileObject = pOvFile;
		}


		for (i=0; i<SoundItem.ucSoundCount; i++)
		{
			SoundItem.pSoundBufferList[i].bStereo = (wfm.nChannels == 1) ? false : true;
			if (i == 0)
				SoundItem.pSoundBufferList[i].pSoundBuffer = pSoundBuffer8;
			// duplikovanie sound bufrov, ak je to potrebne
			else if (dwCreationFlags & P3DBCAPS_CTRLFX)		// pre zvuky s efektmi je potrebne vytvorit novy bufer
			{
				pSoundBuffer8 = NULL;
				if (FAILED (m_pDS->CreateSoundBuffer (&sound_desc, &pSoundBuffer, NULL)))
					goto err_quit3;
				// vytvoreny buffer ma typ LPDIRECTSOUNDBUFFER, konverzia na LPDIRECTSOUNDBUFFER8
				pSoundBuffer->QueryInterface (IID_IDirectSoundBuffer8, (LPVOID*) &pSoundBuffer8);
				pSoundBuffer->Release();
				if (pSoundBuffer8 == NULL) goto err_quit3;
				// naplnenie bufferu datami ogg suboru
				if (FAILED (pSoundBuffer8->Lock (0, 0, (LPVOID*)&lpvWrite, &dwBufferLength, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
				{pSoundBuffer8->Release(); goto err_quit3;}
				int intSection = 0;
				DWORD dwPos = 0;
				LONG lRead = 1;
				dwBufferSize = min (dwDataSize, dwBufferLength);

				ov_raw_seek (pOvFile, 0);
				while((lRead != 0) && (dwPos < dwBufferSize))		// dekodovanie
				{
					lRead = ov_read (pOvFile, lpvWrite + dwPos, dwBufferSize - dwPos, 0, 2, 1, &intSection);
					if (lRead < 0)		// chyba
					{pSoundBuffer8->Unlock (lpvWrite, dwBufferLength, NULL, 0); pSoundBuffer8->Release(); goto err_quit3;}
					dwPos += lRead;
				}
				pSoundBuffer8->Unlock (lpvWrite, dwBufferLength, NULL, 0);
				SoundItem.pSoundBufferList[i].pSoundBuffer = pSoundBuffer8;
			}
			else if (FAILED(m_pDS->DuplicateSoundBuffer (pSoundBuffer8, (LPDIRECTSOUNDBUFFER*)&SoundItem.pSoundBufferList[i].pSoundBuffer)))
				goto err_quit3;

			// query 3D interface
			if (dwCreationFlags & DSBCAPS_CTRL3D)
			{
				if (FAILED (SoundItem.pSoundBufferList[i].pSoundBuffer->QueryInterface (IID_IDirectSound3DBuffer8, \
					(LPVOID*)&SoundItem.pSoundBufferList[i].pSound3DBuffer)))
					goto err_quit3;
				// nastavenie min a max dosahu 3D bufru
				SoundItem.pSoundBufferList[i].pSound3DBuffer->SetMinDistance (fMin3DDistance, DS3D_DEFERRED);
				SoundItem.pSoundBufferList[i].pSound3DBuffer->SetMaxDistance (fMax3DDistance, DS3D_DEFERRED);
			}

			if (SoundItem.bStream)
			{
				SoundItem.pSoundBufferList[i].dwCurrentFileOffset = SoundItem.dwUpdateBlockSize * 2;
				// vytvorenie notify udalosti - iba pre stream bufre
				DSBPOSITIONNOTIFY		PositionNotify[2];
				LPDIRECTSOUNDNOTIFY8	lpDsNotify;
				PositionNotify[0].dwOffset = SoundItem.dwUpdateBlockSize;
				PositionNotify[0].hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
				PositionNotify[1].dwOffset = SoundItem.dwUpdateBlockSize * 2 - 2;
				PositionNotify[1].hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
				if ((PositionNotify[0].hEventNotify == NULL) || (PositionNotify[1].hEventNotify == NULL))
					goto err_quit3;

				if (FAILED (SoundItem.pSoundBufferList[i].pSoundBuffer->QueryInterface (IID_IDirectSoundNotify8, (LPVOID*)&lpDsNotify)))
					goto err_quit3;
				if (FAILED (lpDsNotify->SetNotificationPositions (2, (LPCDSBPOSITIONNOTIFY)&PositionNotify)))
				{lpDsNotify->Release (); goto err_quit3;}
				lpDsNotify->Release ();
				SoundItem.pSoundBufferList[i].hUpdateEvents[0] = PositionNotify[0].hEventNotify;
				SoundItem.pSoundBufferList[i].hUpdateEvents[1] = PositionNotify[1].hEventNotify;
			}
		}

		if (!InsertNewSoundItem (SoundItem))
			return 0;
	}
	else
		return 0;		// nepodporovany zvukovy format


	return dwSoundID;

err_quit1:
	if (pSoundBuffer8)
		pSoundBuffer8->Release ();
	if (soundFile)
		g_pFS->UnLoad (soundFile, pMapView);
	return 0;
err_quit2:
	if (pSoundBuffer8)
		pSoundBuffer8->Release ();
	if (pOvFile)
	{
		if (bClearOVFileOnErr) ov_clear (pOvFile);
		delete pOvFile;
	}
	return 0;
err_quit3:
	SoundItem.Unload ();
	SAFE_DELETE_ARRAY (pMapView)
		return 0;
}
