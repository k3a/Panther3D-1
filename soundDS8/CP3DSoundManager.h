
//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Sound Engine classes declaration
//-----------------------------------------------------------------------------


/************************************************************************


												CP3DSoundManager

 CP3DSoundManager je hlavna trieda, ktora inicializuje DirectSound a robi kompletny management vsetkych zvukov.
 Program by mal na zaciatku vytvorit jednu instanciu tejto triedy, ktora sa bude uvolnovat az pri
 ukonceni programu. SoundEngine nepovoli vytvorenie viac ako jednej instancie triedy CP3DSoundManager.
 
 Kazdy zvuk je identifikovany svojim menom (max. DEF_SOUNDNAME_SIZE znakov).
 
 CP3DSoundManager::m_p3DListener predstavuje objekt virtualneho posluchaca v 3D prietore.
 Najdolezitejsia "listener" fcia je Listener_SetPositionOrientation().

 Fcia CP3DSoundManager::Update() je dost dolezita a treba ju volat kazdy frame. Najlepsie v casti medzi
 EndScene() a Present(). Spusta zvuky, updatuje stream bufre, nastavuje poziciu 3D zvukov a tak pod.

 Ak je to mozne doporucujem nacitat zvuky v davkach. Napr. pred zaciatkom levelu sa nacitaju vsetky
 zvuky pouzite v tomto levele. Zvuky, ktore netreba nacitat pred zaciatkom levelu su napr. objemne
 dialogy alebo hudba ... vyskytuju sa len v urcitej casti hry. Taketo zvuky je mozne nacitat
 "on the fly", podla potreby. Na objemne zvukove subory (dialogy, hudba atd) je vhodne pouzivat format ogg
 a nacitat ich ako streaming zvuky. Na vacsinu beznych "kratkych" zvukov ako su zvuky zbrani, kroky,
 rozne mechanicke zvuky, atd. je najlepsie  pouzivat format wav. Upozornujem ze ogg subory mozu
 v pamati zaberat po rozbaleni aj viac ako desatnasobok svojej velkosti na disku. Vtedy je dobre pouzit
 streaming a po pouziti taketo zvuky uvolnovat. SoundEngine sa pozera na ogg subory len ako na komprimovane wav
 subory a vzdy pred pouzitim ich dekomprimuje. T.j. z pohladu SoundEngine je rozdiel medzi wav a ogg iba ten,
 ze ogg zaberaju na disku menej miesta. Stream zvuky su viac narocne na reziu(CPU time) ako non-stream
 zvuky. Stream bufre sa vzdy umiestnuju do systemovej pamati (RAM) a miesaju sa softwarovo (CPU),
 vyzaduju reziu ako pravidelna dekompresia a presuny dat.

 Takze z pohladu performace su "najlacnejsie" zvuky non-stream, *.wav, bez flagu P3DBCAPS_CTRLFX !
 Najsetrnejsie k pamati su stream zvuky (len pre objemne zvuky)
 Najsetrnejsie k miestu na disku su ogg subory.

	Kvoli optimalizacii je vhodne, aby vsetky zvuky mali rovnaky format, t.j. vzorkovaciu frekvenciu,
 pocet kanalov (mono/stereo) a pocet bitov na sample. Tento format zvukov by sa mal zhodovat s
 vystupnym hardwarovym formatom. Nie je to nutnost a zvuky sa budu prehravat aj ked to nie je splnene,
 ale prinasa to pre CPU a hardware zvukovej karty dodatocne vypocty pri konverzii. Vystupny HW format
 sa da nastavit fciou CP3DSoundManager::SetPrimaryBufferFormat().

 Zvuky sa nacitavaju fciou CP3DSoundManager::LoadSound();
 Popis parametrov fcie:

 strSoundName - nazov zvuku, case-sensitive, zvuk musi byt definovany v sound_def.xml
 dwMaxSim - maximalny pocet sucasne hrajucich zvukov, ktore bude dana entita pouzivat




 P3DBCAPS_CTRL3D - vytvara zvuk s 3D vlastnostami, zdrojovy zvukovy subor musi mat mono format, vacsina
						pouzitych zvukov v hre budu 3D zvuky, 3D pozicia (a pripadne rychlost) sa nastavuje
						pomocou triedy CP3DSoundContainer
 P3DBCAPS_CTRLVOLUME - na zvuky s tymto flagom ma vplyv fcia SetMasterVolume(); kde fVolume je od 0 do 1.0f,
						nastavuje sa iba utlm, zvuky nie je mozne zosilnit
 P3DBCAPS_CTRLFX - na zvuky s tymto flagom ma vplyv fcia SetMasterFXs() a SetSoundFXs(), umoznuje pouzivat DirectSound efekty,
						treba pouzivat s mierou. Efekty v DX nie su "hardware accelerated"! Zmenu efektu volanim
						fcie SetMasterFXs() "je pocut", preto ju netreba volat prilis casto (experimentovat s
						DX efektmi sa da pomocou SoundFX.exe samplu z DX SDK)


												CP3DSoundContainer

 Kazdy objekt v hre, ktory je pohyblivy a vydava zvuky by mal mat vlastnu
 instanciu triedy IP3DSoundContainer. Trieda ma stihly, ale postacujuci interface.
 Pre vsetky 3D zvuky prehravane fciou IP3DSoundContainer::Play() sa automaticky nastavuje 3D poloha
 podla IP3DSoundContainer::m_3DPos. Je mozne nastavovat aj rychlost zvuku - doppler efekt. Trieda
 IP3DSoundContainer sluzi k jedinemu ucelu a to je priradenie 3D pozicie ku zvuku.

													Efekty a Volume

 Funkciami SetMasterVolume() a SetMasterFXs() sa nastavuje volume a efekt pre vsetky zvuky globalne.
 Ale maju dosah iba na tie zvuky, ktore boli vytvorene s flagmi P3DBCAPS_CTRLVOLUME a P3DBCAPS_CTRLFX.
 Zvuky s P3DBCAPS_CTRLFX su o dost narocnejsie na spracovanie ako bezne zvuky.

													Info

 Pozadovane minimalne DirectX8.
 Defaultne 3D jednotky su centimetre (nastavuju sa podla makra ONE_METER_DIV)
 Zvuky sa nacitaju z podadresara "Data\Sound".
 Wav subory musia mat najmenej 200B.
 Streaming zvuky musia trvat minimalne 1s.
 Zvuky s efektmi musia trvat minimalne 150ms.
 3D zvuky musia mat mono format - 1 kanal

 Kvoli statickym knizniciam treba k parametrom linkera pridat
 pre debug: /nodefaultlib:"LIBCMTD" /nodefaultlib:"LIBCD"
 pre release: /nodefaultlib:"LIBCMT" /nodefaultlib:"LIBCD"


************************************************************************/

#pragma once

#include "..\shared\IP3DSoundEngine.h"
#include "..\shared\sorting_alg.h"
#include "..\shared\HP3DVector.h"
#define DWORD_PTR	DWORD
#include <dsound.h>
#include <math.h>
#include "vorbisfile.h"
#include "common.h"


enum SOUND_TYPE {
	SOUND_TYPE_WAV,
	SOUND_TYPE_OGG,
	SOUND_TYPE_MP3		// zatial nepodporovane
};

enum STREAM_LOOP_CONST {
	STREAMLOOP_NONLOOP,
	STREAMLOOP_STOPLOOP2,
	STREAMLOOP_STOPLOOP1,
	STREAMLOOP_PLAYLOOP
};

#define		HASH_TABLE_SIZE			300		// max. pocet moznych zvukov
#define		ACTIVE_LIST_SIZE		50		// max. pocet (akychkolvek) sucasne prehravanych zvukov
#define		DEF_SOUNDNAME_SIZE		32		// max. pocet znakov identifikatora zvuku

#define MIN_MAX_DISTANCE_RATIO	7.0f
#define DEF_SOUND_MIN_DISTANCE	ONE_METER
#define DEF_SOUND_MAX_DISTANCE	(MIN_MAX_DISTANCE_RATIO * ONE_METER)

#define DEF_ROLLOFF_FACTOR	1.4f


class CP3DSoundManager;
class CP3DSoundContainer;
extern CP3DSoundManager	*g_pSoundMan;

// ak je definovane tak pri nacitani 3D stereo .wav zvukov sa zvuk konvertuje na mono format
// ak nie je definovane potom sa takyto subor nenacita
#define CONVERT_3DWAVE_TO_MONO

// wav riff chunk format
//---------------------------------
struct WAVE_CHUNK{
	DWORD		ck_ID;
	DWORD		ck_Size;
	BYTE		*ck_Data;
};

//---------------------------------
struct WAVE_SOUNDFILE_DESC{
	WAVEFORMATEX	wfex;
	BYTE			*pSoundData;
	DWORD			dwSoundDataSize;
};


//---------------------------------
struct SOUND_HANDLE_PROVIDER {
private:
	unsigned int	dwNextFreeHandle;
public:
	SOUND_HANDLE_PROVIDER() : dwNextFreeHandle(1) {}
	unsigned int Get()
	{
		unsigned int dwRet = dwNextFreeHandle;
		if (++dwNextFreeHandle == 0) dwNextFreeHandle = 1;
		return dwRet;
	}
};


// struktura reprezentuje jeden z DirectSound bufrov zvuku
//---------------------------------
struct SOUND_BUFFER {
	LPDIRECTSOUNDBUFFER8	pSoundBuffer;
	LPDIRECTSOUND3DBUFFER8	pSound3DBuffer;
	int						intActiveListIdx;		// urcuje ci sa dany zvuk prave prehrava, -1 ak sa nenachadza v active liste
	// tieto polozky su platne iba pre stream bufre
	HANDLE					hUpdateEvents[2];		// Windows udalosti signalizujuce potrebu obnovit stream bufer
	STREAM_LOOP_CONST		loopFlag;				// hodnoty: 0 pre non-looping, 3 pre looping, 1 a 2 ukoncenie loopingu
	DWORD					dwCurrentFileOffset;	// aktualna pozicia prehravaneho bufru v bajtoch, odtial sa zacne nasledujuci update
	//____________________________________
	bool					bNoneEffectSet;			// true ak buffer nema nastavene ziadne efekty
	bool					bDefVolumeSet;			// true ak je nastavene default volume
	bool					bStereo;				// true pre stereo buffer
	// funkcie
	SOUND_BUFFER() {Clear();}
	void Clear()
	{
		memset (this, 0, sizeof(SOUND_BUFFER));
		intActiveListIdx = -1;
		bNoneEffectSet = true;
		bDefVolumeSet = true;
	}
};


#define CLEARANCE_CANDIDATE		0x0001	// nastavuje sa pre uz spustene zvuky
#define PLAY_ON_UPDATE			0x0002
#define PLAY_IN_LOOP			0x0004
#define STOP_ALL_FLAG			0x0008	// flag sa nastavuje pre zvuky pozastavene fciou StopAll()
#define MUTED_AT_MAX_DIST		0x0010	// flag sa nastavuje pre 3D zvuky pozastavene pri prekroceni max. vzdialenosti

// polozka active-listu
//---------------------------------
struct ACTIVE_LIST_ITEM {
	unsigned int			dwSoundID;				// sound ID, 0 pre ziaden zvuk
	P3D_SOUND_HANDLE		dwSoundHandle;			// docasny handle umoznuje manipulovat so zvukom pocas doby jeho prehravania, 0 pre ziaden handle
	int						intHashTableIdx;		// index do tabulky m_SoundHashTable, -1 pre ziaden index
	WORD					wSoundBufferIdx;		// index pouziteho DS bufru v pSoundBufferList
	WORD					wFlags;					// pozri konstanty vyssie
	CP3DSoundContainer		*pOwner;				// "vlastnik" zvuku, urcuje 3D poziciu a rychlost zvuku
	bool					bDuplicatedBuffer;		// true ak bola vytvorena docasna kopia bufru, data bufru su v DuplicatedBuffer
	SOUND_BUFFER			DuplicatedBuffer;
	// funkcie
	ACTIVE_LIST_ITEM() {Clear();}
	void Clear()
	{
		dwSoundID = 0; dwSoundHandle = 0; intHashTableIdx = -1; wSoundBufferIdx = 0;
		wFlags = 0; pOwner = NULL; bDuplicatedBuffer = false;
		DuplicatedBuffer.Clear ();
	}
};

// struktura popisujuca jeden zvuk
//---------------------------------
struct SOUND_ITEM {
	char			strSoundName[DEF_SOUNDNAME_SIZE];
	unsigned int	dwSoundID;			// unique identifier, 0 pre ziaden zvuk
	DWORD			dwCreationFlags;	// P3DBCAPS_CTRL3D, P3DBCAPS_CTRLVOLUME, P3DBCAPS_CTRLFX
	BYTE			ucType;				// SOUND_TYPE_WAV, SOUND_TYPE_OGG
	bool			bStream;			// non-stream alebo stream
	BYTE			ucSampleOverPrior;	// override priority
	BYTE			ucSoundCount;		// pocet zvuk. bufrov v pSoundBufferList (max. 255)
	SOUND_BUFFER	*pSoundBufferList;	// pole DirectSound bufrov daneho zvuku
	DWORD			dwLoadRequests;		// pocet poziadaviek o loading zvuk. bufrov daneho zvuku
	bool			bMuteAtMaxDist;
	// tieto polozky su platne iba pre stream bufre
	int				intSilenceValue;	// 0 pre 16bit PCM, 128 pre 8bit PCM
	DWORD			dwUpdateBlockSize;
	DWORD			dwFileRawDataSize;
	union {
		BYTE			*pWaveSoundData;		// pre wav
		OggVorbis_File	*pOggFileObject;		// pre ogg
	};
	//////////////////////////////////////////////////////////////////////////
	// funkcie
	SOUND_ITEM() {Clear();}
	void Unload();
	void Clear()
		{memset (this, 0, sizeof(SOUND_ITEM));}
};



//////////////////////////////////////////////////////////////////////////
//										CP3DSoundManager
//////////////////////////////////////////////////////////////////////////

// TODO: pridat lost_sounds management, vypnutie/zapnutie efektov a nacitanie aliasov !!!
//---------------------------------
class CP3DSoundManager : public IP3DSoundManager
{
	friend SOUND_ITEM;
	friend CP3DSoundContainer;
private:
	LPDIRECTSOUND8				m_pDS;							// top-level direct sound objekt
	LPDIRECTSOUND3DLISTENER8	m_p3DListener;					// 3D listener
	P3DXVector3D					m_ListenerPosition;

	bool						m_bInitialized;
	bool						m_bInCachingState;
	P3D_SFX_DESC				m_MasterEffects[P3D_SOUNDFX_COUNT];
	DWORD						m_dwMasterEffectsNum;		// pocet efektov v m_MasterEffects
	LONG						m_lMasterVolume;
	SoundEventNotifyCallback	m_NotifyCallbackFunc;
	SOUND_HANDLE_PROVIDER		m_SoundHandleProvider;		// pouziva sa pre navratove hodnoty fcii PlaySound()

	ACTIVE_LIST_ITEM			m_ActiveList [ACTIVE_LIST_SIZE];	// zoznam prave prehravanych zvukov
	SOUND_ITEM					m_SoundHashTable [HASH_TABLE_SIZE];	// zoradena vzostupne podla dwSoundID
	DWORD						m_dwHashTableItemsCount;			// pocet zvukov v m_SoundHashTable
	// xml
	ezxml_t						m_pXmlSoundDefs;

	// convar effect_change callback
	static void SoundFX_Change_Callback (ConVar* pCVar);
	// callback pre ukoncene entity
	void ContainerDestroyAlert (IP3DSoundContainer *pOwner);
	// functions
	bool CreateTemporaryBuffer (IN const SOUND_ITEM &SoundItem, OUT SOUND_BUFFER &SoundBuffer);
	bool UnloadSound (DWORD dwSoundIdx);
	bool FindSoundByName (IN const char *strSoundName, OUT DWORD &dwHashTableIdx);
	bool FindSoundByID (IN unsigned int dwSoundID, OUT DWORD &dwHashTableIdx);
	int FindSoundByNameInActiveList (const char *strSoundName, IP3DSoundContainer *pOwner);
	int FindSoundByIDInActiveList (unsigned int dwSoundID, IP3DSoundContainer *pOwner);
	int FindSoundByHandleInActiveList (P3D_SOUND_HANDLE dwSoundHandle);
	bool SetEffectOnBuffer (LPDIRECTSOUNDBUFFER8 pSoundBuffer8, bool bSetMasterEffects = true, DWORD dwEffectsNum = 0, P3D_SFX_DESC *pEffects = NULL);

	bool InsertNewSoundItem (const SOUND_ITEM	&newSoundItem);
	void RecalculateActiveListIndexes();
	void RemoveFromActiveList (int intIdx);
	void UpdateMasterVolumeEffects (const SOUND_ITEM &pSoundItem, SOUND_BUFFER &pSoundBuffer);
	P3D_SOUND_HANDLE PlayFreeSample (const char *strSoundName, IP3DSoundContainer *pOwner, P3DXVector3D *pSoundPos, bool bLoop);
	P3D_SOUND_HANDLE PlayFreeSample (unsigned int dwSoundID, IP3DSoundContainer *pOwner, P3DXVector3D *pSoundPos, bool bLoop);
	P3D_SOUND_HANDLE PlayFreeSample (DWORD dwSoundIdx, IP3DSoundContainer *pOwner, P3DXVector3D *pSoundPos, bool bLoop);
	P3D_SOUND_HANDLE PlaySoundInternal (DWORD dwActiveListIdx, IP3DSoundContainer *pOwner, bool bLoop);

public:
	CP3DSoundManager();
	~CP3DSoundManager();

	bool Init();
	bool Initialize (HWND hWnd, SoundEventNotifyCallback notifyCallbackFunc = NULL);
	bool SetPrimaryBufferFormat (SOUND_CHANNELS PrimaryChannels, SOUND_BITS_PER_SAMPLE BitsPerSample, SOUND_SAMPLES_PER_SECOND SamplesPerSec);

	void StartCaching ();
	void StopCaching ();
	unsigned int LoadSound (const char *strSoundName, DWORD dwMaxSim = 1);
	bool UnloadSound (const char *strSoundName);
	bool UnloadSound (unsigned int dwSoundID);
	void UnloadAll ();
	bool SoundName2SoundID (IN const char *strSoundName, OUT unsigned int &dwSoudID);
	bool SoundID2SoundName (IN unsigned int dwSoudID, OUT const char *strSoundName);
	// playing functions
	P3D_SOUND_HANDLE PlaySound (const char *strSoundName, bool bLoop = false);
	P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, bool bLoop = false);
	P3D_SOUND_HANDLE PlaySound (const char *strSoundName, P3DXVector3D &pSoundPos, bool bLoop = false);
	P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, P3DXVector3D &pSoundPos, bool bLoop = false);
	P3D_SOUND_HANDLE PlaySound (const char *strSoundName, IP3DSoundContainer *pOwner, bool bLoop = false);
	P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, IP3DSoundContainer *pOwner, bool bLoop = false);

	bool StopSound (P3D_SOUND_HANDLE dwSoundHandle);
	void StopAll (bool bPauseOnly = true);
	void ResumeAll ();

	bool SetSoundVolume (P3D_SOUND_HANDLE soundHandle, float fVolume = 1.0f);
	bool SetSoundFXs (P3D_SOUND_HANDLE soundHandle, DWORD dwEffectsNum = 0, P3D_SFX_DESC *pEffects = NULL);
	// global parameters
	void SetMasterVolume (float fVolume = 1.0f);
	bool SetMasterFXs (DWORD dwEffectsNum, P3D_SFX_DESC *pEffects);
	bool Apply3DChanges ();

	// listener functions
	bool Listener_SetAllParameters (P3DLISTENER	&pListenerParams);
	bool Listener_GetAllParameters (P3DLISTENER	&pListenerParams);
	bool Listener_SetPositionOrientation (P3DXVector3D &vecNewPosition, P3DXVector3D &vecNewFront);
	bool Listener_SetVelocity (P3DXVector3D &vecNewVelocity);
	bool Listener_SetDopplerFactor (float fNewDopplerFactor);
	bool Listener_SetRolloffFactor (float fNewRolloffFactor);

	void Update();
};

//////////////////////////////////////////////////////////////////////////
//										CP3DSoundContainer
//////////////////////////////////////////////////////////////////////////

//---------------------------------
class CP3DSoundContainer : public IP3DSoundContainer
{
	friend void CP3DSoundManager::Update ();
private:
	void *m_CallbackData;

	void FireNotifyCallback (P3D_SOUND_HANDLE soundHandle);

public:
	CP3DSoundContainer();
	~CP3DSoundContainer();
	bool Init ();

	void RegisterCallbackData (void *callbackData);

	P3D_SOUND_HANDLE PlaySound (const char *strSoundName, bool bLoop = false);
	P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, bool bLoop = false);
	bool Stop (P3D_SOUND_HANDLE dwSoundHandle);
}; 


//////////////////////////////////////////////////////////////////////////
inline void SOUND_ITEM::Unload()
{
	if (pSoundBufferList)
	{
		for (DWORD i=0; i<(DWORD)ucSoundCount; i++)
		{
			if (pSoundBufferList[i].intActiveListIdx >= 0)
				if (g_pSoundMan)
					g_pSoundMan->RemoveFromActiveList (pSoundBufferList[i].intActiveListIdx);
			SAFE_RELEASE (pSoundBufferList[i].pSoundBuffer)
			SAFE_RELEASE (pSoundBufferList[i].pSound3DBuffer)
			if (bStream)
			{
				if (pSoundBufferList[i].hUpdateEvents[0])
					CloseHandle (pSoundBufferList[i].hUpdateEvents[0]);
				if (pSoundBufferList[i].hUpdateEvents[1])
					CloseHandle (pSoundBufferList[i].hUpdateEvents[1]);
			}
		}
		delete[] pSoundBufferList;
	}
	if ((bStream) && (pWaveSoundData != NULL))
	{
		if (ucType == SOUND_TYPE_WAV)
			delete[] pWaveSoundData;
		else if (ucType== SOUND_TYPE_OGG)
			ov_clear (pOggFileObject);
	}
	Clear();
}
