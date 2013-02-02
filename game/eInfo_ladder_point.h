//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Ladder point - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"
#include "HUD.h"

#define MOVE_NONE 0 // hrac pouze stoji u nejakeho bodu zebriku
#define MOVE_START 1 // hrac stoji u nejakeho bodu zebriku a stiskl klavesu pouzit
#define MOVE_MOVE 2 // hrac uz stiskl pouzit u nejakeho bodu a uz se od nej vzdalil -> pokud se dostane do nejakeho bodu, tak konec pohybu

#define ACTIVE_LADDER_DISTANCE_HOR	30.0f
#define ACTIVE_LADDER_DISTANCE_VER	10.0f
#define ACTIVE_LADDER_UP_DISTANCE_HOR	45.0f

class eInfo_ladder_point : public eBase
{
public:
	eInfo_ladder_point();
	~eInfo_ladder_point();
	void Finalize();
	bool Spawn(geometryData_s& gdata);
	void Render(float deltaTime);
private:
	P3DVector3D m_vPos;
	int m_nAngle;
	P3DVector3D *m_vPlayerPos;
	EntEvent OnStartMove;
	EntEvent OnEndMove;
	int bUp;
	UINT m_nID;
};