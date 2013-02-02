#include "common.h"

// GLOBALNI UKAZATELE NA CASTO POUZIVANE SINGLETONY

IP3DRenderer* g_pRenderer=NULL;
IP3DMeshLoader* g_pMeshLoader=NULL;
IP3DFont* fntA15=NULL; // 15 Bold font
IP3DFont* fntA17=NULL; // 17 font
EngineSettings_t g_pEngSet; // engine settings
IP3DConsole* g_pConsole=NULL; // global console
IP3DTimer* g_pTimer=NULL; // global timer
IP3DInput* g_pInput=NULL; // global input
IP3DDraw2D *g_p2D=NULL; // pro 2D kreslení
IP3DXML*	g_pXML=NULL;
IP3DBSPMap*	g_pBSPMap=NULL;
IP3DFrustum* g_pFrustum=NULL;
IP3DEngine* g_pEngine=NULL;
IP3DFileSystem* g_pFS=NULL;
IP3DPostprocessManager	*g_pPPManager=NULL;
IP3DStereoscopic	*g_pStereoscopic=NULL;
IP3DEntityMgr *g_pEntMgr=NULL;
IP3DAlphaManager	*g_pAlphaManager=NULL;
IP3DSky				*g_pSkyBox=NULL;
IP3DSoundManager *g_pSoundMan=NULL;
IP3DHelperObject	*g_pHelperObject=NULL;
IP3DLightManager	*g_pLightManager=NULL;
CP3DLocalization *g_pLocaliz=NULL;

// PHYSICS
IP3DPhysEngine *g_pPhysEngine=NULL;

// gui
IP3DGUISystem *g_pGUISystem=NULL;

// delta time
float g_fTimeDelta=1.0f;