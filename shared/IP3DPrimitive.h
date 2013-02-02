//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Basic primitives interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

class IP3DPrimitive : public IP3DBaseInterface
{
public:
	virtual bool CreateBox(float Width, float Height, float Depth)=0;
	virtual bool CreateSphere(float radius, unsigned int vsegments, unsigned int hsegments)=0;
	virtual bool CreateTeapot()=0;
	virtual void Render()=0;
};

#define IP3DRENDERER_PRIMITIVE "P3DPrimitive_1" // nazev ifacu