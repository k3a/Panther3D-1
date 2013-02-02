#include "common.h"

LPDIRECT3D9             g_pD3D           = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pD3DDevice     = NULL; // Our rendering device

EngineSettings_t g_pEngSet; // engine settings
IP3DConsole *g_pConsole=NULL; // globální konzola
CP3DTextureLoader	g_TextureLoader;
IP3DMaterialManager *g_pMaterialManager;
CP3DRenderer *g_pRenderer=NULL; // globální promìnná
IP3DMeshLoader	*g_pMeshLoader=NULL;
IP3DPostprocessManager *g_pPostProcessMgr=NULL;
IP3DAlphaManager	*g_pAlphaManager=NULL;
IP3DFileSystem *g_pFS=NULL;
IP3DXML	*g_pXML=NULL;
IP3DEngine *g_pEngine=NULL;
IP3DPhysEngine *g_pPhysEngine=NULL;
IP3DFrustum* g_pFrustum=NULL;
IP3DLightManager	*g_pLightManager=NULL;
IP3DTimer *g_pTimer=NULL;
IP3DSoundManager *g_pSoundMan=NULL;
IP3DEntityMgr *g_pEntMgr=NULL;
// renderer:
CP3DResourceManager *g_pResMgr=NULL;
CP3DDraw2D *g_p2D=NULL;

D3DXMATRIXA16	g_matView;
D3DXMATRIXA16	g_matProj;
D3DXMATRIXA16	g_matViewProj;
P3DXVector3D		g_cameraPosition;
float				g_currentFOV = DEFAULT_FIELD_OF_VIEW;

CP3DStats g_stats;

// g_paramTable sa pouziva vo funkciach SetShaderParameters() aby 
// nebolo potrebne vzdy alokovat niekolko kB na zasobniku
MATERIAL_PARAMETER g_paramTable[SEM_NUM];

//*********************************** funkce ***********************************
#include <dxerr9.h>
void DXErrInfo (HRESULT hr, char *szFile, DWORD dwLine)
{
	const char *szErrName, *szErrDesc;
	char szWinErrDesc[128] = {0}, szErr[256] = {0};

	szErrName = DXGetErrorString9 (hr);
	if (hr > 0)		// windows API error
	{
		FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD)hr, LANG_NEUTRAL, szWinErrDesc, sizeof(szWinErrDesc)-1, NULL);
		szErrDesc = szWinErrDesc;
	}
	else				// DX API error
		szErrDesc = DXGetErrorDescription9 (hr);

	if (szFile)
		wsprintf (szErr, "DXError:0x%lX (%d), file=\"%s\" line=%d: %s - %s\r\n", hr, hr, szFile, dwLine, szErrName, szErrDesc);
	else
		wsprintf (szErr, "DXError:0x%lX (%d): %s - %s\r\n", hr, hr, szErrName, szErrDesc);

	CON(MSG_CON_ERR, szErr);
}