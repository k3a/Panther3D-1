//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Base entity class + Useful functions for an entities
//-----------------------------------------------------------------------------
// TODO: Prepocitavat pozice stredu bounding sphere podle matice
#include "eBase.h"
#include "common.h"

// #####################################################################################################################
// TRIDY

// *********************************************************************************************************************
// Data entity. Kazda entita zdedi jednu instanci z eBase... Umoznuje registrovat private cleny entitove tridy.
EntData::EntData() {pVals=NULL;nVals=0; pEvents=NULL;nEvents=0;}
EntData::~EntData() { 
	for(int i=0;i<nVals;i++){
		delete[] pVals[i].name; 
		if(pVals[i].type == TYPE_STRING && pVals[i].bStringNeedDelete && *pVals[i].szValue)
			SAFE_DELETE_ARRAY(*pVals[i].szValue);
	} 
	delete[] pVals;nVals=0; 
	for(int i=0;i<nEvents;i++){delete[] pEvents[i].pEventName;};delete[] pEvents;nEvents=0;
}

void EntData::AddValue(const char* name, void* val, entType_e type, int flags, const char* helpStr) // *val bude zkopirovano, nezapomenout uvolnit
																		  // (jen pokud se pouzije jinde nez v makru ENT_DATA)
{
	// vytvor nove, vetsi pole a zkopiruj stare
	entVal_t* newVals = new entVal_t[nVals+1];
	for(int i=0;i<nVals;i++)
	{
		newVals[i] = pVals[i];
	}

	char* str = new char[strlen(name)+1];
	strcpy(str, name);

	newVals[nVals].name = str;
	if (g_pEngine->InEditorMode())
		newVals[nVals].szHelp = g_pLocaliz->Translate_const(helpStr);
	else
		newVals[nVals].szHelp = "";

	switch (type)
	{
	case TYPE_STRING:	newVals[nVals].szValue = (char**)val; newVals[nVals].bStringNeedDelete=false; break;
	case TYPE_INT:		newVals[nVals].nValue = (int*)val; break;
	case TYPE_FLOAT:	newVals[nVals].fValue = (float*)val; break;
	case TYPE_VECTOR:   newVals[nVals].vValue = (P3DXVector3D*)val; break;
	case TYPE_POINTER:  newVals[nVals].pValue = (void**)val; break;
	case TYPE_SF1:
	case TYPE_SF2:
	case TYPE_SF3:
	case TYPE_SF4:
	case TYPE_SF5:
	case TYPE_SF6:
	case TYPE_SF7:
	case TYPE_SF8:      newVals[nVals].bValue = (bool*)val; break;
	}

	newVals[nVals].type = type;
	newVals[nVals].flags = flags;

	if(pVals) delete[] pVals;
	pVals = newVals; // pouzij nove

	nVals++;
}
void EntData::AddEvent(EntEvent* pEvent, const char* szName, const char* helpStr)
{
	EntEvent_t* TMPpEvents=new EntEvent_t[nEvents+1];
	for(int i=0; i<=nEvents;i++)
	{
		if (i == nEvents)
		{
			// pridej novy zaznam
			TMPpEvents[i].pEvent = pEvent;
			TMPpEvents[i].pEventName = new char[strlen(szName)+1];
			strcpy(TMPpEvents[i].pEventName, szName);
			if (g_pEngine->InEditorMode())
				TMPpEvents[i].pEventHelpStr = g_pLocaliz->Translate_const(helpStr);
			else
				TMPpEvents[i].pEventHelpStr = "";
		}
		else
		{
			// zkopiruj stary
			TMPpEvents[i].pEvent = pEvents[i].pEvent;
			TMPpEvents[i].pEventName = new char[strlen(pEvents[i].pEventName)+1];
			strcpy(TMPpEvents[i].pEventName, pEvents[i].pEventName);
		}
	}

	// uklid
	for(int i=0;i<nEvents;i++){delete[] pEvents[i].pEventName;}
	delete[] pEvents;

	pEvents = TMPpEvents;
	nEvents++; //ok, navyseno
}

// *********************************************************************************************************************
// Udalost entity, kazda entita muze mit libovolny pocet udalosti... a pak jen zavolat RaiseEvent, kdyz k udalosti dojde
EntEvent::EntEvent(){numTargets=0;};
EntEvent::~EntEvent(){
	if(numTargets>0)
	{
		// uvolni vsechny pole v indexech a navic cele pole
		for (UINT i=0; i<numTargets; i++)
		{
			// uvolni string, jestli je typu string a je nastaven
			for (int n=0;n<nNumParams[i];n++)
				if(g_entCmds[nCmdIDs[i]].type == TYPE_STRING && pParams[i][n].c)
					delete[] pParams[i][n].c;

			if (pParams[i]) delete[] pParams[i];
			if (pszTmpTargets[i]) delete[] pszTmpTargets[i];
		}
		delete[] pszTmpTargets;
		delete[] pParams;

		delete[] nCmdIDs; 
		delete[] pTargets; 
		delete[] nNumParams;
	}
};
void EntEvent::RaiseEvent(eBase* pCaller)
{
	for(UINT i=0;i<numTargets;i++)
	{
		if(!pTargets[i])
		{
			CON(MSG_CON_ERR, "Entity %s (class %s): Can't call %s(?), event target %s doesn't exist!", pCaller->GetTargetName(), pCaller->GetClassName(), g_entCmds[nCmdIDs[i]].szName, pszTmpTargets[i]);
			continue; //pokud neni cil
		}

		if (pTargets[i]->m_bKilled)
		{
			CON(MSG_CON_DEBUG, "Entity %s (class %s): Can't call %s(?), event target %s (class %s) was killed!", pCaller->GetTargetName(), pCaller->GetClassName(), g_entCmds[nCmdIDs[i]].szName, pTargets[i]->GetTargetName(), pTargets[i]->m_pHolder->m_szEntityClassName);
			continue;
		}

		// zavolej prislusnoy prikaz dle id
		if (!pTargets[i]->Command(pCaller, nCmdIDs[i], pParams[i], nNumParams[i])
			&& !pTargets[i]->BaseCommand(pCaller, nCmdIDs[i], pParams[i], nNumParams[i]))
		{
			CON (MSG_CON_DEBUG, "Entity %s (class %s): Failed to call %s.%s(?)! This command isn't implemented or entity returned false!", pCaller->GetTargetName(), pCaller->GetClassName(), pTargets[i]->GetTargetName(), g_entCmds[nCmdIDs[i]].szName);
		}
	}
};
void EntEvent::AddConnection(const char* pszTmpTarget, int nCmdID, const char* szValue)
{
	int currNumParams=0; // akt. pocet parametru
	ECMD_PARAM *currParams = NULL; // pole parametru

	if (szValue[0]!=0) // ma parametry
	{

		// projdi szValue a zjisti kolik ma parametru
		size_t vlen = strlen(szValue);
		currNumParams++; // jeden tady je urcite
		bool bQuotes=false;
		for (UINT vi=0; vi < vlen; vi++) // spocti pocet parametru
		{
			if (szValue[vi]=='\'') { bQuotes=!bQuotes; continue; }
			if (!bQuotes && szValue[vi] == ',' && szValue[vi+1] != 0)
				currNumParams++;
		}

		// vytvor pole parametru
		currParams = new ECMD_PARAM[currNumParams];

		int procParm=0; // aktualne zpracovavany index v currParams
		int lastVI=0;
		char tmpParam[255]=""; // aktualni parametr
		bQuotes=false; // radeji ;)
		for (UINT vi=0; vi <= vlen; vi++) // nastav jednotlive parametry
		{
			if (szValue[vi]=='\'') { bQuotes=!bQuotes; continue; }
			if ((!bQuotes && szValue[vi]==',') || szValue[vi]==0)
			{
				if (szValue[vi-1]=='\'') // verze v pripade uvozovek '
				{
					strncpy(tmpParam, &szValue[lastVI+1], vi-lastVI-2);
					tmpParam[vi-lastVI-2]=0;
				}
				else
				{
					strncpy(tmpParam, &szValue[lastVI], vi-lastVI);
					tmpParam[vi-lastVI]=0;
				}

				// nastav podle typu
				switch (g_entCmds[nCmdID].type)
				{
				case TYPE_INT:
					currParams[procParm].i = atoi(tmpParam);
					break;
				case TYPE_FLOAT:
					currParams[procParm].f = (float)atof(tmpParam);
					break;
				case TYPE_STRING:
					currParams[procParm].c = new char[strlen(tmpParam)+1];
					strcpy(currParams[procParm].c, tmpParam);
					break;
				default:
					CON(MSG_CON_ERR, "EntityMgr: Unknown entity command parameter type %d!", g_entCmds[nCmdID].type);
				}

				lastVI = vi+1;
				procParm++;
			}
		}
	}



	eBase** TMPpTargets = new eBase*[numTargets+1];
	for(UINT i=0;i<=numTargets;i++)
	{
		TMPpTargets[i] = NULL; // zatim, pozdeji pri volani FindTargets() v Finalize() 
							   // se zjisti konkretni ukazatele a smaze  pszTmpTargets
	}
	//
	char** TMPpszTmpTargets = new char*[numTargets+1];
	for(UINT i=0;i<=numTargets;i++)
	{
		if(i==numTargets)
		{
			TMPpszTmpTargets[numTargets] = new char[strlen(pszTmpTarget)+1];
			strcpy(TMPpszTmpTargets[numTargets], pszTmpTarget);
		}
		else
		{
			TMPpszTmpTargets[i] = new char[strlen(pszTmpTargets[i])+1];
			strcpy(TMPpszTmpTargets[i], pszTmpTargets[i]);
		}
	}
	//
	int* TMPnCmdIDs = new int[numTargets+1];
	for(UINT i=0;i<=numTargets;i++)
	{
		if(i==numTargets)
		{
			TMPnCmdIDs[i]=nCmdID;
		}
		else
			TMPnCmdIDs[i] = nCmdIDs[i];
	}
	//
	int* TMPnNumParams = new int[numTargets+1];
	for(UINT i=0;i<=numTargets;i++)
	{
		if(i==numTargets)
		{
			TMPnNumParams[i]=currNumParams;
		}
		else
			TMPnNumParams[i] = nNumParams[i];
	}
	//
	ECMD_PARAM** TMPpParams = new ECMD_PARAM*[numTargets+1];
	for(UINT i=0;i<=numTargets;i++)
	{
		if(i==numTargets)
		{
			TMPpParams[i]=currParams;
		}
		else
			TMPpParams[i] = pParams[i];
	}
	//
	if(numTargets>0)
	{
		delete[]nCmdIDs;
		delete[]pTargets;
		delete[]nNumParams;
		delete[]pParams;
		for(UINT i=0; i<numTargets; i++)
		{
			delete[] pszTmpTargets[i];
		}
		delete[] pszTmpTargets;
	}
	pTargets = TMPpTargets;
	nCmdIDs = TMPnCmdIDs;
	nNumParams = TMPnNumParams;
	pParams = TMPpParams;
	pszTmpTargets = TMPpszTmpTargets;
	//
	numTargets++;
}
void EntEvent::FindTargets()
{
	for(UINT i=0;i<numTargets;i++)
	{
		if (pszTmpTargets[i]) //pokud je cil jako string
		{
			pTargets[i] = g_pEntMgr->FindEntityByTargetname(pszTmpTargets[i]);
			if (pTargets[i]) SAFE_DELETE_ARRAY(pszTmpTargets[i]);
		}
	}
}

// *********************************************************************************************************************
// eBase - zaklad kazde entitove tridy
eBase::eBase():
m_aabb(-15.0f, -15.0f, -15.0f, 15.0f, 15.0f, 15.0f),
m_sphere(15.0f)
{
	InitBaseRenderable(this);
	InitBasePhysObj(this);

	m_nSortKeyMajor=10;
	m_nSortKeyMinor=0;
	m_bKilled=false;
	m_pPrev=NULL;
	m_pNext=NULL;
	m_spawnFlags=0;
	m_pParent=NULL;
	m_szParent[0]=0;
	m_optimType = OPTIM_BSPHERE;
	m_bVisible = true;
	m_entityFlags = 0;
	
	m_pTargetName = new char[10];
	strcpy(m_pTargetName, "<Unnamed>");

	m_matrix.SetIdentityMatrix();

	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);
	g_pBSPMap = (IP3DBSPMap*)I_GetClass(IP3DRENDERER_BSPMAP);
	g_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM);

	m_bSelected=false;
}

void eBase::Kill(eBase* pCaller)
{
	m_bKilled=true; // nastav jako "zabitou za behu"

	if (pCaller==this) // self destruct? :)
	{
		ECON(MSG_CON_DEBUG, "Warning: This entity was self-destructed (it can be runtime fault in scripting)!", pCaller->GetTargetName(), pCaller->GetClassName()); 
		return;
	}

	if (pCaller)
		ECON(MSG_CON_DEBUG, "Warning: This entity was killed by %s (class %s)!", pCaller->GetTargetName(), pCaller->GetClassName()); 
	else
		ECON(MSG_CON_DEBUG, "Warning: This entity was killed by engine or script!"); 
}

bool eBase::BaseCommand(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams)
{
	ECMDLIST_BEGIN
		ECMD_BEGIN(_EC_KILL)
			Kill(pCaller);
		ECMD_END
	ECMDLIST_END
}

void eBase::SetParent(char* szParent){ // meze se volat kdykoliv, ale pak je nutne volat po finalize RelinkParent()
	if (szParent && szParent[0]!=0)
	{
		strcpy(m_szParent, szParent);
	}
	else
	{
		SetMoveType(MOVE_PHYS);
	}
};

void eBase::RelinkParent() // nastavi si znova relativni pozici k parentu
{
	if(!m_pParent)
	{
		// najdi ze stringu
		if (m_szParent[0]!=0)
		{
			m_pParent = g_pEntMgr->FindEntityByTargetname(m_szParent);
			if (!m_pParent) 
			{
				ECON(MSG_CON_ERR, "BaseEntity: Can' link position parent '%s'", m_szParent);
				return;
			}
			// ziskej relativni matici k parentu
			m_pParent->GetWorldMatrix(m_relMatrix);
			m_relMatrix.m_posit3.x -= m_matrix.m_posit3.x;
			m_relMatrix.m_posit3.y -= m_matrix.m_posit3.y;
			m_relMatrix.m_posit3.z -= m_matrix.m_posit3.z;
			
			SetMoveType(MOVE_PARENT);
		}
	}
}

bool eBase::GetWorldPos(OUT P3DXVector3D &outPos)const // vrati true doslo-li od posledniho volani ke zmene pozice
{
	outPos.x = m_matrix._41;
	outPos.y = m_matrix._42;
	outPos.z = m_matrix._43;
	return true;
};

void eBase::SetRelAABB(P3DAABB &aabb)
{
	m_aabb = m_aabb_spawn = m_aabb_spawn = aabb;
}

void eBase::SetRelSphere(P3DSphere &sph)
{
	m_sphere = m_sphere_world = m_sphere_spawn = sph;
}

//////////////////////////////////////// OPTIMALIZACE ////////////////////////////// ----------------------------------

void eBase::RecalculateIntersectClusters ()
{
	m_bVisible = false;

	// vypocet noveho zoznamu kolidujucich clusterov
	if (m_optimType == OPTIM_BSPHERE)
	{
		P3DSphere m_boundsphereActual;

		GetWorldSphere(m_boundsphereActual);
		g_pBSPMap->CalculateIntersectClusters (m_boundsphereActual, m_clusterList);

		for (DWORD i=0; i<m_clusterList.dwNumClusters; i++)
			if (g_pBSPMap->ClusterVisible (m_clusterList.intCluster[i]))
			{m_bVisible = true; return;} // nevyhovuje pro PVS

		if (!g_pFrustum->SphereInFrustum (m_boundsphereActual.pointOrigin.x, m_boundsphereActual.pointOrigin.y, m_boundsphereActual.pointOrigin.z, m_boundsphereActual.fRadius))
		{
			return;		// nevyhovuje pre frustum
		}
	}
	else
	{
		ECON(MSG_CON_DEBUG, "RecalculateIntersectClusters(): AABB Optimization not yet implemented :'(");
	}
}


////////////////////// EDITOR HELPERS - ZATIM JEN NA DEBUG A TEST FIXME: DEBUG: //////////////////// --------------------------------------------------------------

#include "IP3DMaterialManager.h"
IP3DMaterialManager *g_pMaterialMgr=NULL;
void eBase::EditorRender(float deltaTime)
{
	// renderuj aabb
	P3DAABB waabb;
	GetWorldAABB(waabb);

	if (waabb.min.x==0 && waabb.max.x==0) return;

	if(!g_pMaterialMgr) g_pMaterialMgr = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	g_pMaterialMgr->TurnOffShaders();

	if (m_bSelected)
		g_p2D->DrawAABB(waabb, P3DCOLOR_XRGB(255, 255, 0));
	else
		g_p2D->DrawAABB(waabb, P3DCOLOR_XRGB(128, 128, 0));

	/*// projektuj osy do 2D
	P3DXVector3D pntNull, pntX, pntY, pntZ;
	GetWorldPos(pntNull); g_pRenderer->WorldToScreenSpace(pntNull, false);
	pntX.x=1.0f; pntX.y=0.0f; pntX.z=0.0f; g_pRenderer->WorldToScreenSpace(pntX, true); pntX.x+=pntNull.x; pntX.y+=pntNull.y;
	GetWorldPos(pntY); pntY.y+=50.0f; g_pRenderer->WorldToScreenSpace(pntY, true); pntY.x+=pntNull.x; pntY.y+=pntNull.y;
	GetWorldPos(pntZ); pntZ.z+=50.0f; g_pRenderer->WorldToScreenSpace(pntZ, true); pntZ.x+=pntNull.x; pntZ.y+=pntNull.y;
	*/
	// renderuj osy
	/*g_p2D->Line_DrawSimple(pntNull.x, pntNull.y, pntX.x, pntX.y, P3DCOLOR_XRGB(255, 0, 0));*/

	if (m_bSelected) // TODO: EDITOR: FIXME: Tohle se bude renderovat v editoru a pak posouvat osami, zde jen na test
	{
		P3DXVector3D start, end;
		GetWorldPos(start); 
		end=start; end.x+=30.0f; g_p2D->Draw3DLine(start, end, P3DCOLOR_XRGB(255, 0, 0));
		end=start; end.y+=30.0f; g_p2D->Draw3DLine(start, end, P3DCOLOR_XRGB(0, 255, 0));
		end=start; end.z+=30.0f; g_p2D->Draw3DLine(start, end, P3DCOLOR_XRGB(0, 0, 255));
	}

	// nakresli nazev entity nebo classname
	const char* clsName = GetClassName();
	const char* tName = GetTargetName();
	const char* drawName;

	if (tName[0] && tName[0]!='<')
		drawName=tName;
	else
		drawName=clsName;

	P3DXVector3D pntNull;
	GetWorldPos(pntNull); g_pRenderer->WorldToScreenSpace(pntNull, false);
	
	if (pntNull.z > 0 && pntNull.z < 300) 
		g_p2D->DrawText(drawName, (int)pntNull.x, (int)pntNull.y, P3DCOLOR_XRGB(255, 128, 0));
}

bool eBase::EditorTestSelection(P3DRay& sel)const
{
	P3DAABB waabb;
	GetWorldAABB(waabb);
	return Ray2AABBTest(sel, waabb);
}

void eBase::BaseFinalize()
{
	Finalize(); // konkretni entity

	RelinkParent();
	m_aabb_spawn = m_aabb;
	m_aabb_world = m_aabb;
}

void eBase::BaseRender(float deltaTime)
{
	if (m_bVisible)
	{
		if (CP3DEntityMgr::s_renderModels) RenderableRender(deltaTime);
		if (CP3DEntityMgr::s_renderEntities) Render(deltaTime); // konkretni entity
		if (CP3DEntityMgr::s_renderHelpers) EditorRender(deltaTime);
	}
}

void eBase::BaseThink(float deltaTime) // vola se kazdy snimek
{
	/*
	     H_________G MAX     A[min.x,min.y,min.z]
	     /|       /|         B[max.x,min.y,min.z]
	    E--------F |         C[max.x,min.y,max.z]
	    | |      | |         D[min.x,min.y,max.z]
	    | |D_____|_|C        E[min.x,max.y,min.z]
	    |/       |/          F[max.x,max.y,min.z]
	    A--------B           G[max.x,max.y,max.z]
	  MIN                    H[min.x,max.y,max,z]

	   y z
	   |/_x
	*/

	// nejdrive think konkretni entity
	Think(deltaTime);

	bool bmatrixChangedInThisMethod = false;


	/// VYPOCTI NOVOU MATICI DLE RODICE 
	// TODO: pocatecni rotace parentu se nebere v potas... skusit vyresit tim, ze pri
	// ziskavani toho vektrou od parentu pouzit inverzni matici parentu a pokusit se ziskat vektor jakoby pri nulove rotaci parenta?
	// logicky by melo jit preci
	if (m_pParent && m_pParent->WasMatrixChanged())
	{
		P3DXMatrix newMat;
		m_pParent->GetWorldMatrix(newMat);

		P3DXVector3D vec = m_relMatrix.m_posit3;
		vec.TransformAsVector(newMat);
		m_matrix = newMat;
		m_matrix.m_posit3.x -= vec.x;
		m_matrix.m_posit3.y -= vec.y;
		m_matrix.m_posit3.z -= vec.z;
		m_bMatrixChangedPrev = m_bMatrixChanged; // NUTNE
		m_bMatrixChanged = true; // vsak ted jsme ji zmenili ze... :)
		bmatrixChangedInThisMethod = true;

		// pokud ma tahle entita parenta a ma fyzikalni model, nastav natvrdo jeho matici
		SetPhysWorldMatrix(m_matrix);
	}

	// POKUD DOSLO KE ZMENE MATICE, PREPOCTI AABB, SVETLO A CLUSTERY
	if (WasMatrixChanged())
	{
		// VYPOCTI NOVE AABB (pomocny obrazek s min a max nakresen na zacatku metody :-P)
		if (m_entityFlags & EF_UPDATE_AABB)
		{
			P3DXVector3D P[8];
			P[0]=P3DXVector3D(m_aabb_spawn.min.x, m_aabb_spawn.min.y, m_aabb_spawn.min.z); P[0].TransformAsVector(m_matrix);
			P[1]=P3DXVector3D(m_aabb_spawn.max.x, m_aabb_spawn.min.y, m_aabb_spawn.min.z); P[1].TransformAsVector(m_matrix);
			P[2]=P3DXVector3D(m_aabb_spawn.max.x, m_aabb_spawn.min.y, m_aabb_spawn.max.z); P[2].TransformAsVector(m_matrix);
			P[3]=P3DXVector3D(m_aabb_spawn.min.x, m_aabb_spawn.min.y, m_aabb_spawn.max.z); P[3].TransformAsVector(m_matrix);
			P[4]=P3DXVector3D(m_aabb_spawn.min.x, m_aabb_spawn.max.y, m_aabb_spawn.min.z); P[4].TransformAsVector(m_matrix);
			P[5]=P3DXVector3D(m_aabb_spawn.max.x, m_aabb_spawn.max.y, m_aabb_spawn.min.z); P[5].TransformAsVector(m_matrix);
			P[6]=P3DXVector3D(m_aabb_spawn.max.x, m_aabb_spawn.max.y, m_aabb_spawn.max.z); P[6].TransformAsVector(m_matrix);
			P[7]=P3DXVector3D(m_aabb_spawn.min.x, m_aabb_spawn.max.y, m_aabb_spawn.max.z); P[7].TransformAsVector(m_matrix);

			m_aabb.min.x = m_aabb.min.y = m_aabb.min.z = 0.0f;
			m_aabb.max.x = m_aabb.max.y = m_aabb.max.z = 0.0f;

			for (int i=0; i<8; i++)
			{
				if (P[i].x < m_aabb.min.x) m_aabb.min.x = P[i].x;
				if (P[i].y < m_aabb.min.y) m_aabb.min.y = P[i].y;
				if (P[i].z < m_aabb.min.z) m_aabb.min.z = P[i].z;

				if (P[i].x > m_aabb.max.x) m_aabb.max.x = P[i].x;
				if (P[i].y > m_aabb.max.y) m_aabb.max.y = P[i].y;
				if (P[i].z > m_aabb.max.z) m_aabb.max.z = P[i].z;
			}
		}

		// VYPOCTI NOVE SPHERE
		if (m_entityFlags & EF_UPDATE_BSPHERE)
		{
			P3DXVector3D vecTmp(m_sphere_spawn.pointOrigin);
			vecTmp.TransformAsPoint(m_matrix);
			m_sphere.pointOrigin = vecTmp;
		}

		// AABB do world space
		m_aabb_world.min.x = m_aabb.min.x+m_matrix._41; m_aabb_world.min.y = m_aabb.min.y+m_matrix._42; m_aabb_world.min.z = m_aabb.min.z+m_matrix._43;
		m_aabb_world.max.x = m_aabb.max.x+m_matrix._41; m_aabb_world.max.y = m_aabb.max.y+m_matrix._42; m_aabb_world.max.z = m_aabb.max.z+m_matrix._43;
		// bsphere do world space
		m_sphere_world.pointOrigin.x = m_sphere.pointOrigin.x + m_matrix._41;
		m_sphere_world.pointOrigin.y = m_sphere.pointOrigin.y + m_matrix._42;
		m_sphere_world.pointOrigin.z = m_sphere.pointOrigin.z + m_matrix._43;

		RecalculateIntersectClusters();
		RecalculateLightingAtNewPosition();
	}

	// nastavime jakoby se nezmenila pozice matice :) kdyztak ji fyzika nebo neco jineho nastavi
	// pomoci SetWorldMatrix() znova na true...
	if (!bmatrixChangedInThisMethod) 
	{
		m_bMatrixChangedPrev = m_bMatrixChanged; // NUTNE
		m_bMatrixChanged = false;
	}
}