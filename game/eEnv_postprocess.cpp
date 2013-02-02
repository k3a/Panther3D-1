//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Postprocess entity - entity class
//-----------------------------------------------------------------------------
// TODO: Umoznit pridavat a odstranovat jako string za behu + umoznit aktivovat/deaktivovat vsechny
#include "eEnv_postprocess.h"
#include "IP3DPostprocessManager.h"

static IP3DPostprocessManager *g_pPostMgr=NULL;

REGISTER_ENTITY(eEnv_postprocess, "env_postprocess", false, false, false);

eEnv_postprocess::eEnv_postprocess()
{
	ENT_DATA("p1", m_pszPost[0], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p2", m_pszPost[1], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p3", m_pszPost[2], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p4", m_pszPost[3], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p5", m_pszPost[4], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p6", m_pszPost[5], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p7", m_pszPost[6], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p8", m_pszPost[7], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p9", m_pszPost[8], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
	ENT_DATA("p10", m_pszPost[9], TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edPostprocess");
}

eEnv_postprocess::~eEnv_postprocess()
{
	// TODO: ? KEX: jj mas pravdu. Vyresi novy ent system, tak zkonroluju a podam na wiki info jak se pracuje s string daty entit...
	/*for (int i=0;i<MAX_POSTPROCESS_NUM;i++)
		SAFE_DELETE(m_pszPost[i]);*/

	for (int i=0;i<MAX_POSTPROCESS_NUM;i++)
		if (m_pszPost[i] && m_pszPost[i][0]!=0)
			g_pPostMgr->UnCache(m_pszPost[i]);
}

bool eEnv_postprocess::Spawn(geometryData_s& gdata)
{
	g_pPostMgr = (IP3DPostprocessManager*)I_GetClass(IP3DRENDERER_POSTPROCESSMANAGER);
	if (!g_pPostMgr)
	{
		ECON(MSG_CON_ERR, "Can't obtain postprocess manager!");
		Kill(this);
		return false;
	}

	int numPost=0;
	for (int i=0;i<MAX_POSTPROCESS_NUM;i++)
	{
		if (m_pszPost[i] && m_pszPost[i][0]!=0)
		{
			if (g_pPostMgr->PreCache(m_pszPost[i]))
			{
				ECON(MSG_CON_INFO, "Postprocess %s loaded successfuly.", m_pszPost[i]);
				numPost++;
			}
			else
				ECON(MSG_CON_ERR, "Can't load %s postprocess!", m_pszPost[i]);
		}
	}

	if (!numPost)
	{
		ECON(MSG_CON_ERR, "This entity has 0 loaded postprocesses!");
		Kill(this);
		return false;
	}

	return true;
}

void eEnv_postprocess::Reset(eBase* pCaller, int nValue)
{
	if (!nValue)
	{
		g_pPostMgr->FlushChain();
	}
	else
	{
		if (m_pszPost[nValue-1] && m_pszPost[nValue-1][0]!=0)
		g_pPostMgr->RemoveEffect(m_pszPost[nValue-1]);
	}
}

void eEnv_postprocess::Activate(eBase* pCaller, int nValue)
{
	if (nValue<1) nValue=1;
	if (nValue>MAX_POSTPROCESS_NUM) nValue=MAX_POSTPROCESS_NUM;

	if (m_pszPost[nValue-1] && m_pszPost[nValue-1][0]!=0)
		if(!g_pPostMgr->AddEffect(m_pszPost[nValue-1]))
		{
			ECON(MSG_CON_DEBUG, "Failed to add %s postprocess!", m_pszPost[nValue-1]);
		}
}