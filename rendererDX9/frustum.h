//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Frustum optimalization class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dfrustum.h"
#include "d3dx9.h"

enum FrustumSide
{
	RIGHT, // The RIGHT side of the frustum
	LEFT, // The LEFT	 side of the frustum
	BOTTOM, // The BOTTOM side of the frustum
	TOP, // The TOP side of the frustum
	FRONT, // The FRONT side of the frustum
	BACK // The BACK	side of the frustum
}; 

enum PlaneData
{
	A, // The X value of the plane's normal
	B, // The Y value of the plane's normal
	C, // The Z value of the plane's normal
	D // The distance the plane is from the origin
};

class CP3DFrustum : public IP3DFrustum
{
public:
	CP3DFrustum();
	~CP3DFrustum();
	void NormalizePlane(float ppf_frustum[6][4], int i_side);
	void CalculateFrustum (float matView[16], float matProj[16]);
	bool PointInFrustum(float f_x, float f_y, float f_z);
	bool SphereInFrustum(float f_x, float f_y, float f_z, float f_radius);
	bool CubeInFrustum(float f_x, float f_y, float f_z, float f_size);
	bool BoxInFrustum(float f_min_x, float f_min_y, float f_min_z, float f_max_x, float f_max_y, float f_max_z);
private:
	float m_ppfFrustum[6][4];
	float   pf_proj[16]; // This will hold our projection matrix
	float   pf_modl[16]; // This will hold our modelview matrix
	float   pf_clip[16]; // This will hold the clipping planes
};