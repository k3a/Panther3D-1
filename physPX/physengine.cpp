//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics core engine
//-----------------------------------------------------------------------------
#include "physengine.h"
#include "debug_renderable.h"
#include "ISharedEntity.h"
#include "ip3dengine.h"

REGISTER_SINGLE_CLASS(CP3DPhysEngine, IP3DPHYS_PHYSENGINE); // zaregistruj tuto tøídu

// ------

ConVar CVphys_debug("phys_debug", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Render physics debug data");
ConVar CVphys_dbg_pos("phys_dbg_pos", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Visualize body axes");
ConVar CVphys_dbg_vel("phys_dbg_vel", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Visualize velocity");
ConVar CVphys_dbg_shp("phys_dbg_shp", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Visualize collision shape");
ConVar CVphys_dbg_com("phys_dbg_com", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Visualize centre of mass");
ConVar CVphys_dbg_vrd("phys_dbg_vrd", "localhost", CVAR_ARCHIVE, "Visual Remote Debugger hostname");

static MyStream sMyStream;

NxVec3  gDefaultGravity(0,-600.0f,0); // v cm/s2

CP3DPhysEngine::~CP3DPhysEngine()
{
	if (g_pScene) g_pSDK->releaseScene(*g_pScene);
	NxReleaseControllerManager(g_pCtrlMgr);
	NxReleasePhysicsSDK(g_pSDK);
	SAFE_DELETE(g_pFont);
	g_pScene = NULL;
}

bool CP3DPhysEngine::Init()
{
	I_RegisterModule("physPX");
	// memory leaks detection
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(293);

	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	if (!g_pConsole) return false;
	g_pConsole->RegisterLastConVar(g_pLastConVar);

#ifdef _DEBUG
	CON(MSG_CON_INFO, "= Ageia PhysX Physics Engine %d.%d.%d (%s, %s, DEBUG) initialization =", NX_SDK_VERSION_MAJOR, NX_SDK_VERSION_MINOR, NX_SDK_VERSION_BUGFIX, __DATE__, __TIME__);
#else
	CON(MSG_CON_INFO, "= Ageia PhysX Physics Engine %d.%d.%d (%s, %s) initialization =", NX_SDK_VERSION_MAJOR, NX_SDK_VERSION_MINOR, NX_SDK_VERSION_BUGFIX, __DATE__, __TIME__);
#endif

	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	g_pFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);	
			   g_pFont->CreateFont("Arial", 12, true, false);
	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
	g_pXML = (IP3DXML*)I_GetClass(IP3DENGINE_XML);
	g_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);
	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);

	// vytvor physX
	NxSDKCreateError err;
	char ver[32]; sprintf(ver, "%d.%d.%d.%d", P3DVER_1, P3DVER_2, P3DVER_3, P3DVER_4);
	g_pSDK = NxCreatePhysicsSDKWithID(NX_PHYSICS_SDK_VERSION, "Reversity Studios", "Panther3D Engine", ver,
		     "PhysX rocks! ;) --- www.reversity.org", &g_pAllocator, &sMyStream, NxPhysicsSDKDesc(), &err);

	if (!g_pSDK)
	{
		if (err == NXCE_PHYSX_NOT_FOUND)
			CON(MSG_ERR_FATAL, "Can't initialize Ageia PhysX! Please try to reinstall PhysX drivers...");
		else if(err == NXCE_WRONG_VERSION)
			CON(MSG_ERR_FATAL, "Wrong version of Ageia PhysX drivers! Please try to install recent PhysX drivers...");
		else if(err == NXCE_DESCRIPTOR_INVALID)
			CON(MSG_ERR_FATAL, "Wrong Ageia PhysX Descriptor! Please try to install recent PhysX drivers...");
		else if(err == NXCE_CONNECTION_ERROR)
			CON(MSG_ERR_FATAL, "A PhysX card was found, but there are problems when communicating with the card!");
		else if(err == NXCE_RESET_ERROR)
			CON(MSG_ERR_FATAL, "A PhysX card was found, but it did not reset (or initialize) properly!");
		else if(err == NXCE_IN_USE_ERROR)
			CON(MSG_ERR_FATAL, "A PhysX card was found, but it is already in use by another application!");
		else if(err == NXCE_BUNDLE_ERROR)
			CON(MSG_ERR_FATAL, "A PhysX card was found, but there are issues with loading the firmware!");
		else
			CON(MSG_ERR_FATAL, "Can't initialize Ageia PhysX runtime!");
		return false;
	}

	// vytvor cooking
	g_pCooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
	if (!g_pCooking)
	{
		CON(MSG_ERR_FATAL, "Can't get Ageia PhysX cooking!");
		return false;
	}
	if (!g_pCooking->NxInitCooking())
	{
		CON(MSG_ERR_FATAL, "Can't initialize Ageia PhysX cooking!");
		return false;
	}

	// vytvor character controller
	g_pCtrlMgr = NxCreateControllerManager(&g_pAllocator);
	if (!g_pCtrlMgr)
	{
		CON(MSG_ERR_FATAL, "Can't initialize Ageia PhysX Character Controller Manager!");
		return false;
	}

	CON(MSG_CON_INFO, "Phys: Physics engine sucessfuly initialized...");
	bHW = false;

	return true;
}

bool CP3DPhysEngine::Initialize()
{
	// nastav parametry
	g_pSDK->setParameter(NX_SKIN_WIDTH, 0.01f);
	g_pSDK->setParameter(NX_VISUALIZATION_SCALE, 1);

	// vytvor fyz. scenu, nejdrive zkus HW, jinak softwarovou
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = gDefaultGravity;
	sceneDesc.flags |= NX_SF_ENABLE_ACTIVETRANSFORMS;
	g_pScene = NULL;
	if (g_pSDK->getNbPPUs() > 0)
	{
		NxSceneDesc sceneDesc;
 		sceneDesc.simType = NX_SIMULATION_HW;
		g_pScene = g_pSDK->createScene(sceneDesc);
		bHW = true;
	}

	if(!g_pScene)
	{ 
		sceneDesc.simType = NX_SIMULATION_SW; 
		g_pScene = g_pSDK->createScene(sceneDesc);  
		if(!g_pScene) 
		{
			CON(MSG_CON_ERR, "Phys: Can't create scene!");
			return false;
		}
		bHW = false;
		CON(MSG_CON_INFO, "Phys: PhysX hardware not found, using software simulation ;)");
	}

	// vytvor default material, DEBUG:! FIXME:
	NxMaterial* defaultMaterial = g_pScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.5);
	defaultMaterial->setStaticFriction(0.5);
	defaultMaterial->setDynamicFriction(0.5);

	// pripoj se na Visual Remote Debugger
	g_pSDK->getFoundationSDK().getRemoteDebugger()->connect(CVphys_dbg_vrd.GetString(), 5425);

	// radeji proved pocatecni krok... at jsou pocatecni vysledky kroku
	SimulateBegin(0.0f);
	SimulateEnd();

	return true;
}

void CP3DPhysEngine::SimulateBegin(float deltaTime)
{
	g_fTimeDelta = deltaTime;
	g_pScene->simulate(deltaTime);
	g_pScene->flushStream();
	
	// Ziskej aktivni telesa (ty co zmenili pozici od posledniho volani) a nastav entitam tuto novou matici 
	NxU32 nbTransforms = 0;   
	NxActiveTransform *activeTransforms = g_pScene->getActiveTransforms(nbTransforms);
	    
	if(nbTransforms && activeTransforms)    
	{
		for(NxU32 i = 0; i < nbTransforms; ++i)
		{
			IP3DPhysBody* pPhysBody = (IP3DPhysBody*)activeTransforms[i].userData;
			if (pPhysBody)
			{
				ISharedEntity *pEntity = (ISharedEntity *)pPhysBody->GetUserData();
				if(pEntity) 
					pEntity->SetWorldMatrix(Nx2Mat(activeTransforms[i].actor2World));
			}
		}
	}
}

void CP3DPhysEngine::SimulateEnd()
{
	g_pScene->fetchResults(NX_RIGID_BODY_FINISHED, true);

	NxReal maxTimestep;
	NxTimeStepMethod method;
	NxU32 maxIter;
	NxU32 numSubSteps;

	g_pScene->getTiming(maxTimestep, maxIter, method, &numSubSteps);
	if (numSubSteps)  g_pCtrlMgr->updateControllers();
}

void CP3DPhysEngine::RenderDebug()
{
	if (CVphys_debug.GetBool()) 
	{
		//nastaveni
		g_pSDK->setParameter(NX_VISUALIZE_BODY_AXES, CVphys_dbg_pos.GetFloat());
		g_pSDK->setParameter(NX_VISUALIZE_BODY_LIN_VELOCITY, CVphys_dbg_vel.GetFloat());
		g_pSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, CVphys_dbg_shp.GetFloat());
		g_pSDK->setParameter(NX_VISUALIZE_BODY_MASS_AXES, CVphys_dbg_com.GetFloat());

		//renderuj
		::RenderDebug(*g_pScene->getDebugRenderable());
	}
}

IP3DPhysBody* CP3DPhysEngine::GetNearestWorldRayCastHit(P3DXVector3D start, P3DXVector3D end, float &collParam)
{
	P3DXVector3D vStart = start;
	P3DXVector3D vEnd = end;
	P3DXVector3D vDir = vEnd - vStart;

	NxRay worldRay;    
	worldRay.orig=NxVec3(vStart.x, vStart.y, vStart.z); 
	worldRay.dir= NxVec3(vDir.x, vDir.y, vDir.z);
	worldRay.dir.normalize(); //Important!!
	NxRaycastHit hit;
	NxShape *shp = g_pScene->raycastClosestShape(worldRay, NX_ALL_SHAPES, hit, -1, vDir.Length());
	
	if (!shp)
	{
		collParam = 0.0f;
		return NULL;
	}

	P3DXVector3D vImpact(hit.worldImpact.x, hit.worldImpact.y, hit.worldImpact.z);
	vImpact -= vStart;

	collParam = 1.0f / vDir.Length() * vImpact.Length();

	return (IP3DPhysBody*)hit.shape->getActor().userData;
}

void CP3DPhysEngine::SetCollision(PhysCollision collision)
{
	g_pScene->setGroupCollisionFlag(collision.group, PHYSGROUP_NON_COLLIDABLE, false);
	g_pScene->setGroupCollisionFlag(collision.group, PHYSGROUP_COLLIDABLE, true);
	for (int i=1; i<PHYSGROUP_NON_COLLIDABLE; i++)
	{
		g_pScene->setGroupCollisionFlag(collision.group, i, collision.bCollide[i]);
	}
}

void CP3DPhysEngine::BeginNewScene()
{
	g_meshManager.Shutdown();
	g_meshManager.Initialize();
}