//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Physical controlled model. Entity class.
//-----------------------------------------------------------------------------
// TODO: UPRAVIT NA NOVY ENT. SYSTEM. ASI JE POTREBA TAKE UKLADAT SILY
//       KTERE NA TELESO PUSOBI, NAPR. KDYZ BUDE TELESO V DOBE SAVU HRY
//       ZROVNA VE VZDUCHU.!!!
//TODO: pozor je potreba spravne nastavit freeze. Kdyz se totiz mapa nacte ze savu a bude ulozeno v savu 0, model
//      zamrzne, prestoze mohla byt hra ulozena v dobe, kdy byl obekt letici a ve vzduchu, pouzit napr. 2 po
//      rozmrazeni...

#include "eMisc_model_dynamic.h"

REGISTER_ENTITY(eMisc_model_dynamic, "misc_model_dynamic", true);

//---------------------------------
eMisc_model_dynamic::eMisc_model_dynamic()
{
	//registrace
	ENT_DATA_VEC("angles", m_vStartRot, 0, 0, 0, E_KEY_DONT_SAVE, "#entities_edAngles");
	ENT_DATA("model", m_pszModelPath, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edModel");
	ENT_DATA("freeze", m_bFreeze, TYPE_INT, 0, E_KEY_SAVE, "#entities_edFreeze");
	ENT_DATA("paint", m_nPaint, TYPE_INT, 0, E_KEY_DONT_SAVE, "#entities_edPaint");
}

//---------------------------------
eMisc_model_dynamic::~eMisc_model_dynamic()
{
	SAFE_DELETE_ARRAY(m_pszModelPath);
}

//---------------------------------
bool eMisc_model_dynamic::Spawn(geometryData_s& gdata)
{
	if (!LoadModel(m_pszModelPath, m_nPaint)) return false;
	if (!CreatePhysModel()) return false;
	return true;
}

void eMisc_model_dynamic::Finalize()
{
}