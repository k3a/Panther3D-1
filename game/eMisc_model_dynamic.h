//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Physical controlled model. Entity class declaration.
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "eBase.h"

class eMisc_model_dynamic : public eBase
{
public:
	eMisc_model_dynamic();
	~eMisc_model_dynamic();
	bool Spawn(geometryData_s& gdata);
	void Finalize();
//	void TestFunc(float fValue, int intValue, void *pValue);
private:

private:
	char					*m_pszModelPath;
	int						m_bFreeze;
	P3DXVector3D				m_vStartPos;
	P3DXVector3D				m_vStartRot;
	int						m_nPaint;
};