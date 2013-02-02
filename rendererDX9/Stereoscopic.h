//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Stereoscopic manager (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dstereoscopic.h"
#include "common.h"

#define STEREOSCOPIC_FVF	(D3DFVF_XYZRHW | D3DFVF_TEX1)

struct STEREOSCOPIC_VERTEX_FORMAT {
	float	x, y, z, rhw;
	float	u, v;
};

STEREOSCOPIC_VERTEX_FORMAT ScreenQuadSTEREOSCOPIC[4] = {	{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
															{-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
															{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
															{-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f}
														};

//---------------------------------
class CP3DStereoscopic : public IP3DStereoscopic
{
private:
	LPDIRECT3DVERTEXDECLARATION9	m_pScreenQuadVertexDecl;
	LPDIRECT3DVERTEXBUFFER9			m_pScreenQuadVertexBuffer;
	LPDIRECT3DSURFACE9				m_pPreviousTarget;
	
	LPDIRECT3DTEXTURE9 m_pTexLeft;
	LPDIRECT3DSURFACE9 m_pSurfLeft;
	
	LPDIRECT3DTEXTURE9 m_pTexRight;
	LPDIRECT3DSURFACE9 m_pSurfRight;
	DWORD m_dwMat;
	
	void SetShaderParameters();
public:
	CP3DStereoscopic();
	~CP3DStereoscopic();
	bool Init();
	void Begin (int eye);
	void End (int eye);
};
