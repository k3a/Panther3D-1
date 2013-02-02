//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Basic primitives class definition
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "primitive.h"

REGISTER_CLASS(CP3DPrimitive, IP3DRENDERER_PRIMITIVE); // zaregistruj tuto tøídu

CP3DPrimitive::CP3DPrimitive()
{
	m_pMesh = NULL;
}

CP3DPrimitive::~CP3DPrimitive()
{
	SAFE_RELEASE(m_pMesh);
}

bool CP3DPrimitive::CreateBox(float Width, float Height, float Depth)
{
	if (FAILED(D3DXCreateBox( g_pD3DDevice, Width, Height, Depth, &m_pMesh, NULL)))
		return false; // chyba
	return true;
}

bool CP3DPrimitive::CreateSphere(float radius, unsigned int vsegments, unsigned int hsegments)
{
	if (FAILED(D3DXCreateSphere( g_pD3DDevice, radius, vsegments, hsegments, &m_pMesh, NULL)))
		return false; // chyba
	return true;
}

bool CP3DPrimitive::CreateTeapot()
{
	if (FAILED(D3DXCreateTeapot( g_pD3DDevice, &m_pMesh, NULL)))
		return false; // chyba
	return true;
}

void CP3DPrimitive::Render()
{
	STAT(STAT_DRAW_CALLS, 1);
	STAT(STAT_DRAW_VERTS, m_pMesh->GetNumVertices());
	m_pMesh->DrawSubset(0);
}