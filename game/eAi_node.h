//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (4.7.2006)
// Purpose:	AI node (waypoint) entity
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eAi_node : public eBase
{
public:
	eAi_node();
	~eAi_node();
	bool Spawn(geometryData_s& gdata);
private:
	P3DVector3D m_vPos;
	P3DVector3D m_vRot;
	UINT m_nRadius;
};