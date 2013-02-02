//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Vertex buffer class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dvertexbuffer.h"
#include "d3dx9.h"

class CP3DVertexBuffer : public IP3DVertexBuffer
{
public:
	CP3DVertexBuffer();
	~CP3DVertexBuffer();
	HRESULT CreateVB(int vertNum, P3DVertexElement *vertDeclaration, UINT nSizeInBytes, bool dynamic = false, bool writeonly = false, bool points = false);
	HRESULT Lock (void** data, unsigned int SizeToLock = 0);
	HRESULT UnLock ();
	void Render(P3DPrimitiveType_t primType, int start, int num);
	void RenderIndexed(P3DPrimitiveType_t primType, int baseVertIndex, unsigned int MinIndex, unsigned int NumVert, unsigned int StartIndex, unsigned int PrimitiveCnt);
	void Release();
	void Use();
private:
	unsigned int m_vertNum;
	unsigned int m_SizeInBytes;
	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	// novinka
	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDeclaration;
	HRESULT GenerateVertexDeclaration(P3DVertexElement *vertDeclaration);
};