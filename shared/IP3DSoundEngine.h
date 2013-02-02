
//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Sound Engine interfaces
//-----------------------------------------------------------------------------

#pragma once


#include <windows.h>
#include "..\shared\interface.h"
#include "..\shared\types.h"
#include "..\shared\symbols.h"
#include "..\shared\IP3DConsole.h"


typedef	unsigned int	P3D_SOUND_HANDLE;

typedef void (*SoundEventNotifyCallback) (void *callbackData, P3D_SOUND_HANDLE soundHandle);

// flagy (creation flags) pre LoadSound()
#define		P3DBCAPS_CTRL3D				0x00000010
#define		P3DBCAPS_CTRLVOLUME			0x00000080
#define		P3DBCAPS_CTRLFX				0x00000200
//#define		P3DBCAPS_GLOBALFOCUS			0x00008000		// len pre debug ucely ???


enum P3DSOUND_PRIORITY {
	P3DSOUND_PRIORITY_LOW,
	P3DSOUND_PRIORITY_MEDIUM,
	P3DSOUND_PRIORITY_HIGH
};

// sound sample override priority constants
enum P3DSAMPLE_OVERRIDE {
	P3DSAMPLE_OVER_DEFAULT,	// def. nastavenie - pre 3D zvuky plati P3DSAMPLE_OVER_3DPOS a pre non-3D zvuky plati P3DSAMPLE_OVER_PLAYPOS
	P3DSAMPLE_OVER_NONE,		// ak nie je volny ziaden sound buffer - nevrati ziaden handle
	P3DSAMPLE_OVER_3DPOS,		// -//- vrati zvuk, ktory je v 3D priestore najdalej od listenera (plati iba pre 3D zvuky)
	P3DSAMPLE_OVER_FORCE_FIRST_BUFFER	// -//- vrati jeden z prave prehravanych bufrov
};

enum SOUND_CHANNELS {
	SOUND_CHANNELS_MONO = 1,
	SOUND_CHANNELS_STEREO = 2
};

enum SOUND_BITS_PER_SAMPLE {
	SOUND_BITS_8 = 8,
	SOUND_BITS_16 = 16,
// 	SOUND_BITS_24 = 24,		// nepouzivat!
// 	SOUND_BITS_32 = 32,
// 	SOUND_BITS_64 = 64
};

enum SOUND_SAMPLES_PER_SECOND {
	SOUND_SAMPLERATE_8000	= 8000,
	SOUND_SAMPLERATE_11025	= 11025,
	SOUND_SAMPLERATE_22050	= 22050,
	SOUND_SAMPLERATE_32000	= 32000,
	SOUND_SAMPLERATE_44100	= 44100,
	SOUND_SAMPLERATE_48000	= 48000,
	SOUND_SAMPLERATE_88200	= 88200,
	SOUND_SAMPLERATE_96000	= 96000,
	SOUND_SAMPLERATE_192000	= 192000
};


//////////////////////////////////////////////////////////////////////////
//									DirectSound struktury
//////////////////////////////////////////////////////////////////////////

struct P3DLISTENER
{
	DWORD				dwSize;
	P3DXVector3D		vPosition;
	P3DXVector3D		vVelocity;
	P3DXVector3D		vOrientFront;
	P3DXVector3D		vOrientTop;
	float				flDistanceFactor;
	float				flRolloffFactor;
	float				flDopplerFactor;
};

// struktury pre nastavenie vlastnosti efektov
//---------------------------------

struct P3DFX_CHORUS{
  float	fWetDryMix;
  float	fDepth;
  float	fFeedback;
  float	fFrequency;
  long	lWaveform;
  float	fDelay;
  long	lPhase;
};

struct P3DFX_COMPRESSOR {
  float	fGain;
  float	fAttack;
  float	fRelease;
  float	fThreshold;
  float	fRatio;
  float	fPredelay;
};

struct P3DFX_DISTORTION {
  float	fGain;
  float	fEdge;
  float	fPostEQCenterFrequency;
  float	fPostEQBandwidth;
  float	fPreLowpassCutoff;
};

struct P3DFX_ECHO {
  float	fWetDryMix;
  float	fFeedback;
  float	fLeftDelay;
  float	fRightDelay;
  long	lPanDelay;
};

struct P3DFX_FLANGER {
  float	fWetDryMix;
  float	fDepth;
  float	fFeedback;
  float	fFrequency;
  long	lWaveform;
  float	fDelay;
  long	lPhase;
};

struct P3DFX_GARGLE {
  DWORD	dwRateHz;
  DWORD	dwWaveShape;
};
 

struct P3DFX_I3DL2REVERB{
  long	lRoom;
  long	lRoomHF; 
  float	flRoomRolloffFactor;
  float	flDecayTime;
  float	flDecayHFRatio;
  long	lReflections;
  float	flReflectionsDelay;
  long	lReverb;
  float	flReverbDelay; 
  float	flDiffusion;
  float	flDensity;
  float	flHFReference;
};

struct P3DFX_PARAMEQ {
  float	fCenter;
  float	fBandwidth;
  float	fGain;
};

struct P3DFX_WAVESREVERB {
  float	fInGain; 
  float	fReverbMix;
  float	fReverbTime;
  float	fHighFreqRTRatio;
};


//////////////////////////////////////////////////////////////////////////
//										Effects struktury
//////////////////////////////////////////////////////////////////////////

#define P3D_SOUNDFX_COUNT	9
#define P3D_SOUNDFX_BUFFER_SIZE	48

//const unsigned int FXParamsSizes[P3D_SOUNDFX_COUNT] = {28, 24, 20, 20, 28, 8, 48, 12, 16};

// identifikatory efektov pre SetMasterFX()
// nemenit poradie!
enum SFX_IDENTIFIER {
	SFX_ID_NONE,
	SFX_ID_CHORUS,
	SFX_ID_COMPRESSOR,
	SFX_ID_DISTORTION,
	SFX_ID_ECHO,
	SFX_ID_FLANGER,
	SFX_ID_GARGLE,
	SFX_ID_I3DL2REVERB,
	SFX_ID_PARAMEQ,
	SFX_ID_REVERB
};

struct P3D_SFX_DESC{
	SFX_IDENTIFIER	effectID;
	union {
		P3DFX_CHORUS		chorusParams;
		P3DFX_COMPRESSOR	compressorParams;
		P3DFX_DISTORTION	distortionParams;
		P3DFX_ECHO			echoParams;
		P3DFX_FLANGER		flangerParams;
		P3DFX_GARGLE		gargleParams;
		P3DFX_I3DL2REVERB	reverb2Params;
		P3DFX_PARAMEQ		paramqParams;
		P3DFX_WAVESREVERB	reverbParams;

	};
	P3D_SFX_DESC()
	{
		effectID = SFX_ID_NONE;
		memset (&chorusParams, 0, sizeof(P3D_SOUNDFX_BUFFER_SIZE));
	}
};


//////////////////////////////////////////////////////////////////////////
//											Interfaces
//////////////////////////////////////////////////////////////////////////

class IP3DSoundContainer;

//---------------------------------
class IP3DSoundManager : public IP3DBaseInterface
{
public:
	virtual bool Initialize (HWND hWnd, SoundEventNotifyCallback notifyCallbackFunc = NULL) = 0;
	virtual bool SetPrimaryBufferFormat (SOUND_CHANNELS PrimaryChannels, SOUND_BITS_PER_SAMPLE BitsPerSample, SOUND_SAMPLES_PER_SECOND SamplesPerSec) = 0;

	virtual unsigned int LoadSound (const char *strSoundName, DWORD dwMaxSim = 1) = 0;
	virtual bool UnloadSound (const char *strSoundName) = 0;
	virtual bool UnloadSound (unsigned int dwSoundID) = 0;
	virtual void UnloadAll () = 0;
	virtual bool SoundName2SoundID (IN const char *strSoundName, OUT unsigned int &dwSoudID) = 0;
	virtual bool SoundID2SoundName (IN unsigned int dwSoudID, OUT const char *strSoundName) = 0;
	// playing functions
	virtual P3D_SOUND_HANDLE PlaySound (const char *strSoundName, bool bLoop = false) = 0;
	virtual P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, bool bLoop = false) = 0;
	virtual P3D_SOUND_HANDLE PlaySound (const char *strSoundName, P3DXVector3D &pSoundPos, bool bLoop = false) = 0;
	virtual P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, P3DXVector3D &pSoundPos, bool bLoop = false) = 0;
	virtual P3D_SOUND_HANDLE PlaySound (const char *strSoundName, IP3DSoundContainer *pOwner, bool bLoop = false) = 0;
	virtual P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, IP3DSoundContainer *pOwner, bool bLoop = false) = 0;
	virtual bool StopSound (P3D_SOUND_HANDLE dwSoundHandle) = 0;
	virtual void StopAll (bool bPauseOnly = true) = 0;
	virtual void ResumeAll () = 0;

	virtual bool SetSoundVolume (P3D_SOUND_HANDLE soundHandle, float fVolume = 1.0f) = 0;
	virtual bool SetSoundFXs (P3D_SOUND_HANDLE soundHandle, DWORD dwEffectsNum = 0, P3D_SFX_DESC *pEffects = NULL) = 0;
	// global parameters
	virtual void SetMasterVolume (float fVolume = 1.0f) = 0;
	virtual bool SetMasterFXs (DWORD dwEffectsNum, P3D_SFX_DESC *pEffects) = 0;
	virtual bool Apply3DChanges () = 0;

	// listener functions
	virtual bool Listener_SetAllParameters (P3DLISTENER	&pListenerParams) = 0;
	virtual bool Listener_GetAllParameters (P3DLISTENER	&pListenerParams) = 0;
	virtual bool Listener_SetPositionOrientation (P3DXVector3D &vecNewPosition, P3DXVector3D &vecNewFront) = 0;
	virtual bool Listener_SetVelocity (P3DXVector3D &vecNewVelocity) = 0;
	virtual bool Listener_SetDopplerFactor (float fNewDopplerFactor) = 0;
	virtual bool Listener_SetRolloffFactor (float fNewRolloffFactor) = 0;
	virtual void Update() = 0;
};
#define IP3DSOUND_SOUNDMANAGER "P3DSoundManager_1" // nazev ifacu

//---------------------------------
class IP3DSoundContainer : public IP3DBaseInterface
{
public:
	P3DXVector3D	m_3DPos;				// 3D pozicia sound-kontajnera
	P3DXVector3D	m_3DVelocity;		// rychlost
	bool			m_bUseVelocity;	// true ak sa pouziva aj rychlost

	virtual void RegisterCallbackData (void *callbackData) = 0;

	virtual P3D_SOUND_HANDLE PlaySound (const char *strSoundName, bool bLoop = false) = 0;
	virtual P3D_SOUND_HANDLE PlaySound (unsigned int dwSoundID, bool bLoop = false) = 0;
	virtual bool Stop (P3D_SOUND_HANDLE dwSoundHandle) = 0;
};
#define IP3DSOUND_SOUNDCONTAINER "P3DSoundContainer_1" // nazev ifacu
