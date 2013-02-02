
//------------ Copyright © 2005-2006 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	IP3DRenderableObject interfaces
//-----------------------------------------------------------------------------


#pragma once

#include "interface.h"
#include "IP3DDynlightManager.h"


//---------------------------------
class IP3DRenderableObject : public IP3DBaseInterface {
public:
	virtual void GetBoundSphere (OUT P3DSphere &boundSphere) = 0;
	virtual bool GetModelSortKeys (UINT &SortKeyMajor, UINT &SortKeyMinor) = 0;
	virtual bool Load (const char *szXMLFileName, DWORD dwPainting = 0, P3DPoint3D *StartPosition = NULL) = 0;
	virtual void RecalculateIntersectClusters () = 0;
	virtual void RecalculateLighting () = 0;
	virtual void Loop (P3DMatrix &TransformMatrix, bool bPosChange, bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input) = 0;
	virtual void Render () = 0;
	virtual void RenderInternal() = 0;		// volane iba alpha_managerom
};


#define IP3DENGINE_RENDERABLE_OBJECT "P3DRENDERABLE_2" // nazev ifacu
