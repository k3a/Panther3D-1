//------------ Copyright © 2005-2006 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Entity manager for spawning, loading, saving, rendering... entities
//-----------------------------------------------------------------------------
#include "EntityMgr.h"
#include "common.h" // konzole

// *********************************************************************************************************************
// DRŽÁK JEDNÉ ENTITOVÉ TØÍDY
EntClassHolder* EntClassHolder::g_pLastEntityHolder = NULL;

EntClassHolder::EntClassHolder(CreateEntityFn fnCreateEntity, const char* szEntityClassName, bool bNeedSort, bool bNeedThink, bool bNeedRender) :
m_szEntityClassName(szEntityClassName)
{
	m_pNext = g_pLastEntityHolder; // nastav jako dalsi soucasny posledni holder
	m_fnCreateEntity = fnCreateEntity; // zkopiruj ukazatel na funkci pro vytvoreni tridy
	g_pLastEntityHolder = this; // posledni holder = tento
	//
	m_pFirstEntity = NULL;
	m_bNeedSort = bNeedSort;
	m_bNeedThink = bNeedThink;
	m_bNeedRender = bNeedRender;
}
eBase* EntClassHolder::SpawnNewEntity()
{
	eBase* pEnt = (eBase*)m_fnCreateEntity(); // vytvor novou instanci
	if (!pEnt) return NULL;

	pEnt->m_pHolder=this;

	return pEnt;
}

// *********************************************************************************************************************
// ENTITY MANAGER
bool CP3DEntityMgr::Spawn(const epair_s *sEnt, const int numEntP, geometryData_s gdata) // vstupni data budou smazana, je nutne je zkopirovat
{
	// nacist prislušnou tridu
	char* clsName = ValueForKey(sEnt, numEntP, "classname", NULL);
	if (!clsName || strlen(clsName)==0) 
	{
		g_pConsole->Message(MSG_CON_ERR, "EntityMgr: Entity without a classname key!");
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
		g_pConsole->Message(MSG_CON_ERR, "EntityMgr: Can't create entity! Unknown classname '%s'?", clsName);
		return false;
	}

	//nastav zakl. veci a parametry entity
	char* szTargetName = ValueForKey(sEnt, numEntP, "targetname", NULL);
	if (szTargetName) 
		ent->SetTargetName(szTargetName);

	//nastav cile udalosti
	for (int i=0; i < numEntP; i++)
		if (!strnicmp(sEnt[i].key, "on", 2)) //nalezena udalost
		{
			// odstran cislo z konce, pokud tam je
			char *pszEventName = new char[strlen(sEnt[i].key)+1];
			strcpy(pszEventName, sEnt[i].key);
			if(!IsCharAlpha(pszEventName[strlen(pszEventName)-1])) pszEventName[strlen(pszEventName)-1] = 0;

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
				char* szChar = new char[2]; szChar[1]=0; // jeden znak
				for(unsigned int i=0;i<strlen(szVal);i++)
				{
					if (!bTarget)
					{
						if (szVal[i]=='.') { bTarget=true; continue; } // pokud je '.', mame cil
						szChar[0] = szVal[i]; // zkopiruj znak
						strcat(szTarget, szChar); // pripoj na konec
					}
					else if (!bCmd)
					{
						if (szVal[i]=='(') { bCmd=true; continue; } // pokud je '(', mame prikaz
						szChar[0] = szVal[i]; // zkopiruj znak
						strcat(szCmd, szChar); // pripoj na konec
					}
					else if (!bParam)
					{
						if (szVal[i]==')') { bCmd=true; break; } // pokud je ')', mame i parametr = vse :)
						szChar[0] = szVal[i]; // zkopiruj znak
						strcat(szParam, szChar); // pripoj na konec
					}
				}

				// ze stringu ziskej cislo prikazu a parametr jako cislo
				int nCmd=0; int nParam=0;
				nCmd = GetCmdID(szCmd);
				if (strlen(szParam)) nParam = atoi(szParam);

                //pripoj
				pEvent->AddConnection(szTarget, nCmd, nParam);

				//uvolni
				delete[] szTarget; 
				delete[] szCmd; 
				delete[] szParam; 
				delete[] szChar;
			}
		}

	//nastav data entity, ktere je mozno nastavit
	for(int i=0;i<ent->m_Data.GetNumVals();i++)
	{
		entVal_t* pVal = ent->m_Data.GetValue(i);
		if (pVal->flags == E_KEY_SAVE || pVal->flags == E_KEY_DONT_SAVE)
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
						if(*pVal->szValue) SAFE_DELETE_ARRAY(*pVal->szValue);
						char* charV = ValueForKey(sEnt, numEntP, pVal->name, NULL);
						*pVal->szValue = new char[strlen(charV)+1];
						strcpy(*pVal->szValue, charV);
						break;
					}
				case TYPE_VECTOR:
					VectorForKey(sEnt, numEntP, pVal->name, pVal->vValue);
					break;
				}
			}
		}
	}

	//TODO: POSLAT ENTITAM GEOMETRY INFO!!!!
	if(!ent->Spawn(gdata)) // nepodaril se spawn, odstran entitu
	{
		if (ent->m_pTargetName)
			g_pConsole->Message(MSG_CON_ERR, "EntityMgr: Can't Spawn() entity with targetname='%s'!", ent->m_pTargetName);
		else
			g_pConsole->Message(MSG_CON_ERR, "EntityMgr: Can't Spawn() '%s' entity!", entH->m_szEntityClassName);

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
			if(!entX->GetNext()) // konec seznamu, umisti novou entitu na konec
			{
				entX->SetNext(ent); // posledni ze seznamu dej jako dalsi novou entitu
				ent->SetPrevious(entX); // nove entite dej jako predchozi posledni ze seznamu
				ent->SetNext(NULL); // dalsi uz neni, jsme na konci
				break; //konec...
			}

			if(!entX->GetPrevious()) //pokud nema predchozi, je prvni
			{
				if (ent->GetSortKeyMajor() <= entX->GetNext()->GetSortKeyMajor())
				{
					eBase* pNext = entX->GetNext(); // ziskej dalsi z ent
					ent->SetNext(pNext); // nove entite nastav tuto dalsi
					ent->SetPrevious(entX); // nove entite nastav jako predchozi ent
					entX->SetNext(ent); // ent nastav jako dalsi tuto novou
					pNext->SetPrevious(ent); // dalsi nastav jako predchozi novou
				}
				break;
			}

			if (entX->GetPrevious()->GetSortKeyMajor() < ent->GetSortKeyMajor() && 
				ent->GetSortKeyMajor() < entX->GetNext()->GetSortKeyMajor())
			{
				eBase* pNext = entX->GetNext(); // ziskej dalsi z ent
				ent->SetNext(pNext); // nove entite nastav tuto dalsi
				ent->SetPrevious(entX); // nove entite nastav jako predchozi ent
				entX->SetNext(ent); // ent nastav jako dalsi tuto novou
				pNext->SetPrevious(ent); // dalsi nastav jako predchozi novou
			}
			else // major sort keys se rovnaji, porovnej minor
			{
				// ziskam major ID - bud prvni dva nebo druhe dva to budou
				int keyMajor = (entX->GetPrevious()->GetSortKeyMajor() == ent->GetSortKeyMajor()) ? ent->GetSortKeyMajor() : entX->GetNext()->GetSortKeyMajor();
				
				bool found=false; // nalezeno a zarazeno?
				
				// je potreba dale projit seznam az do jineho majorKey a nebo do konce a spravne umistit entitu s minor
				for(eBase* entXM=entX;entXM;entXM=entXM->GetNext())
				{
					if (keyMajor != entXM->GetSortKeyMajor() && found) break; // dokonceno...
				
					if(!entXM->GetNext()) // konec seznamu, umisti novou entitu na konec
					{
						entXM->SetNext(ent); // posledni ze seznamu dej jako dalsi novou entitu
						ent->SetPrevious(entXM); // nove entite dej jako predchozi posledni ze seznamu
						ent->SetNext(NULL); // dalsi uz neni, jsme na konci
						break; //konec...
					}

					if(!entXM->GetPrevious()) //pokud nema predchozi, je prvni
					{
						if (ent->GetSortKeyMinor() <= entXM->GetNext()->GetSortKeyMinor())
						{
							eBase* pNext = entXM->GetNext(); // ziskej dalsi z ent
							ent->SetNext(pNext); // nove entite nastav tuto dalsi
							ent->SetPrevious(entXM); // nove entite nastav jako predchozi ent
							entXM->SetNext(ent); // ent nastav jako dalsi tuto novou
							pNext->SetPrevious(ent); // dalsi nastav jako predchozi novou
						}
						break;
					}

					if (entXM->GetPrevious()->GetSortKeyMinor() <= ent->GetSortKeyMinor() && 
						ent->GetSortKeyMinor() < entXM->GetNext()->GetSortKeyMinor())
					{
						eBase* pNext = entXM->GetNext(); // ziskej dalsi z ent
						ent->SetNext(pNext); // nove entite nastav tuto dalsi
						ent->SetPrevious(entXM); // nove entite nastav jako predchozi ent
						entXM->SetNext(ent); // ent nastav jako dalsi tuto novou
						pNext->SetPrevious(ent); // dalsi nastav jako predchozi novou
						found = true; // nalezeno a zarazeno
					}
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
			pEnt->Finalize(); // umozni udelat entite prace se vsemi instancemi entit

			// najdi ukaztele na cile udalosti
			for(int i=0; i < pEnt->m_Data.GetNumEvents(); i++)
			{
				EntEvent* pEvent = pEnt->m_Data.GetEvent(i);
				if (pEvent) pEvent->FindTargets();
			}
		}
	}
}

void CP3DEntityMgr::Render(float deltaTime)
{
	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		if (!entH->m_bNeedRender) continue; // nepotrebuje vykreslovat
		if (!entH->m_pFirstEntity) continue; // neobsahuje tridy
		entH->m_pFirstEntity->PreRender(); // pred vykreslovanim
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			if(pEnt->m_bKilled) continue;
			pEnt->Render(deltaTime); //vykreslovani
		}
		entH->m_pFirstEntity->PostRender(); // po vykreslovani
	}
}

void CP3DEntityMgr::Think(float deltaTime)
{
	// projdi vsechny holdery
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		if (!entH->m_bNeedThink) continue; // nepotrebuje aktualizaci
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			if(pEnt->m_bKilled) continue;
			pEnt->Think(deltaTime); //aktualizace
		}
	}
}

void CP3DEntityMgr::KillAll()
{
	eBase* pLast=NULL;
	for(EntClassHolder* entH=EntClassHolder::g_pLastEntityHolder;entH;entH=entH->m_pNext)
	{
		// projdi vsechny entity v holderu
		for(eBase* pEnt=entH->m_pFirstEntity;pEnt;pEnt=pEnt->GetNext())
		{
			if(pLast)SAFE_DELETE_ARRAY(pLast->m_pTargetName);
			SAFE_DELETE(pLast);
			pLast = pEnt;
		}
		if(pLast)SAFE_DELETE_ARRAY(pLast->m_pTargetName);
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
			if(!pEnt->m_pTargetName) continue;
			if(!stricmp(pEnt->m_pTargetName, szTargetName)) 
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