//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Point Particle Emitor
//-----------------------------------------------------------------------------
#include "particle_pointemitor.h"
#include "common.h"
#include "IP3DPrimitive.h"

static IP3DSpriteBunch *s_pSpriteBunch=NULL;

CP3DPointEmitor::~CP3DPointEmitor()
{
	if(bOwnTexture && m_pTexture) delete m_pTexture; 

	if (m_pParticles) delete[] m_pParticles;
	if (m_pPosSize) delete[] m_pPosSize;
}

bool CP3DPointEmitor::CreateEmitor()
{
	m_bLoaded = false;

	// zkontroluj zda je vse nastavene
	if (!m_pTexture)
	{
		CON(MSG_CON_ERR, "Cannot create emitor without texture!");
		return false;
	}

	if(!m_pParticles)
	{
		CON(MSG_CON_ERR, "Cannot create emitor without particles! You must call SetMaxParticleNum() before...");
		return false;
	}

	s_pSpriteBunch = (IP3DSpriteBunch*)I_GetClass(IP3DRENDERER_SPRITEBUNCH);
	m_hShader = s_pSpriteBunch->AddShader("spec_particles.fx");

	m_bLoaded = true;
	return true;
}

void CP3DPointEmitor::Update()
{
	if (!m_bLoaded) return;

	m_fTimeRemain += GetDeltaTime();

	// nutny novy spawn?
	if (m_fTimeRemain > m_fEmissionRate)
	{
		// kolik spawnu je treba?
		int numSpawn = (int)floorf(m_fTimeRemain / m_fEmissionRate);
		m_fTimeRemain -= m_fEmissionRate*numSpawn;

		for (int iS=0; iS<numSpawn; iS++) // proved novy spawn
		{
			int index = -1;
			// vyres pripad pokud je plno
			if (m_nNumParticles==m_nMaxParticles)
			{
				switch(m_spawnType)
				{
				case PARTICLE_WAIT:
					return;
				case PARTICLE_REPLACE_RANDOM:
					index = randi(m_nMaxParticles-1);
					break;
				case PARTICLE_REPLACE_OLDEST:
					float maxLifeTime=0; int index;
					for (unsigned int iP=0; iP<m_nMaxParticles; iP++)
						if (m_pParticles[iP].lifeTime>maxLifeTime)
						{
							maxLifeTime=m_pParticles[iP].lifeTime;
							index = iP;
						}
					break;
				}
			}

			// jeste mame volno, najdi volnou castici
			if (index==-1)
			{
				for (unsigned int iP=0; iP<m_nMaxParticles; iP++)
					if (m_pParticles[iP].lifeTime==0)
					{
						index = iP;
						m_nNumParticles++;
						break;
					}
			}

			// ziskej novou pozici a nastav pocatecni vlastnosti
			GetNewSpawnData(m_pParticles[index]);
		}
	}

	// aktualizace dat
	m_aabb.min.x = m_aabb.min.y = m_aabb.min.z = 0.0f; m_aabb.max = m_aabb.min;
	for (unsigned int iP=0; iP<m_nMaxParticles; iP++)
	{
		if (m_pParticles[iP].lifeTime==0) continue; // castice se nepouziva

		m_pParticles[iP].lifeTime -= GetDeltaTime();

		// zemrela? odstran
		if (m_pParticles[iP].lifeTime<0)
		{
			m_pParticles[iP].lifeTime=0;
			m_nNumParticles--;
		}

		m_pParticles[iP].velocity.x -= m_pParticles[iP].velocity.x * m_fAirResistance*GetDeltaTime();
		m_pParticles[iP].velocity.y -= m_pParticles[iP].velocity.y * m_fAirResistance*GetDeltaTime();
		m_pParticles[iP].velocity.z -= m_pParticles[iP].velocity.z * m_fAirResistance*GetDeltaTime();

		m_pParticles[iP].velocity.y -= m_fGravity*GetDeltaTime();

		m_pParticles[iP].oldPos = m_pParticles[iP].position;
		m_pParticles[iP].position += m_pParticles[iP].velocity*GetDeltaTime();

		//m_pParticles[iP].position.y -= m_fGravity*GetDeltaTime();

		// AKTUALIZUJ AABB A BSPHERE
		if (m_pParticles[iP].position.x < m_aabb.min.x) m_aabb.min.x = m_pParticles[iP].position.x;
		if (m_pParticles[iP].position.y < m_aabb.min.y) m_aabb.min.y = m_pParticles[iP].position.y;
		if (m_pParticles[iP].position.z < m_aabb.min.z) m_aabb.min.z = m_pParticles[iP].position.z;
		if (m_pParticles[iP].position.x > m_aabb.max.x) m_aabb.max.x = m_pParticles[iP].position.x;
		if (m_pParticles[iP].position.y > m_aabb.max.y) m_aabb.max.y = m_pParticles[iP].position.y;
		if (m_pParticles[iP].position.z > m_aabb.max.z) m_aabb.max.z = m_pParticles[iP].position.z;
	}
	// vytvor kruznici opsanou AABB
	P3DXVector3D vLength = m_aabb.max;
	vLength -= m_matrix.m_posit3;
	m_fRadius = vLength.Length();
}

void CP3DPointEmitor::GetNewSpawnData(Particle &part)
{
	switch(m_emission_type)
	{
	case PARTICLE_EMISSION_POINT:
		float dev=0; // deviance
		// pozice
		part.position = m_matrix.m_posit3;

		// rychlost
		part.velocity = m_emission_point.vDirVelocity;

		if (m_vEmissionDeviation.x>0.0f) part.velocity.x += randf(-m_vEmissionDeviation.x, m_vEmissionDeviation.x);
		if (m_vEmissionDeviation.y>0.0f) part.velocity.y += randf(-m_vEmissionDeviation.y, m_vEmissionDeviation.y);
		if (m_vEmissionDeviation.z>0.0f) part.velocity.z += randf(-m_vEmissionDeviation.z, m_vEmissionDeviation.z);

		part.velocity.x *= m_fEmissionVelocity;
		part.velocity.y *= m_fEmissionVelocity;
		part.velocity.z *= m_fEmissionVelocity;

		// zitovnost
		if (m_fLifeTimeDev>0.0f) dev=randf(-m_fLifeTimeDev, m_fLifeTimeDev); else dev=0.0f;
		part.lifeTime = m_fLifeTime+dev;
		break;
	}

	part.oldPos = part.position;
}

void CP3DPointEmitor::Render()
{
	if (!m_bLoaded) return;

	int index=0;

	//g_pRenderer->EnableLighting(true);// DEBUG!!!!
	for (unsigned int iP=0; iP<m_nNumParticles; iP++)
	{
		if (m_pParticles[iP].lifeTime==0) continue;

		m_pPosSize[index].x = m_pParticles[iP].position.x;
		m_pPosSize[index].y = m_pParticles[iP].position.y;
		m_pPosSize[index].z = m_pParticles[iP].position.z;
		m_pPosSize[index].w = 1.0f; // TODO: SIZE

		index++;
	}
	//g_pRenderer->EnableLighting(false);// DEBUG!!!!

	int currIndex=0, currRendered=0;
	do 
	{
		currRendered = index>128?128:index;
		s_pSpriteBunch->Render(index>128?128:index, m_pTexture, m_hShader, &m_pPosSize[currIndex], NULL );
		currIndex+=currRendered;
	} while(index>currIndex);


	// DEBUG RENDERING - TEXT
	char str[512];
	sprintf(str, "Num. particles: %d / %d", m_nNumParticles, m_nMaxParticles);
	P3DXVector3D vTextPos=m_matrix.m_posit3;
	g_pRenderer->WorldToScreenSpace(vTextPos, false);
	if (vTextPos.z<500 && vTextPos.z>0) g_p2D->DrawText(str, (int)vTextPos.x, (int)vTextPos.y, P3DCOLOR_AQUAMARINE);

	// DEBUG RENDERING - AABB
	g_p2D->DrawAABB(m_aabb, P3DCOLOR_AQUAMARINE);
}

void CP3DPointEmitor::SetMaxParticleNum(unsigned int particleNum) {
	m_nMaxParticles = particleNum; 

	if (m_pParticles) delete[] m_pParticles;
	m_pParticles = new Particle[m_nMaxParticles];

	if (m_pPosSize) delete[] m_pPosSize;
	m_pPosSize = new P3DVector4D[m_nMaxParticles];

	for (unsigned int iP=0; iP<m_nMaxParticles; iP++)
		m_pParticles[iP].lifeTime = 0.0f;
}

void CP3DPointEmitor::SetTexture(const char* strTexturePath)
{
	bOwnTexture = true;
	m_pTexture = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE);
	if (m_pTexture) m_pTexture->Load(strTexturePath, TT_2D);
}