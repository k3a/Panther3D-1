//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Entity manager for spawning, loading, saving, rendering... entities
//-----------------------------------------------------------------------------
// TODO: Pøi save/loadu naèítat a ukládat spawnflagy i origin a rotace
#include "EntityMgr.h"
#include "common.h" // konzole
#include "eBase.h" // konzole

// *********************************************************************************************************************
// PRIKAZY PRO ENTITY (nutno take upravit .h)
ECMD_COMMAND g_entCmds[_EC_NUMCMDS] = {
	{"Enable", 0, 0, "#entities_ecEnable"},
	{"Disable", 0, 0, "#entities_ecDisable"},
	{"SetText", 1, TYPE_STRING, "#entities_ecSetText"},
	{"SetColor", 3, TYPE_INT, "#entities_ecSetColor"},
	{"SetAlpha", 1, TYPE_INT, "#entities_ecSetAlpha"},
	{"Break", 0, 0, "#entities_ecBreak"},
	{"Toggle", 1, TYPE_INT, "#entities_ecToggle"},
	{"SetValue", 1, TYPE_INT, "#entities_ecSetValue"},
	{"Reset", 0, 0, "#entities_ecReset"},
	{"Action", 0, 0, "#entities_ecAction"},
	{"Print", 1, TYPE_STRING, "#entities_ecPrint"},
	{"Command", 1, TYPE_STRING, "#entities_ecCommand"},
	{"Kill", 0, 0, "#entities_ecKill"},
};

// *********************************************************************************************************************
// ZAKL. FUNKCE PRO PRACI S PARAMETRY ENTIT
// pri zmene techto funkci zmenit i v bspmap.cpp
inline int FindIndex(const epair_s *sEnt, const int numKeyP, const char *key)
{
	for (int i=0; i < numKeyP; i++)
		if (strcmpi(sEnt[i].key, key)==0) return i;
	return -1;
}

// ziskavani klicu z entit
inline char *ValueForKey(const epair_s *sEnt, const int numKeyP, const char *key, char *defaultKey)
{
	int keyI = FindIndex(sEnt, numKeyP, key);
	if (keyI>-1) return sEnt[keyI].value;
	return defaultKey;
}
inline float FloatForKey(const epair_s *sEnt, const int numKeyP, const char *key, float defaultKey) 
{
	int keyI = FindIndex(sEnt, numKeyP, key);
	if (keyI>-1) return (float)atof(sEnt[keyI].value);
	return defaultKey; // default
}
inline void VectorForKey(const epair_s *sEnt, const int numKeyP, const char *key, P3DXVector3D *vecOut )
{
	double	v1, v2, v3;
	int keyI = FindIndex(sEnt, numKeyP, key);

	if (keyI<0) return; // chybny klic

	// scanf do double, pak uloz do vektoru
	v1 = v2 = v3 = 0;
	sscanf (sEnt[keyI].value, "%lf %lf %lf", &v1, &v2, &v3);
	vecOut->x = (float)v1;
	vecOut->y = (float)v3; // prehod y za z
	vecOut->z = (float)v2; // prehod z za y
}

// *********************************************************************************************************************
// DRŽÁK JEDNÉ ENTITOVÉ TØÍDY
EntClassHolder* EntClassHolder::g_pLastEntityHolder = NULL;

EntClassHolder::EntClassHolder(CreateEntityFn fnCreateEntity, const char* szEntityClassName, bool bNeedSort) :
m_szEntityClassName(szEntityClassName)
{
	m_pNext = g_pLastEntityHolder; // nastav jako dalsi soucasny posledni holder
	m_fnCreateEntity = fnCreateEntity; // zkopiruj ukazatel na funkci pro vytvoreni tridy
	g_pLastEntityHolder = this; // posledni holder = tento
	//
	m_pFirstEntity = NULL;
	m_bNeedSort = bNeedSort;
}
eBase* EntClassHolder::SpawnNewEntity()
{
	eBase* pEnt = (eBase*)m_fnCreateEntity(); // vytvor novou instanci
	if (!pEnt) return NULL;

	pEnt->m_pHolder=this;

	return pEnt;
}

void VypisEntity(eBase* b, const char* kdeACo)
{
	return;
	char tmp[512];
	char bla[2000]="";
	for (eBase* c=b;c;c=c->GetNext())
	{
		sprintf(tmp, "%d %d | ", c->GetSortKeyMajor(), c->GetSortKeyMinor());
		strcat(bla, tmp);
	}
	CON(MSG_CON_DEBUG, "%s = %s", bla, kdeACo);
}

// *********************************************************************************************************************
// ENTITY MANAGER
REGISTER_SINGLE_CLASS(CP3DEntityMgr, IP3DGAME_ENTITYMGR); // zaregistruj tuto tøídu
bool CP3DEntityMgr::s_renderModels;
bool CP3DEntityMgr::s_renderEntities;
bool CP3DEntityMgr::s_renderHelpers;
bool CP3DEntityMgr::Initialize()
{
	g_pLocaliz = (IP3DLocalization*)I_GetClass(IP3DENGINE_LOCALIZATION);
	if (!g_pLocaliz) return false;

	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
	if (!g_pEngine) return false;

	// preloz lokalizovatelne POPISY PRIKAZU ENTIT
	if (g_pEngine->InEditorMode())
	{
		for (int i=0;i<_EC_NUMCMDS;i++)
			g_entCmds[i].szHelp = g_pLocaliz->Translate_const(g_entCmds[i].szHelp);
	}

	return true;
}

bool CP3DEntityMgr::Spawn(const epair_s *sEnt, const int numEntP, geometryData_s gdata) // vstupni data budou smazana, je nutne je zkopirovat
{
	// nacist prislušnou tridu
	char* clsName = ValueForKey(sEnt, numEntP, "classname", NULL);
	if (!clsName || strlen(clsName)==0) 
	{
		CON(MSG_CON_ERR, "EntityMgr: Entity without a classname key!");
		return false; // neni zadana trida
	}

	// projdi vsechny EntityClassHoldery a pokud se najit holder s timto classname
	eBase* ent=NULL;
	EntClassHolder* entH=NULL;
	for(entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		if (!strcmpi(entH->m_szEntityClassName, clsName))
		{
			// vytvor novou instanci entitove tridy v tomto holderu
			ent = entH->SpawnNewEntity();
			break;
		}
	}
	if (!ent) //bud se nenaslo classname, nebo se nepodarilo vytvorit instanci ent. tridy, spis ale to prvni
	{
		CON(MSG_CON_ERR, "EntityMgr: Can't create entity! Unknown classname '%s'?", clsName);
		return false;
	}

	//nastav zakl. veci a parametry entity
	char* szTargetName = ValueForKey(sEnt, numEntP, "targetname", NULL);
	if (szTargetName) 
		ent->SetTargetName(szTargetName);

	// - spawnflags
	char* szTmp = ValueForKey(sEnt, numEntP, "spawnflags", NULL);
	if (szTmp)
	{
		ent->SetSpawnFlags(atoi(szTmp));
	}

	// - angles
	P3DXVector3D tmpVec; tmpVec.x=0.0f;tmpVec.y=0.0f;tmpVec.z=0.0f;
	float tmpFloat=FloatForKey(sEnt, numEntP, "angle", 0.0f);
	VectorForKey(sEnt, numEntP, "angles", &tmpVec);
	if (tmpFloat!=0.0f && tmpVec.x==0.0f && tmpVec.y==0.0f && tmpVec.z==0.0f) tmpVec.y = tmpFloat; // sestroj angles z angle
	ent->SetWorldRotation(tmpVec);

	// - origin
	tmpVec.x=0.0f;tmpVec.y=0.0f;tmpVec.z=0.0f;
	VectorForKey(sEnt, numEntP, "origin", &tmpVec);
	ent->SetWorldPos(tmpVec);

	// - parent
	szTmp = ValueForKey(sEnt, numEntP, "parent", NULL);
	if (szTmp)
	{
		ent->SetParent(szTmp);
	}

	//nastav cile udalosti
	for (int i=0; i < numEntP; i++)
		if (!strnicmp(sEnt[i].key, "on", 2)) //nalezena udalost
		{
			// odstran cislo z konce, pokud tam je
			char *pszEventName = new char[strlen(sEnt[i].key)+1];
			strcpy(pszEventName, sEnt[i].key);
			if(!IsCharAlpha(pszEventName[strlen(pszEventName)-1])) pszEventName[strlen(pszEventName)-1] = 0;
			if(!IsCharAlpha(pszEventName[strlen(pszEventName)-2])) pszEventName[strlen(pszEventName)-2] = 0;

			//pokus se najit tuto udalost
			EntEvent* pEvent = ent->m_Data.GetEvent(pszEventName);

			delete[] pszEventName;

			if(pEvent) // nalezena 
			{
				// zjisti nazev cile, nazev prikazu a parametr = cile jako string
				char* szVal = ValueForKey(sEnt, numEntP, sEnt[i].key, NULL);
				if(!szVal) continue;

				// rozdel na stringy nazev cile, nazev prikazu a parametr
				char* szTarget = new char[strlen(szVal)+1];
				char* szCmd = new char[strlen(szVal)+1];
				char* szParam = new char[strlen(szVal)+1];
				strcpy(szTarget, ""); strcpy(szCmd, ""); strcpy(szParam, ""); //vynuluj
				bool bTarget=false, bCmd=false, bParam=false; // info o tom, zda uz je string naplnen
				int currIndex=0;
				for(unsigned int i=0;i<=strlen(szVal);i++)
				{
					if (!bTarget)
					{
						if (szVal[i]=='.') { szTarget[currIndex]=0; bTarget=true; currIndex=0; continue; } // pokud je '.', mame cil
						szTarget[currIndex] = szVal[i]; // pripoj na konec
						currIndex++;
					}
					else if (!bCmd)
					{
						if (szVal[i]=='(' || szVal[i]==0) { szCmd[currIndex]=0; bCmd=true; currIndex=0; continue; } // pokud je '(', mame prikaz
						szCmd[currIndex] = szVal[i]; // pripoj na konec
						currIndex++;
					}	
					else if (!bParam)
					{
						if (szVal[i]==')') { szParam[currIndex]=0; bCmd=true; break; } // pokud je ')', mame i parametr = vse :)
						szParam[currIndex] = szVal[i]; // pripoj na konec
						currIndex++;
					}
				}

				// ze stringu ziskej cislo prikazu a parametr jako cislo
				int nCmd=-1;
				//nCmd = GetCmdID(szCmd); // E TODO: odstranit celkove ;)
				for (int ci=0; ci < _EC_NUMCMDS; ci++)
					if (!stricmp(szCmd, g_entCmds[ci].szName))
					{
						nCmd = ci;
						break;
					}

				// pokud nenalezen
				if (nCmd==-1)
				{
					CON(MSG_CON_DEBUG, "EntityMgr: Entity %s (class %s): Call of undefined entity command %s!", ent->GetTargetName(), ent->m_pHolder->m_szEntityClassName, szCmd);
					//uvolni
					delete[] szTarget; 
					delete[] szCmd; 
					delete[] szParam; 
					continue; // dalsi radek
				}

                //pripoj
				pEvent->AddConnection(szTarget, nCmd, szParam);

				//uvolni
				delete[] szTarget; 
				delete[] szCmd; 
				delete[] szParam; 
			}
		}

	//nastav data entity, ktere je mozno nastavit
	for(int i=0;i<ent->m_Data.GetNumVals();i++)
	{
		entVal_t* pVal = ent->m_Data.GetValue(i);
		if (pVal->flags == E_KEY_SAVE || pVal->flags == E_KEY_DONT_SAVE || pVal->flags == E_SPAWNFLAG)
		{
			char* val = ValueForKey(sEnt, numEntP, pVal->name, NULL);
			if (val) 
			{
				switch (pVal->type)
				{
				case TYPE_FLOAT: 
					*pVal->fValue = FloatForKey(sEnt, numEntP, pVal->name, 0.0f); 
					break;
				case TYPE_INT: 
					*pVal->nValue = atoi(ValueForKey(sEnt, numEntP, pVal->name, NULL));
					break;
				case TYPE_STRING:
					{
						// UPD: FIXME: sem to hadze chybu E TODO: co to je ? a vlastne celkove treba vychytat default stringy u klicu entit a tu geometrii
						if(*pVal->szValue && pVal->bStringNeedDelete) SAFE_DELETE_ARRAY(*pVal->szValue);
						pVal->bStringNeedDelete = true;
						char* charV = ValueForKey(sEnt, numEntP, pVal->name, NULL);
						*pVal->szValue = new char[strlen(charV)+1];
						strcpy(*pVal->szValue, charV);
						break;
					}
				case TYPE_VECTOR:
					VectorForKey(sEnt, numEntP, pVal->name, pVal->vValue);
					break;
				// spawnflags
				case TYPE_SF1:
					*pVal->bValue = ent->TestSF(1 << 0);
					break;
				case TYPE_SF2:
					*pVal->bValue = ent->TestSF(1 << 1);
					break;
				case TYPE_SF3:
					*pVal->bValue = ent->TestSF(1 << 2);
					break;
				case TYPE_SF4:
					*pVal->bValue = ent->TestSF(1 << 3);
					break;
				case TYPE_SF5:
					*pVal->bValue = ent->TestSF(1 << 4);
					break;
				case TYPE_SF6:
					*pVal->bValue = ent->TestSF(1 << 5);
					break;
				case TYPE_SF7:
					*pVal->bValue = ent->TestSF(1 << 6);
					break;
				case TYPE_SF8:
					*pVal->bValue = ent->TestSF(1 << 7);
					break;
				}
			}
		}
	}

	//TODO: POSLAT ENTITAM GEOMETRY INFO!!!!
	if(!ent->Spawn(gdata)) // nepodaril se spawn, odstran entitu
	{
		CON(MSG_CON_ERR, "EntityMgr: Can't Spawn() entity with targetname='%s' (class %s)!", ent->GetTargetName(), ent->GetClassName());

		eBase* pPrev = ent->GetPrevious();
		eBase* pNext = ent->GetNext();
		if(pPrev)pPrev->SetNext(pNext);
		if(pNext)pNext->SetPrevious(pPrev);

		if(entH->m_pFirstEntity==ent) entH->m_pFirstEntity=NULL;
		SAFE_DELETE(ent);
		return false;
	}
	
	// uvolnit geometry info
	if(gdata.numFaces>0){
		for(UINT gf=0;gf<gdata.numFaces;gf++)
		{
//			SAFE_DELETE_ARRAY(gdata.faces[gf].verts);
		}
//		SAFE_DELETE_ARRAY(gdata.faces);
	}

	// nyni je mozne entitu zaradit, protoze ve spawn se nastavil sortkey
	// pokud je prvni:
	if (!ent->m_pHolder->m_pFirstEntity) {
		ent->m_pHolder->m_pFirstEntity = ent; 
		ent->SetNext(NULL); 
		ent->SetPrevious(NULL);
		return true;
	}

	if (ent->m_pHolder->m_bNeedSort) // pokud je potreba seradit, spravne zaradit dle sortkey
		for(eBase* entX=ent->m_pHolder->m_pFirstEntity;entX;entX=entX->GetNext())
		{
			if(!entX->GetPrevious()) //pokud nema predchozi, je prvni
			{
				if (ent->GetSortKeyMajor() < entX->GetSortKeyMajor())
				{
					ent->SetNext(entX); // nove nastav jako dalsi tuhle byvalou prvni
					ent->SetPrevious(NULL); // nova entita je prvni - nema predchozi
					entX->SetPrevious(ent); // byvale prvni nastav jako predchozi tuto
					ent->m_pHolder->m_pFirstEntity = ent; // nyni je tato entita prvni entitou v holderu
					return true; // ok, zaradit, hotovo, je uplne prvni
				}
				else if (ent->GetSortKeyMajor() == entX->GetSortKeyMajor() && entX->GetNext() && entX->GetNext()->GetSortKeyMajor() != entX->GetSortKeyMajor()) // nutno porovna minor, major se shoduji
																			// to vyse je zde proto, kdyz bude napr. 1 1 1, tak aby to neudelalo 11 13 11 ale slo o krok dale a pak udela 11 11 13
				{
					if (ent->GetSortKeyMinor() <= entX->GetSortKeyMinor())
					{
						ent->SetNext(entX); // nove nastav jako dalsi aktualni
						ent->SetPrevious(NULL); // nova nema prechozi - je prvni
						entX->SetPrevious(ent); // aktualni nastav jako predchozi novou
						ent->m_pHolder->m_pFirstEntity = ent; // nyni je tato entita prvni entitou v holderu
						VypisEntity(ent->m_pHolder->m_pFirstEntity, "Prvni, major =, minor prvniho <=");
						return true; // ok, zaradit, hotovo, je uplne prvni
					}
					else // patri za nej
					{
						eBase* next = entX->GetNext();
						entX->SetNext(ent);
						ent->SetPrevious(entX);
						ent->SetNext(next);
						VypisEntity(ent->m_pHolder->m_pFirstEntity, "Prvni, major=, minor prvniho >");
						return true;
					}
				}
				
				if (!entX->GetNext()) // je ale zaroven posledni a nepatri pred nej -> patri za!
				{
					entX->SetNext(ent); // posledni ze seznamu dej jako dalsi novou entitu
					ent->SetPrevious(entX); // nove entite dej jako predchozi posledni ze seznamu
					ent->SetNext(NULL); // dalsi uz neni, jsme na konci
					return true; //konec...
				}
				continue; // neni prvni, prvni entita mela mensi sortkes, potreba porovnat s dalsi v seznamu
			}
		
			if(!entX->GetNext()) // konec seznamu, umisti novou entitu na konec
			{
				if (ent->GetSortKeyMajor() < entX->GetSortKeyMajor())
				{
					// umisti pred entX
					eBase* prev = entX->GetPrevious();
					entX->SetPrevious(ent); 
					ent->SetPrevious(prev);
					ent->SetNext(entX);
					prev->SetNext(ent);
					return true;
				}
				else if (ent->GetSortKeyMajor() > entX->GetSortKeyMajor())
				{
					// umisti za entX na uplny konec seznamu
					entX->SetNext(ent);
					ent->SetPrevious(entX);
					ent->SetNext(NULL);
					return true; //konec...
				}
				else // treba porovnat minor
				{
					if (ent->GetSortKeyMinor() <= entX->GetSortKeyMinor())
					{
						// umisti pred entX
						eBase* prev = entX->GetPrevious();
						entX->SetPrevious(ent); 
						ent->SetPrevious(prev);
						ent->SetNext(entX);
						prev->SetNext(ent);
						VypisEntity(ent->m_pHolder->m_pFirstEntity, "Konec, major=, minor <=, davcam pred posledni v seznamu");
						return true;
					}
					else
					{
						// umisti za entX na uplny konec seznamu
						entX->SetNext(ent);
						ent->SetPrevious(entX);
						ent->SetNext(NULL);
						VypisEntity(ent->m_pHolder->m_pFirstEntity, "Konec, major=, minor >, davam na uplny konec");
						return true; //konec...
					}
				}
			}
			
			/*if (entX->GetSortKeyMajor() < ent->GetSortKeyMajor() && 
				ent->GetSortKeyMajor() < entX->GetNext()->GetSortKeyMajor())
			{
				// nastav novou primo za aktualni
				eBase* pNext = entX->GetNext(); // ziskej dalsi z aktualni
				entX->SetNext(ent); // aktualni nastav jako dalsi tuhle novou
				ent->SetPrevious(entX); // tehle nove nastav jako predchozi aktualni
				ent->SetNext(pNext); // nove nastav jako dalsi byvalou dalsi aktualni
				VypisEntity(ent->m_pHolder->m_pFirstEntity, "Major prvni < druhe, );
					return true; // ok, zarazeno
			}*/
			
			if (ent->GetSortKeyMajor() < entX->GetSortKeyMajor())
			{
				// nastav novou primo pred aktualni
				eBase* pPrev = entX->GetPrevious();
				entX->SetPrevious(ent);
				ent->SetPrevious(pPrev);
				ent->SetNext(entX);
				pPrev->SetNext(ent);
				VypisEntity(ent->m_pHolder->m_pFirstEntity, "Major prvni < druhe, vkladam pred aktualni");
				return true; // ok, zarazeno
			}
			else if (entX->GetSortKeyMajor() == ent->GetSortKeyMajor()
				/*|| ent->GetSortKeyMajor() == entX->GetNext()->GetSortKeyMajor()*/) // major sort keys se rovnaji, porovnej minor
			{
				// je potreba porovnat minor u entit se stejnym major
			
				// ziskam major ID
				int keyMajor = ent->GetSortKeyMajor();
				
				eBase* nextEnt = entX->GetPrevious();
				if (!nextEnt) nextEnt = entX;
				
				// je potreba dale projit seznam az do jineho majorKey a nebo do konce a spravne umistit entitu s minor
				for(eBase* entXM=nextEnt;entXM;entXM=entXM->GetNext())
				{
					if(!entXM->GetPrevious()) //pokud nema predchozi, je prvni, FIXME: Je to ok? K tomu asi dojde velmi zridka
					{
						if (keyMajor == entXM->GetSortKeyMajor() &&
							ent->GetSortKeyMinor() <= entXM->GetSortKeyMinor()) //(ale musi mit spravny major!)
						{
							// nastav uplne na zacatek
							ent->SetNext(entXM); // nove nastav jako dalsi tuhle byvalou prvni
							ent->SetPrevious(NULL); // nova entita je prvni - nema predchozi
							entXM->SetPrevious(ent); // byvale prvni nastav jako predchozi tuto
							ent->m_pHolder->m_pFirstEntity = ent; // nyni je tato entita prvni entitou v holderu
							VypisEntity(ent->m_pHolder->m_pFirstEntity, "MINOR, zacatek seznamu, vkladam nazacatek");
							return true; // nastaveno
						}
						if (!entXM->GetNext()) // je ale zaroven posledni a nepatri pred nej -> patri za!
						{
							entXM->SetNext(ent); // posledni ze seznamu dej jako dalsi novou entitu
							ent->SetPrevious(entXM); // nove entite dej jako predchozi posledni ze seznamu
							ent->SetNext(NULL); // dalsi uz neni, jsme na konci
							VypisEntity(ent->m_pHolder->m_pFirstEntity, "MINOR, zacatek seznamu, vkladam hned za prvni");
							return true; //konec...
						}
						continue; // vyresi se v dalsim kroku
					}
				
					if(!entXM->GetNext() && entXM->GetSortKeyMajor() == keyMajor) // konec seznamu, umisti novou entitu na konec
					{
						if (ent->GetSortKeyMinor() <= entXM->GetSortKeyMinor())
						{
							// umisti pred tuto posledni
							eBase* prev = entXM->GetPrevious();
							entXM->SetPrevious(ent);
							ent->SetPrevious(prev);
							ent->SetNext(entXM);
							prev->SetNext(ent);
							VypisEntity(ent->m_pHolder->m_pFirstEntity, "MINOR, konec seznamu, vkladam pred posledni");
							return true;
						}
						else // umisti uplne na konec seznamu
						{
							// umisti za entX na uplny konec seznamu
							entXM->SetNext(ent);
							ent->SetPrevious(entXM);
							ent->SetNext(NULL);
							VypisEntity(ent->m_pHolder->m_pFirstEntity, "MINOR, konec seznamu, davam uplne na konec");
							return true;
						}
					}

					if (keyMajor == entXM->GetSortKeyMajor() &&
						ent->GetSortKeyMinor() <= entXM->GetSortKeyMinor())
					{
						// nastav hned pred aktualni
						eBase* prev = entXM->GetPrevious();
						ent->SetPrevious(prev);
						ent->SetNext(entXM);
						entXM->SetPrevious(ent);
						prev->SetNext(ent);
						VypisEntity(ent->m_pHolder->m_pFirstEntity, "MINOR, ent <= entXM, vkladam pred aktualni");
						return true; // nastaveno
					}
					/*TOHLE TU ASI NEMA BYT, TO SE PROVEDE AZ V DALSIM KROKUelse if (keyMajor == entXM->GetSortKeyMajor() &&
						ent->GetSortKeyMinor() > entXM->GetSortKeyMinor()) // patri za
					{
						// nastav hned za aktualni
						eBase* next = entXM->GetNext();
						ent->SetPrevious(entXM);
						ent->SetNext(next);
						entXM->SetNext(ent);
						next->SetPrevious(ent);
						VypisEntity(ent->m_pHolder->m_pFirstEntity, "MINOR, ent > entXM, vkladam za aktualni");
						return true; // nastaveno
					}*/
				}
			}
		}
	else //neni potreba serazovat
	{
		ent->SetNext(ent->m_pHolder->m_pFirstEntity); // nove dej jako dalsi prvni
		ent->SetPrevious(NULL); // jako predchozi null
		ent->m_pHolder->m_pFirstEntity->SetPrevious(ent); // stare jako predchozi novou
		ent->m_pHolder->m_pFirstEntity = ent; // nastav prvni jako novou
	}

	return true;
}

void CP3DEntityMgr::Finalize()
{
	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			pEnt->BaseFinalize(); // umozni udelat entite prace se vsemi instancemi entit

			// najdi ukaztele na cile udalosti
			for(int i=0; i < pEnt->m_Data.GetNumEvents(); i++)
			{
				EntEvent* pEvent = pEnt->m_Data.GetEvent(i);
				if (pEvent) pEvent->FindTargets();
			}
		}
	}

	// mozna by to slo dat uz i pri finalize ale pro jistotu to provedu az ted kdyby se ve finaliza zmenili pozice
	// zde totiz nastavim entitam jejich relativni pozice k parentu
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			pEnt->RelinkParent();
		}
	}
}

void CP3DEntityMgr::Render(float deltaTime, bool renderModels, bool renderEntities, bool renderHelpers)
{
	Prof(GAME_CP3DEntityMgr__Render);

	s_renderModels=renderModels;
	s_renderEntities=renderEntities;
	s_renderHelpers=renderHelpers;

	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		if (!entH->m_pFirstEntity) continue; // neobsahuje tridy

		if (s_renderEntities) entH->m_pFirstEntity->PreRender(); // pred vykreslovanim
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			if(pEnt->m_bKilled) continue;
			pEnt->BaseRender(deltaTime);
		}
		if (s_renderEntities) entH->m_pFirstEntity->PostRender(); // po vykreslovani
	}
}

void CP3DEntityMgr::Think(float deltaTime)
{
	Prof(GAME_CP3DEntityMgr__Think);
	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			if(pEnt->m_bKilled) continue;
			pEnt->BaseThink(deltaTime); //aktualizace
		}
	}
}

void CP3DEntityMgr::KillAll()
{
	eBase* pLast=NULL;
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		pLast=NULL;
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			SAFE_DELETE(pLast);
			pLast = pEnt;
		}
		SAFE_DELETE(pLast);
		entH->m_pFirstEntity = NULL;
	}
}

eBase* CP3DEntityMgr::FindEntityByTargetname(const char* szTargetName)
{
	if(!szTargetName) return NULL;
	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			if(!stricmp(pEnt->GetTargetName(), szTargetName)) 
			{
				if (pEnt->m_bKilled) continue; // pokud byla za behu smazana, tak jako by nebyla
				return pEnt; // ok, existuje
			}
		}
	}
	return NULL;
}

eBase* CP3DEntityMgr::FindEntityByClassname(const char* szClassName)
{
	if(!szClassName) return NULL;
	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		// vrat prvni z holderu, pokud existuje, jinak nic
		if(!strcmpi(entH->m_szEntityClassName,szClassName))  // pokud je to tento holder
		{
			if (entH->m_pFirstEntity) // pokud je prvni entita
				return entH->m_pFirstEntity;
			else
				return NULL;
		}
			
	}
	return NULL;
}