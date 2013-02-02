//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Target_print (print text to client screen) entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "eBase.h"
#include "ip3dfont.h"
#include "hcountdowntimer.h"

class eTarget_print : public eBase
{
public:
	eTarget_print();
	~eTarget_print();
	bool Spawn(geometryData_s& gdata);
	void Render(float deltaTime);
	void Activate(eBase* pCaller, int nValue);
private:
	char *m_pSText;
	float m_pWait;
	IP3DFont *m_pFont;
	CCountdownTimer m_Tim;
};