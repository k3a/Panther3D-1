//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Frustum optimalization interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

class IP3DFrustum : public IP3DBaseInterface
{
public:
	virtual void NormalizePlane(float ppf_frustum[6][4], int i_side)=0;
	virtual void CalculateFrustum (float matView[16], float matProj[16])=0;
	virtual bool PointInFrustum(float f_x, float f_y, float f_z)=0;
	virtual bool SphereInFrustum(float f_x, float f_y, float f_z, float f_radius)=0;
	virtual bool CubeInFrustum(float f_x, float f_y, float f_z, float f_size)=0;
	virtual bool BoxInFrustum(float f_min_x, float f_min_y, float f_min_z, float f_max_x, float f_max_y, float f_max_z)=0;
};

#define IP3DRENDERER_FRUSTUM "P3DFrustum_1" // nazev ifacu
#define IP3DRENDERER_FRUSTUM_PRIVATE "P3DFrustumPrivate_1" // nazev ifacu
