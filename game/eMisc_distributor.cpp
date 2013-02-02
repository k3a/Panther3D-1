//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (22.10.2006)
// Purpose:	Distributor entity (if you fire this entity it fires first, 
//			then next, next, ...)
//-----------------------------------------------------------------------------
#include "eMisc_distributor.h"

REGISTER_ENTITY(eMisc_distributor, "misc_distributor", false, false, false);

eMisc_distributor::eMisc_distributor()
{
	ENT_DATA("akt_item", m_fActItem, TYPE_INT, 1, E_SAVE, "");

	ENT_EVENT("On1Activate", On1Activate, "#entities_eeOnAction");
	ENT_EVENT("On2Activate", On2Activate, "#entities_eeOnAction");
	ENT_EVENT("On3Activate", On3Activate, "#entities_eeOnAction");
	ENT_EVENT("On4Activate", On4Activate, "#entities_eeOnAction");
	ENT_EVENT("On5Activate", On5Activate, "#entities_eeOnAction");
	ENT_EVENT("On6Activate", On6Activate, "#entities_eeOnAction");
	ENT_EVENT("On7Activate", On7Activate, "#entities_eeOnAction");
	ENT_EVENT("On8Activate", On8Activate, "#entities_eeOnAction");
	ENT_EVENT("On9Activate", On9Activate, "#entities_eeOnAction");
	ENT_EVENT("On10Activate", On10Activate, "#entities_eeOnAction");

	ENT_DATA("LOOP", m_bLoop, TYPE_SF1, false, 0, "#entities_sfLoop");
}


void eMisc_distributor::Activate(eBase* pCaller, int nValue)
{
	switch(m_fActItem)
	{
		case 1:
			if (On1Activate.GetNumTargets()>0) 
				On1Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 2:
			if (On2Activate.GetNumTargets()>0) 
				On2Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 3:
			if (On3Activate.GetNumTargets()>0) 
				On3Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 4:
			if (On4Activate.GetNumTargets()>0) 
				On4Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 5:
			if (On5Activate.GetNumTargets()>0) 
				On5Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 6:
			if (On6Activate.GetNumTargets()>0) 
				On6Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 7:
			if (On7Activate.GetNumTargets()>0) 
				On7Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 8:
			if (On8Activate.GetNumTargets()>0) 
				On8Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 9:
			if (On9Activate.GetNumTargets()>0) 
				On9Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
		case 10:
			if (On10Activate.GetNumTargets()>0) 
				On10Activate.RaiseEvent(pCaller);
			else
				if (m_bLoop) m_fActItem=0;
			break;
	}
	
	m_fActItem++;
}