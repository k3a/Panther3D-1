//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Basic primitives class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dprimitive.h"
#include "d3dx9.h"

class CP3DPrimitive : public IP3DPrimitive
{
public:
	CP3DPrimitive();
	~CP3DPrimitive();
	bool CreateBox(float Width, float Height, float Depth);
	bool CreateSphere(float radius, unsigned int vsegments, unsigned int hsegments);
	bool CreateTeapot();
	void Render();
private:
	LPD3DXMESH m_pMesh;
	unsigned long m_pNumV;
};