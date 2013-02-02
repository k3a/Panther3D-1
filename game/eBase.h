//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Base entity class declaration (all entities must inherit this)
//-----------------------------------------------------------------------------
#pragma once
#include "ip3dentitymgr.h"
#include "types.h"
#include "symbols.h"
#include "string"
#include "EntityMgr.h"
#include "HP3DMatrix.h"
#include "eBasePhysObj.h"
#include "eBaseRenderable.h"
#include "ISharedEntity.h"

// *********************************************************************************************************************
// entity flags - mozno pouzit vzdy jen jeden ze seznamu!
#define E_KEY_SAVE 2 //umozni nacteni parametru z .bsp nebo save souboru
#define E_KEY_DONT_SAVE 4 //umozni nacteni pouze z .bsp, neulozi se do save souboru
#define E_SAVE 16 // ulozi se do save souboru, ale nejde ovlivnit nactenim z .bsp
#define E_INTERNAL 256 // pro vnitrni pouziti, ani se nenastavuje z bspmap, ani se neuklada
#define E_SPAWNFLAG 65536 // pouziti jako spawnflag, al type musi byt TYPE_SF1 nebo TYPE_SF2, ...

enum entType_e
{
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_VECTOR,
	TYPE_STRING,
	TYPE_POINTER,
	TYPE_SF1,
	TYPE_SF2,
	TYPE_SF3,
	TYPE_SF4,
	TYPE_SF5,
	TYPE_SF6,
	TYPE_SF7,
	TYPE_SF8
};

/** Sturcture for entity data (parameters) like name, position, interval, color, ... */
struct entVal_t
{
	char* name;
	union{
		int* nValue;
		float* fValue;
		char** szValue;
		P3DXVector3D* vValue;
		void** pValue; // pro libovolny ukazatel, nutno potom deferencovat
		bool* bValue; // hlavne pro spawnflag
	};
	entType_e type;
	int flags;
	const char* szHelp;
	bool bStringNeedDelete; // string need delete? entity data declared using ENT_DATA clear it themselves
};

class EntEvent; // forward declaration
class EntClassHolder;

// *********************************************************************************************************************
// Data entity. Kazda entita zdedi jednu instanci z eBase... Umoznuje registrovat private cleny entitove tridy.
struct EntEvent_t
{
	EntEvent* pEvent;
	char* pEventName;
	const char* pEventHelpStr;
};
class EntData
{
public:
	EntData();
	~EntData();
	void AddValue(const char* name, void* val, entType_e type, int flags, const char* helpStr); // *val bude zkopirovano, nezapomenout uvolnit
																							    // (jen pokud se pouzije jinde nez v makru ENT_DATA)
	void AddEvent(EntEvent* pEvent, const char* szName, const char* helpStr); // pEvent se nesmaze, jen se vezme ukazatel
	int GetNumVals()const {return nVals;};
	entVal_t* GetValue(int index)const { if(index>=nVals)index=nVals-1; return &pVals[index];};
	entVal_t* GetValue(const char* szName){
		for(int i=0;i<nVals;i++)
			if(!strcmpi(pVals[i].name, szName)) return &pVals[i];
		return NULL;
	};
	//
	int GetNumEvents()const {return nEvents;};
	EntEvent* GetEvent(int index)const { if(index>=nEvents)index=nEvents-1; return pEvents[index].pEvent;};
	EntEvent* GetEvent(const char* szEventName) const 
	{
		for(int i=0;i<nEvents;i++)
		{
			if (!strcmpi(pEvents[i].pEventName,szEventName))
			{
				return pEvents[i].pEvent;
			}
		}
		return NULL; //nenalezeno, nacitani entit to asi ignoruje
	};
private:
	entVal_t* pVals; int nVals;
	EntEvent_t* pEvents; int nEvents;
};


// *********************************************************************************************************************

/** Entity render optimization. We predict only there two types, so if you add some new type, search for OPTIM_AABB 
and "else" blocks in code! */
enum eOptimType
{
	/** Use AABB for visibility test */
	OPTIM_AABB=0,
	/** Use bounding sphere for visibility test */
	OPTIM_BSPHERE
};

/** Entity flag for eBase::SetEntityFlag */
#define EF_UPDATE_AABB (1 << 0)
#define EF_UPDATE_BSPHERE (1 << 1)

// *********************************************************************************************************************
// Zakl. iface pro entity
// Entitova trida nahradi nektere metody dle potreby.
// Pro nacitaci veci potreba pouzit Spawn(), konstruktor se pouziva jen na registraci (makro ENT_DATA) a zakl.
// initializaci, nastaveni clenskych hodnot tridy.
// Jakmile bude mit entita Spawn uspesny, zustane v seznamu. Potom, kdyz je treba nejakou entitu za behu ukoncit,
// je potreba davat pozor, zda ji nepouziva jina entita, zda s ni neni nejak spojena, potom je potreba ukoncit i tu
// druhou. Jinak dojde k ukazateli na jiz neexistujici pamet a dojde k divnemu chovani, popr. engine spadne!
// Pokud je potreba entitu "odstranit" za behu, je nutno volat Kill(), tim entita zustane v pameti, ale
// bude se s ni zachazet, jako kdyby neexistovala.
class eBase : public ISharedEntity, public eBaseRenderable, public eBasePhysObj
{
public:
	eBase();
	virtual ~eBase(){
		SAFE_DELETE_ARRAY(m_pTargetName);
		ShutdownBasePhysObj();
	};
	// zakl. metody, ktere ma temer kazda entita
	virtual bool Spawn(geometryData_s& gdata){return true;}; //sem patri veskere nacitaci veci, precaching, ...! Tady se taky vola SetSortKey(). . Provede se pouze poprve, pri nacteni .bsp, u nacitani ulozene hry se zavola pouze Finalize()!!!
	virtual void Finalize(){}; //sem patri ruzne nastavovani pozic, sil, textu, casu - spusti se po nacteni .bsp nebo ulozene hry, jakmile budou vsechny entity nacteny. Nemelo by byt moc narocne, zadne nacitani modelu a tak.
			void BaseFinalize();
	virtual void NewLoad(){}; //sem patri ukonceni instanci a pripraveni na volani Finalize(), vola se pouze pred novym nactenim ulozene hry. Pozor! Nevola se pri ukonceni hry, to je nutno pouzit destruktor!
	virtual void PreRender(){};
	virtual void Render(float deltaTime){};
			void BaseRender(float deltaTime);
	virtual void PostRender(){};
	virtual void Think(float deltaTime){};
			void BaseThink(float deltaTime);
	virtual void Kill(eBase* pCaller); // Mozno pretizit v entitove tride a uvolnit zdroje ;) Ale nutno tam opsat tuhle co je v eBase nebo ji tam nejak zavolat!!!
	virtual void TestFunc(float fValue=0.0f, int intValue=0, void *pValue=NULL){};		// FIXME: debug - odstranit !!!
	// klice, ktere ma mnoho entit
	void SetTargetName(const char* szTName){if(m_pTargetName)SAFE_DELETE_ARRAY(m_pTargetName); m_pTargetName = new char[strlen(szTName)+1]; strcpy(m_pTargetName, szTName);}; // vstupni retezec se zkopiruje, nezapomenout uvolnit...
	const char* GetTargetName()const{return m_pTargetName;}; // bere svou vnitrni promennou, ziskany retezec neuvolnovat!!!
	const char* GetClassName()const{return m_pHolder->m_szEntityClassName;};
	// akce - metody entity - mozno vypisovat hlaseni stylu "Volani na eBase", ale neni to nutne...
	//----------------------------------------------------
	virtual bool Command(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams){return false;};
	bool BaseCommand(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams);
	virtual void Use(eBase* pCaller, int nValue){}; // vola se kdyz hrac nebo npc entitu pouzije
	//----------------------------------------------------
	// prochazeni, trideni a vnitrni metody
	eBase* GetNext()const{return m_pNext;};
	eBase* GetPrevious()const{return m_pPrev;};
	void SetNext(eBase* pEntity){m_pNext=pEntity;};
	void SetPrevious(eBase* pEntity){m_pPrev=pEntity;};
	void SetSortKeys(UINT nMajor, UINT nMinor){m_nSortKeyMajor=nMajor; m_nSortKeyMinor=nMinor;};
	UINT GetSortKeyMajor()const{return m_nSortKeyMajor;};
	UINT GetSortKeyMinor()const{return m_nSortKeyMinor;};
	void SetSpawnFlags(int sf){m_spawnFlags = sf;};
	void SetSpawnFlag(int sf){m_spawnFlags |= sf;};
	int GetSpawnFlags()const{return m_spawnFlags;};
	bool TestSF(int spawn_flag)const{return !!(m_spawnFlags & spawn_flag);}; // na test spawn flagu v entite
	//----------------------------------------------------
	// pohybova hierarchie
	void SetParent(eBase* pParent){ // musi se volat az po tom co parent ma nastavenu pozici a lokalni pozice teto entity je taky nastavena
		m_pParent = pParent;
		RelinkParent();
	};
	void SetParent(char* szParent); // meze se volat kdykoliv, ale pak je nutne volat po finalize RelinkParent()
	void RelinkParent(); // najde parent ze stringu pokud neni a nastavi si znova relativni pozici k parentu
	eBase *GetParent()const{return m_pParent;};
	//----------------------------------------------------
	// pozice a matice
	bool GetWorldPos(OUT P3DXVector3D &outPos)const; // vrati true doslo-li od posledniho volani ke zmene pozice
	void SetWorldPos(P3DXVector3D &inPos){m_matrix._41=inPos.x;m_matrix._42=inPos.y;m_matrix._43=inPos.z;}; // funguje jen pokud je parent NULL (jinak se podle parentu nastavuje pozice sama)
	void SetWorldRotation(P3DXVector3D &vec){m_matrix.RotateZXY(vec.x, vec.y, vec.z);};
	//----------------------------------------------------
	// optimalizace, AABB, ...
	/** Set (raise) specific entity flag. */
	void SetEntityFlag(int flagToRaise){m_entityFlags |= flagToRaise;};
	/** Returns all entity flags */
	int GetEntityFlags()const{ return m_entityFlags;};
	void SetOptimizationType(eOptimType ot){ m_optimType = ot; };
	void SetRelAABB(P3DAABB &aabb);
	inline void GetRelAABB(OUT P3DAABB &aabb)const{ aabb = m_aabb; };
	inline void GetWorldAABB(OUT P3DAABB &aabb)const{ aabb = m_aabb_world; };
	void SetRelSphere(P3DSphere &sph);
	inline void GetRelSphere(OUT P3DSphere &sph)const{ sph = m_sphere; };
	inline void GetWorldSphere(OUT P3DSphere &sph)const{ sph = m_sphere_world; };
	/** Visibility check when entity moves. Internal method, please don't call outside entity system. */
	void RecalculateIntersectClusters();
	/** Is this entity visible by camera? */
	inline bool IsVisible()const{ return m_bVisible; };
	//----------------------------------------------------
	// helpery pro editor
	virtual void EditorRender(float deltaTime);
	virtual bool EditorTestSelection(P3DRay& sel)const;
	virtual void EditorSetSelected(bool bSelected){m_bSelected=bSelected;};
public:
	EntData m_Data;
	EntClassHolder *m_pHolder; // do ktereho patri entita, NEMENIT! 
	bool m_bKilled; // odstranena za behu?
private:
	eBase* m_pPrev;
	eBase* m_pNext;
	UINT m_nSortKeyMajor;
	UINT m_nSortKeyMinor;
	//
	char* m_pTargetName;
	int m_spawnFlags;
	char m_szParent[256];
	//
	bool m_bVisible;
	int m_entityFlags;
	eOptimType m_optimType;
	P3DAABB m_aabb; // aktualni, v local space
	P3DAABB m_aabb_spawn; // pocatecni bez rotace, v local space
	P3DAABB m_aabb_world; // aktualni, ve world space
	P3DSphere m_sphere; // aktualni, v local space
	P3DSphere m_sphere_spawn; // pocatecni bez rotace, v local space
	P3DSphere m_sphere_world; // aktualni, ve world space
	BSPCLUSTER_LIST m_clusterList;
	//
	// EDITOR
	bool m_bSelected;
};


// *********************************************************************************************************************
// Udalost entity, kazda entita muze mit libovolny pocet udalosti... a pak jen zavolat RaiseEvent, kdyz k udalosti dojde
class EntEvent
{
private:
	eBase** pTargets; // pole *
	char** pszTmpTargets; // pole * - docasne, drzi jen jmeno cile
	int* nCmdIDs; // pole 
	ECMD_PARAM** pParams; // pole poli ECMD_PARAM
	int* nNumParams; // pole 
	UINT numTargets;
public:
	EntEvent();
	~EntEvent();
	void RaiseEvent(eBase* pCaller);
	void AddConnection(const char* pTarget, int nCmdID, const char* szValue);
	void FindTargets(); // projde cile a dle nazvu entity ziska na ni ukazatel
	UINT GetNumTargets()const{return numTargets;};
};

// *********************************************************************************************************************
// Makra pro registrace
#define ENT_DATA_VEC(nameStr, memberValue, defaultX, defaultY, defaultZ, flags, helpStr) memberValue##.x=defaultX;##memberValue##.y=defaultY;##memberValue##.z=defaultZ; m_Data.AddValue(nameStr, &##memberValue, TYPE_VECTOR, flags, helpStr) // pouze pro vektory
#define ENT_DATA(nameStr, memberValue, valueType, valueDefault, flags, helpStr) memberValue = valueDefault; m_Data.AddValue(nameStr, &##memberValue, valueType, flags, helpStr) // tam, kde lze pouzit =
#define ENT_EVENT(nameStr, memberEvent, helpStr) m_Data.AddEvent(&##memberEvent,nameStr,helpStr) // nazev se zadava vcetne on na zacatku