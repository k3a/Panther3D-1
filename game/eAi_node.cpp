//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (4.7.2006)
// Purpose:	AI node (waypoint) entity
//-----------------------------------------------------------------------------
#include "eAi_node.h"

eAi_node::eAi_node()
{
	ENT_DATA("radius", m_nRadius, TYPE_INT, 0, E_KEY_DONT_SAVE, "#entities_edRadius");
}

eAi_node::~eAi_node()
{

}

bool eAi_node::Spawn(geometryData_s& gdata)
{
	return true;
}