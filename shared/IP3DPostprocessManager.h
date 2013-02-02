//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	PostProcess manager interface (renderer)
//-----------------------------------------------------------------------------
#pragma once


#include "interface.h"
#include "types.h"


class IP3DPostprocessManager : public IP3DBaseInterface
{
public:
	virtual void Begin () = 0;
	virtual void End () = 0;
	virtual void FlushChain () = 0;
	virtual void ClearCache () = 0;
	virtual bool PreCache (const char *szMaterialName) = 0;
	virtual bool UnCache (const char *szPostEffectName) = 0;
	virtual bool AddEffect (const char *szMaterialName) = 0;
	virtual bool RemoveEffect (const char *szMaterialName) = 0;
};

#define IP3DRENDERER_POSTPROCESSMANAGER "P3DPostProcessManager_1" // nazev ifacu