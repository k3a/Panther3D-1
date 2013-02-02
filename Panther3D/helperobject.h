//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Helper class declaration (engine)
//-----------------------------------------------------------------------------

#pragma once

#include "..\\shared\\IP3DHelperObject.h"

class CP3DHelperObject : public IP3DHelperObject
{
public:
	bool Init();
	void Loop();
};
