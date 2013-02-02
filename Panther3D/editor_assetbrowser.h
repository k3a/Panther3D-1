//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Asset Browser Helper
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "ip3deditorassetbrowser.h"

class CP3DEditorAssetBrowser : IP3DEditorAssetBrowser
{
public:
	CP3DEditorAssetBrowser();
	~CP3DEditorAssetBrowser();
	bool Init();
	void DrawTexture(IP3DTexture *pTexture);
private:
	IP3DTexture* m_pRT;
};