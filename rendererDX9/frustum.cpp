//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Frustum optimalization class definition
//-----------------------------------------------------------------------------

#include "frustum.h"
#include "common.h"

REGISTER_SINGLE_CLASS(CP3DFrustum, IP3DRENDERER_FRUSTUM); // zaregistruj tuto tøídu
REGISTER_CLASS(CP3DFrustum, IP3DRENDERER_FRUSTUM_PRIVATE);

CP3DFrustum::CP3DFrustum()
{
}

CP3DFrustum::~CP3DFrustum()
{
}

void CP3DFrustum::NormalizePlane(float ppf_frustum[6][4], int i_side)
{
	// Here we calculate the magnitude of the normal to the plane (point A B C)
	// Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
	// To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
	float i_magnitude = (float)sqrt( ppf_frustum[i_side][A] * ppf_frustum[i_side][A] + 
									 ppf_frustum[i_side][B] * ppf_frustum[i_side][B] + 
								     ppf_frustum[i_side][C] * ppf_frustum[i_side][C] );

	// Then we divide the plane's values by it's magnitude.
	// This makes it easier to work with.
	ppf_frustum[i_side][A] /= i_magnitude;
	ppf_frustum[i_side][B] /= i_magnitude;
	ppf_frustum[i_side][C] /= i_magnitude;
	ppf_frustum[i_side][D] /= i_magnitude; 
}

void CP3DFrustum::CalculateFrustum (float matView[16], float matProj[16])
{    
	Prof(RENDERER_CP3DFrustum__CalculateFrustum);

	// Get projection matrix
	memcpy(pf_proj, matProj, sizeof(pf_proj));
	// Get modelview matrix
	memcpy(pf_modl, matView, sizeof(pf_proj));

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.

	pf_clip[ 0] = pf_modl[ 0] * pf_proj[ 0] + pf_modl[ 1] * pf_proj[ 4] + pf_modl[ 2] * pf_proj[ 8] + pf_modl[ 3] * pf_proj[12];
	pf_clip[ 1] = pf_modl[ 0] * pf_proj[ 1] + pf_modl[ 1] * pf_proj[ 5] + pf_modl[ 2] * pf_proj[ 9] + pf_modl[ 3] * pf_proj[13];
	pf_clip[ 2] = pf_modl[ 0] * pf_proj[ 2] + pf_modl[ 1] * pf_proj[ 6] + pf_modl[ 2] * pf_proj[10] + pf_modl[ 3] * pf_proj[14];
	pf_clip[ 3] = pf_modl[ 0] * pf_proj[ 3] + pf_modl[ 1] * pf_proj[ 7] + pf_modl[ 2] * pf_proj[11] + pf_modl[ 3] * pf_proj[15];

	pf_clip[ 4] = pf_modl[ 4] * pf_proj[ 0] + pf_modl[ 5] * pf_proj[ 4] + pf_modl[ 6] * pf_proj[ 8] + pf_modl[ 7] * pf_proj[12];
	pf_clip[ 5] = pf_modl[ 4] * pf_proj[ 1] + pf_modl[ 5] * pf_proj[ 5] + pf_modl[ 6] * pf_proj[ 9] + pf_modl[ 7] * pf_proj[13];
	pf_clip[ 6] = pf_modl[ 4] * pf_proj[ 2] + pf_modl[ 5] * pf_proj[ 6] + pf_modl[ 6] * pf_proj[10] + pf_modl[ 7] * pf_proj[14];
	pf_clip[ 7] = pf_modl[ 4] * pf_proj[ 3] + pf_modl[ 5] * pf_proj[ 7] + pf_modl[ 6] * pf_proj[11] + pf_modl[ 7] * pf_proj[15];

	pf_clip[ 8] = pf_modl[ 8] * pf_proj[ 0] + pf_modl[ 9] * pf_proj[ 4] + pf_modl[10] * pf_proj[ 8] + pf_modl[11] * pf_proj[12];
	pf_clip[ 9] = pf_modl[ 8] * pf_proj[ 1] + pf_modl[ 9] * pf_proj[ 5] + pf_modl[10] * pf_proj[ 9] + pf_modl[11] * pf_proj[13];
	pf_clip[10] = pf_modl[ 8] * pf_proj[ 2] + pf_modl[ 9] * pf_proj[ 6] + pf_modl[10] * pf_proj[10] + pf_modl[11] * pf_proj[14];
	pf_clip[11] = pf_modl[ 8] * pf_proj[ 3] + pf_modl[ 9] * pf_proj[ 7] + pf_modl[10] * pf_proj[11] + pf_modl[11] * pf_proj[15];

	pf_clip[12] = pf_modl[12] * pf_proj[ 0] + pf_modl[13] * pf_proj[ 4] + pf_modl[14] * pf_proj[ 8] + pf_modl[15] * pf_proj[12];
	pf_clip[13] = pf_modl[12] * pf_proj[ 1] + pf_modl[13] * pf_proj[ 5] + pf_modl[14] * pf_proj[ 9] + pf_modl[15] * pf_proj[13];
	pf_clip[14] = pf_modl[12] * pf_proj[ 2] + pf_modl[13] * pf_proj[ 6] + pf_modl[14] * pf_proj[10] + pf_modl[15] * pf_proj[14];
	pf_clip[15] = pf_modl[12] * pf_proj[ 3] + pf_modl[13] * pf_proj[ 7] + pf_modl[14] * pf_proj[11] + pf_modl[15] * pf_proj[15];
	
	// Now we actually want to get the sides of the frustum.  To do this we take
	// the clipping planes we received above and extract the sides from them.

	// This will extract the RIGHT side of the frustum
	m_ppfFrustum[RIGHT][A] = pf_clip[ 3] - pf_clip[ 0];
	m_ppfFrustum[RIGHT][B] = pf_clip[ 7] - pf_clip[ 4];
	m_ppfFrustum[RIGHT][C] = pf_clip[11] - pf_clip[ 8];
	m_ppfFrustum[RIGHT][D] = pf_clip[15] - pf_clip[12];

	// Now that we have a normal (A,B,C) and a distance (D) to the plane,
	// we want to normalize that normal and distance.

	// Normalize the RIGHT side
	NormalizePlane(m_ppfFrustum, RIGHT);

	// This will extract the LEFT side of the frustum
	m_ppfFrustum[LEFT][A] = pf_clip[ 3] + pf_clip[ 0];
	m_ppfFrustum[LEFT][B] = pf_clip[ 7] + pf_clip[ 4];
	m_ppfFrustum[LEFT][C] = pf_clip[11] + pf_clip[ 8];
	m_ppfFrustum[LEFT][D] = pf_clip[15] + pf_clip[12];

	// Normalize the LEFT side
	NormalizePlane(m_ppfFrustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	m_ppfFrustum[BOTTOM][A] = pf_clip[ 3] + pf_clip[ 1];
	m_ppfFrustum[BOTTOM][B] = pf_clip[ 7] + pf_clip[ 5];
	m_ppfFrustum[BOTTOM][C] = pf_clip[11] + pf_clip[ 9];
	m_ppfFrustum[BOTTOM][D] = pf_clip[15] + pf_clip[13];

	// Normalize the BOTTOM side
	NormalizePlane(m_ppfFrustum, BOTTOM);

	// This will extract the TOP side of the frustum
	m_ppfFrustum[TOP][A] = pf_clip[ 3] - pf_clip[ 1];
	m_ppfFrustum[TOP][B] = pf_clip[ 7] - pf_clip[ 5];
	m_ppfFrustum[TOP][C] = pf_clip[11] - pf_clip[ 9];
	m_ppfFrustum[TOP][D] = pf_clip[15] - pf_clip[13];

	// Normalize the TOP side
	NormalizePlane(m_ppfFrustum, TOP);

	// This will extract the BACK side of the frustum
	m_ppfFrustum[BACK][A] = pf_clip[ 3] - pf_clip[ 2];
	m_ppfFrustum[BACK][B] = pf_clip[ 7] - pf_clip[ 6];
	m_ppfFrustum[BACK][C] = pf_clip[11] - pf_clip[10];
	m_ppfFrustum[BACK][D] = pf_clip[15] - pf_clip[14];

	// Normalize the BACK side
	NormalizePlane(m_ppfFrustum, BACK);

	// This will extract the FRONT side of the frustum
	m_ppfFrustum[FRONT][A] = pf_clip[ 3] + pf_clip[ 2];
	m_ppfFrustum[FRONT][B] = pf_clip[ 7] + pf_clip[ 6];
	m_ppfFrustum[FRONT][C] = pf_clip[11] + pf_clip[10];
	m_ppfFrustum[FRONT][D] = pf_clip[15] + pf_clip[14];

	// Normalize the FRONT side
	NormalizePlane(m_ppfFrustum, FRONT);
}

bool CP3DFrustum::PointInFrustum( float f_x, float f_y, float f_z )
{
	// Go through all the sides of the frustum
	for(int i = 0; i < 6; i++ )
	{
		// Calculate the plane equation and check if the point is behind a side of the frustum
		if(m_ppfFrustum[i][A] * f_x + m_ppfFrustum[i][B] * f_y + m_ppfFrustum[i][C] * f_z + m_ppfFrustum[i][D] <= 0)
		{
			// The point was behind a side, so it ISN'T in the frustum
			return false;
		}
	}

	// The point was inside of the frustum (In front of ALL the sides of the frustum)
	return true;
}

bool CP3DFrustum::SphereInFrustum( float f_x, float f_y, float f_z, float f_radius )
{
	Prof(RENDERER_CP3DFrustum__SphereInFrustum);
	// Go through all the sides of the frustum
	for(int i = 0; i < 6; i++ )	
	{
		// If the center of the sphere is farther away from the plane than the radius
		if( m_ppfFrustum[i][A] * f_x + m_ppfFrustum[i][B] * f_y + m_ppfFrustum[i][C] * f_z + m_ppfFrustum[i][D] <= -f_radius )
		{
			// The distance was greater than the radius so the sphere is outside of the frustum
			return false;
		}
	}
	
	// The sphere was inside of the frustum!
	return true;
}

bool CP3DFrustum::CubeInFrustum( float f_x, float f_y, float f_z, float f_size )
{
	// Basically, what is going on is, that we are given the center of the cube,
	// and half the length.  Think of it like a radius.  Then we checking each point
	// in the cube and seeing if it is inside the frustum.  If a point is found in front
	// of a side, then we skip to the next side.  If we get to a plane that does NOT have
	// a point in front of it, then it will return false.

	// *Note* - This will sometimes say that a cube is inside the frustum when it isn't.
	// This happens when all the corners of the bounding box are not behind any one plane.
	// This is rare and shouldn't effect the overall rendering speed.

	for(int i = 0; i < 6; i++ )
	{
		if(m_ppfFrustum[i][A] * (f_x - f_size) + m_ppfFrustum[i][B] * (f_y - f_size) + m_ppfFrustum[i][C] * (f_z - f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x + f_size) + m_ppfFrustum[i][B] * (f_y - f_size) + m_ppfFrustum[i][C] * (f_z - f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x - f_size) + m_ppfFrustum[i][B] * (f_y + f_size) + m_ppfFrustum[i][C] * (f_z - f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x + f_size) + m_ppfFrustum[i][B] * (f_y + f_size) + m_ppfFrustum[i][C] * (f_z - f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x - f_size) + m_ppfFrustum[i][B] * (f_y - f_size) + m_ppfFrustum[i][C] * (f_z + f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x + f_size) + m_ppfFrustum[i][B] * (f_y - f_size) + m_ppfFrustum[i][C] * (f_z + f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x - f_size) + m_ppfFrustum[i][B] * (f_y + f_size) + m_ppfFrustum[i][C] * (f_z + f_size) + m_ppfFrustum[i][D] > 0)
		   continue;
		if(m_ppfFrustum[i][A] * (f_x + f_size) + m_ppfFrustum[i][B] * (f_y + f_size) + m_ppfFrustum[i][C] * (f_z + f_size) + m_ppfFrustum[i][D] > 0)
		   continue;

		// If we get here, it isn't in the frustum
		return false;
	}

	// The cube was inside of the frustum!
	return true;
}

bool CP3DFrustum::BoxInFrustum( float f_min_x, float f_min_y, float f_min_z, float f_max_x, float f_max_y, float f_max_z)
{
	Prof(RENDERER_CP3DFrustum__BoxInFrustum);
	// Go through all of the corners of the box and check then again each plane
	// in the frustum.  If all of them are behind one of the planes, then it most
	// like is not in the frustum.
	for(int i = 0; i < 6; i++ )
	{
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;

		// If we get here, it isn't in the frustum
		return false;
	}

	// Return a true for the box being inside of the frustum
	return true;
}