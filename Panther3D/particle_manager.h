//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Particle manager with particle system and BaseEmitor
//-----------------------------------------------------------------------------
#pragma once

#include "HP3DVector.h"

struct Particle
{
public:
	P3DXVector3D		position;
	P3DXVector3D		oldPos;
	float			lifeTime; // v sekundach, bude se sama odecitat, jakmile je 0, znamena to mrtvou castici

	P3DXVector3D		velocity; // nastaví se pri spawnu, ubírá se dle odporu vzduchu a sklání se dle gravitace
	//int				energy;
	//float			size;
	// color
};


/*#include "particle_pointemitor.h"*/

// nacita, uklada do souboru description, spravuje v sobe umistene particle emitory
class CP3DParticleSystem
{

};

// spravuje particle systemy cele hry, nacita je a destruuje
class CP3DParticleManager
{
/*	void Create();
	void Update();
private:
	CP3DPointEmitor m_pTempEmitor;*/
};