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
#pragma once

#include "particle_manager.h"
#include "IP3DTexture.h"
#include "HP3DMatrix.h"
#include "IP3DSpriteBunch.h"

enum ParticleSpawnType
{
	PARTICLE_WAIT=0,
	PARTICLE_REPLACE_RANDOM,
	PARTICLE_REPLACE_OLDEST
};

enum ParticleMoveType
{
	PARTICLE_MOVE_HARD,
	PARTICLE_MOVE_PHYSICS,
};

enum ParticleEmissionType
{
	PARTICLE_EMISSION_POINT=0,
	PARTICLE_EMISSION_BOX,
	PARTICLE_EMISSION_SPHERE,
	PARTICLE_EMISSION_RING
};

struct ParticleEmissionPoint
{
	P3DXVector3D vDirVelocity; // pocatecni smer a rychlost pohybu
};

//------------------------------------------------
class CP3DPointEmitor
{
public:
	CP3DPointEmitor(){ m_fEmissionVelocity=1.0f; m_fLifeTime=1.0f; m_fLifeTimeDev=0.0f; m_matrix.SetIdentityMatrix(); m_fGravity=9.8f; m_fAirResistance=0; m_nNumParticles=0; m_pParticles=NULL; m_fTimeRemain=0; m_bLoaded=false; m_pTexture=NULL; m_bLight=false; m_spawnType=PARTICLE_REPLACE_RANDOM; m_nMaxParticles=100; m_killType=0; m_emission_type=PARTICLE_EMISSION_POINT;};
	~CP3DPointEmitor();
	void SetTexture(IP3DTexture* pTex) { bOwnTexture=false; m_pTexture = pTex; };
	void SetTexture(const char* strTexturePath);
	IP3DTexture *GetTexture()const{ return m_pTexture; };
	void UnloadTexture() { if(m_pTexture) {delete m_pTexture; bOwnTexture=false;} };
	void SetGravity(float fGravity) { m_fGravity=fGravity; };
	void SetAirResistance(float fAR) { if (fAR>0) m_fAirResistance=fAR; };
	void Update(); // aktualizuje emitor
	void Render();
	void EnableLighting(bool bEnable) { m_bLight = bEnable; } ;
	void SetMatrix(P3DMatrix mat) { m_matrix = mat; };
	void SetMaxParticleNum(unsigned int particleNum);
	unsigned int GetMaxParticleNum()const{ return m_nMaxParticles;};
	unsigned int GetNumParticles()const{ return m_nNumParticles; };
	/** Determines what to do, when all the particles */
	void SetParticleSpawnType(ParticleSpawnType pst) { m_spawnType=pst; };
	void SetParticleLifeTime(float lifeTime, float fDeviation=0.0f) { m_fLifeTime=lifeTime; m_fLifeTimeDev=fDeviation;};
	/** Returns actual particle system bounding box in world space. Useful for oprimizations. */
	void GetAABB(P3DAABB &aabb) { aabb=m_aabb; };
	/** Returns actual particle system bounding radius. Useful for oprimizations. */
	float GetRadius()const{ return m_fRadius; };
	void SetKillAABB(P3DAABB &aabb){ m_killType=1; m_aabb_kill=aabb; };
	void SetKillRadius(float fRadius){ m_killType=2; m_radius_kill=fRadius; };
	void SetKillNone(){ m_killType=0; };
	void SetMoveType(ParticleMoveType mt) { m_moveType=mt; };
	void SetEmissionTypePoint(ParticleEmissionPoint ep) { m_emission_point=ep; m_emission_type=PARTICLE_EMISSION_POINT; };
	void SetEmissionVelocity(float fEmissionVelocity) { m_fEmissionVelocity=fEmissionVelocity; };
	void SetEmissionRate(int nSpawnPerSecond){ m_fEmissionRate=1.0f/nSpawnPerSecond; };
	void SetEmissionDeviation(P3DXVector3D &dev){ m_vEmissionDeviation=dev; };
	bool CreateEmitor();
private:
	inline void GetNewSpawnData(Particle &part);

private:
	// EMITOR DATA
	SHADER m_hShader;
	bool m_bLoaded;
	float m_fTimeRemain; // time remained from previous update
	IP3DTexture* m_pTexture;
	bool bOwnTexture; // urcuje zda texuru vytvoril tento emitor nebo mu byla dodana
	P3DXMatrix m_matrix;
	P3DAABB m_aabb;
	float m_fRadius;
	bool m_bLight;

	P3DAABB m_aabb_kill;
	float m_radius_kill;
	char m_killType; // 0-don't kill by volume, 1-kill by AABB, 2-kill by radius

	// SPAWN DATA
	ParticleSpawnType m_spawnType;
	ParticleMoveType m_moveType;

	float m_fGravity; // ubytek predchozi rychlosti za sekundu (pri 1 za sekundu ztrati rychlost)
	float m_fAirResistance; // deleni rychlosti za sekundu

	float m_fLifeTime;
	float m_fLifeTimeDev; // deviation +-

	float m_fEmissionVelocity; // nasobitel te u emission
	float m_fEmissionRate; // new spawn every x miliseconds
	P3DXVector3D m_vEmissionDeviation; // rozptyl oproti smeru emise
	
	ParticleEmissionType m_emission_type;
	union
	{
		ParticleEmissionPoint m_emission_point;
	};

	// PARTICLE DATA
	unsigned int m_nNumParticles;
	unsigned int m_nMaxParticles;
	Particle *m_pParticles; // pole
	P3DVector4D *m_pPosSize; // pole
};