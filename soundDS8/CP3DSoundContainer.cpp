
//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DSoundContainer class definition
//-----------------------------------------------------------------------------


#pragma once

#include "CP3DSoundManager.h"

REGISTER_CLASS(CP3DSoundContainer, IP3DSOUND_SOUNDCONTAINER); // zaregistruj tuto tøídu

//---------------------------------
CP3DSoundContainer::CP3DSoundContainer()
{
	m_CallbackData = NULL;
	m_3DPos = P3DXVector3D(0,0,0);
	m_3DVelocity = P3DXVector3D(0,0,0);
	m_bUseVelocity = false;
}

//---------------------------------
CP3DSoundContainer::~CP3DSoundContainer()
{
	if (g_pSoundMan) g_pSoundMan->ContainerDestroyAlert (this);
}

//---------------------------------
bool CP3DSoundContainer::Init ()
{
	return (g_pSoundMan != NULL);
}

// mozne dodat NULL pre zrusenie callback notifikacii
//---------------------------------
void CP3DSoundContainer::RegisterCallbackData (void *callbackData)
{
	m_CallbackData = callbackData;
}

//---------------------------------
void CP3DSoundContainer::FireNotifyCallback (P3D_SOUND_HANDLE soundHandle)
{
	if (m_CallbackData && g_pSoundMan->m_NotifyCallbackFunc)
		g_pSoundMan->m_NotifyCallbackFunc (m_CallbackData, soundHandle);
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundContainer::PlaySound (const char *strSoundName, bool bLoop)
{
	return g_pSoundMan->PlaySound (strSoundName, (IP3DSoundContainer*) this, bLoop);
}

//---------------------------------
P3D_SOUND_HANDLE CP3DSoundContainer::PlaySound (unsigned int dwSoundID, bool bLoop)
{
	return g_pSoundMan->PlaySound (dwSoundID, (IP3DSoundContainer*) this, bLoop);
}

//---------------------------------
bool CP3DSoundContainer::Stop (P3D_SOUND_HANDLE dwSoundHandle)
{
	return g_pSoundMan->StopSound (dwSoundHandle);
}
