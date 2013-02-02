//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (7.7.2006)
// Purpose:	Particle manager and particle systems
//-----------------------------------------------------------------------------
/*#include "particles.h"
// Particle system - obsahuje nekolik emitoru. Emitory lze pridavat. Jakmile uz v emitoru
//					 nebude zadna ziva castice a emitor nebude opakujici, bude sam odstranen
P3DPSystem::~P3DPSystem()
{
	for(BYTE i = 0; i < m_numEmitters; i++)
	{
		SAFE_DELETE(m_pEmitters[i]);
	}
}

bool P3DPSystem::AddEmitter(P3DPEmitter *pEmitter)
{
	if(m_numEmitters==MAX_EMITTERS-1)
	{
		CON(MSG_CON_DEBUG, "Particles: Max number of emitters (%d) exceeded!", MAX_EMITTERS);
		return false;
	}
	m_pEmitters[m_numEmitters] = pEmitter;
	m_pEmitters[m_numEmitters].m_pSystem = this;
	return true;
}

void P3DPSystem::UpdateAllEmitters(float fDeltaTime)
{
	for(BYTE i = 0; i < m_numEmitters; i++)
	{
		if(m_pEmitters[i])
			m_pEmitters[i]->Update(fDeltaTime);
	}
}

void P3DPSystem::RenderAllEmitters()
{
	for(BYTE i = 0; i < m_numEmitters; i++)
	{
		if(m_pEmitters[i])
			m_pEmitters[i]->Render();
	}
}

// Particle emitter - obsahuje urcity pocet castic. Castice se mohou ruzne premistovat v zavislosti
//					  jejich vlastnostech. Emitovat se mohou jen mrtve castice. 
//					  Pole ktere se nastavi, se samy v destruktoru uvolni.

	Particle *pParticles; // pole castic
	// nastaveni emitoru - grafy maji x rovnu delce emitoru
	int nType; // 0 - ctvercovy typ, 1 - koule
	P3DXVector3D pntMin; // min hodnoty BB nebo x jako polomer koule (pokud je typ koule)
	P3DXVector3D pntMax; // max hodnoty BB
	float fLen; // delka emitoru v sekundach
	bool bRepeat; // opakujici se emitor?
	P3DXVector3D *pvDirAndVelocity; // pole (graf) - smer a rychlost castice
	int nDensity; // pocet castic emitovanych za 1 sekundu (pozdeji graf?)
	float *pfLifeTime; // pole (graf) - zivotnost prave emotovane castice
	float *pfSizeMult; // pole (graf) - nasobitel velikosti prave emitovane castice
	int nParticles; // max. pocet castic
	// nastaveni castic - grafy maji x jako lifetime a ten muze byt ruzny dle nastaveni emitoru
	float fAirResistance; // odpor vzduchu, zpomaleni o m/s za kazdou sekundu - odpocitava se rychlost u kazde castice
	IP3DTexture *pTexture; // textura castice
	float *pfSize; // pole (graf) - pomer veliksoti castice
	float *pfRotation; // pole (graf) - rotace castice kolem osy s tim ze osa miri od stredu castice ke kamere
	int *nfTransp; // pole (graf) - hodnota pruhlednosti
	P3DXVector3D *ppntColor; // pole (graf) - x, y, z jako RGB
	bool bStretch; // "roztahnout" castici mezi jeji posledni a aktualni bod? Tvorba jisker

void P3DPEmitter::P3DPEmitter()
{
	isAlive = true;
}

void P3DPEmitter::~P3DPEmitter()
{
	// uvolni vsechna alokovana pole.
}

bool P3DPEmitter::Create(IP3DTexture* pTex)
{
	if (!pTex)
	{
		CON(MSG_CON_DEBUG, "Particles: Invalid texture (pTex=NULL)!");
		return false;
	}
	m_pTexture = pTex;
	// alokuj vertex a index buffery
}

void P3DPEmitter::Update(float fDeltaTime)
{
	
}

void P3DPEmitter::Render(float fDeltaTime)
{

}*/