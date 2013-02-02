//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Base entity part: Physics description and interaction
//-----------------------------------------------------------------------------
#include "eBasePhysObj.h"
#include "common.h"

#include "eBase.h"

void eBasePhysObj::InitBasePhysObj(eBase* thisBase){
	base=thisBase;
	m_pBody=(IP3DPhysBody *)I_GetClass(IP3DPHYS_PHYSBODY);
	bLoaded=false;
	m_moveType = MOVE_NONE;
}

void eBasePhysObj::ShutdownBasePhysObj(){
	SAFE_DELETE(m_pBody);
}

void eBasePhysObj::GetXmlTransformMatrix (ezxml_t pXml, P3DXMatrix &pMatrix)
{
	double v1, v2, v3; // pomocné promìnné
	if (pXml == NULL) return;
	ezxml_t posXml = g_pXml->Child(pXml, "position"); // ziskej pozici kolize
	ezxml_t rotXml = g_pXml->Child(pXml, "rotation"); // ziskej rotaci kolize
	if ((posXml == NULL) && (rotXml == NULL))
		return;

	pMatrix.SetIdentityMatrix ();
	if (rotXml)
	{
		sscanf (rotXml->txt, "%lf %lf %lf", &v1, &v2, &v3);
		pMatrix.RotateZ ((const float)P3DToRadian(v3));
		pMatrix.RotateX ((const float)P3DToRadian(v1));
		pMatrix.RotateY ((const float)P3DToRadian(v2));
	}
	if (posXml)
	{
		sscanf (posXml->txt, "%lf %lf %lf", &v1, &v2, &v3);
		pMatrix.m_posit.x = (float)v1; pMatrix.m_posit.y = (float)v2; pMatrix.m_posit.z = (float)v3;
	}
}

bool eBasePhysObj::LoadPhysModel(const char* modelXmlPath, bool bStatic) // udela fyzikalni model ze specifikovaneho souboru
{
	ezxml_t modelXml = g_pXml->Parse_file(modelXmlPath);
	if (modelXml == NULL)
	{
		g_pConsole->Message(MSG_CON_ERR, "CreatePhysModel(): Can't parse xml description file '%s'!", modelXmlPath);
		g_pXml->Free(modelXml);
		return false;
	}

	if (!LoadFromXml(modelXml, modelXmlPath, bStatic)) return false;

	return true;
}

bool eBasePhysObj::CreatePhysModel(bool bStatic) // vytvori podle toho co je nacteno v renderable casti eBase, vesmes staci
{
	const char* path = base->GetModelPath();
	if (!path)
	{
		CON(MSG_CON_ERR, "CreatePhysModel(): Can't create physics body from renderable without path. Call LoadModel() before!");
		return false;
	}

	if (!LoadPhysModel(path, bStatic)) return false;

	return true;
}

bool eBasePhysObj::LoadFromXml(const ezxml_t modelXml, const char* modelpath, bool bStatic)
{
	DWORD dwBuf; const char *szBuf; // pomocné promìnné
	ezxml_t part, body;
	P3DXMatrix matTransform;

	DWORD dwCoumpoudsNum;
	const char *primType;
	P3DXVector3D XYZ;
	float h, r;
	PhysShapeDesc desc;

	bLoaded = false;

	if (!modelXml) return false;

	// model type solid
	part = g_pXml->Child(modelXml, "model");

	//////////////////////////////////////////////////////////////////////////
	// nacitanie fyziky - zatial sa pocita iba s typom "solid", zatial ziadne joints a podobne
	if ((part = g_pXml->Child(modelXml, "physics")) == NULL) return false;
	if ((body = g_pXml->Child(part, "body")) == NULL) return false;

	for (dwCoumpoudsNum = 0, part = g_pXml->Child(body, "part"); part; part = part->next)
		dwCoumpoudsNum++;

	if (dwCoumpoudsNum == 0) 
	{
		CON(MSG_CON_ERR, "Model %s doesn't contain any physical shapes! Model won't be physically simulated :(", modelpath);
		return false; // nemame zadne shapes
	}

	for (dwBuf = 0, part = g_pXml->Child(body, "part"); part; part = part->next, dwBuf++)
		if (primType = g_pXml->Attr(part, "type"))
		{
			szBuf = g_pXml->Attr(part, "r");
			r = szBuf ? (float)atof(szBuf) : 0;			// r
			szBuf = g_pXml->Attr(part, "h");
			h = szBuf ? (float)atof(szBuf) : 0;			// h
			szBuf = g_pXml->Attr(part, "x");
			XYZ.x = szBuf ? (float)atof(szBuf) : 0;	// XYZ.x
			szBuf = g_pXml->Attr(part, "y");
			XYZ.y = szBuf ? (float)atof(szBuf) : 0;	// XYZ.y
			szBuf = g_pXml->Attr(part, "z");
			XYZ.z = szBuf ? (float)atof(szBuf) : 0;	// XYZ.z

			if (stricmp (primType, "box") == 0)				// box
			{
				GetXmlTransformMatrix(part, desc.matLocalPose);
				m_pBody->AddShapeBox(desc, XYZ.x, XYZ.y, XYZ.z);
			}
			else if (stricmp (primType, "ellipsoid") == 0)		// ellipsoid
			{
				CON(MSG_CON_ERR, "Model %s uses obsolete ellipsoid shape primitive - please use box, sphere, capsule, cylinder or model instead...", modelpath);
				//coll[dwBuf] = g_pNewtonWorld->CollisionCreateEllipsoid (XYZ, GetXmlTransformMatrix(part, matTransform));
			}
			else if (stricmp (primType, "cone") == 0)			// cone
			{
				CON(MSG_CON_ERR, "Model %s uses obsolete cone shape primitive - please use box, sphere, capsule, cylinder or model instead...", modelpath);
				//coll[dwBuf] = g_pNewtonWorld->CollisionCreateCone(r, h, GetXmlTransformMatrix(part, matTransform));
			}
			else if (stricmp (primType, "capsule") == 0)		// capsule
			{
				GetXmlTransformMatrix(part, desc.matLocalPose);
				m_pBody->AddShapeCapsule(desc, h, r);
			}
			else if (stricmp (primType, "cylinder") == 0)		// cylinder
			{
				GetXmlTransformMatrix(part, desc.matLocalPose);
				m_pBody->AddShapeCylinder(desc, h, r);
			}
			else if (stricmp (primType, "model") == 0)			// model
			{
				ezxml_t XMLfilename;
				if (XMLfilename = g_pXml->Child(part, "filename"))
					GetXmlTransformMatrix(part, desc.matLocalPose);
				m_pBody->AddShapeConvexmesh(desc, XMLfilename->txt);
			}
			else if (stricmp (primType, "null") == 0)			// null
			{
				CON(MSG_CON_ERR, "Model %s uses obsolete null shape primitive - please use box, sphere, capsule, cylinder or model instead...", modelpath);
			}

		}

		if (!m_pBody->CreateBody(!bStatic)) return false; // POKUS SE VYTVORIT TELESO

		// hmotnost
		float fMass = 1.0f;
		if (szBuf = g_pXml->Attr(body, "mass"))
			fMass = (float)atof(szBuf);
		m_pBody->SetMass(fMass);

		// tazisko
		if (szBuf = g_pXml->Attr(body, "mass_center"))
		{
			P3DXVector3D pointMC;
			sscanf (szBuf, "%f %f %f", &pointMC.x, &pointMC.y, &pointMC.z);
			m_pBody->SetCentreOfMass (pointMC);
		}
		// momenty zotrvacnosti
		if (szBuf = g_pXml->Attr(body, "moments_of_inertia"))
		{
			CON(MSG_CON_ERR, "moments_of_inertia not yet implemented!!!");
			/*P3DXVector3D vecMoI;
			sscanf (szBuf, "%f %f %f", &vecMoI.x, &vecMoI.y, &vecMoI.z);
			vecMoI *= fMass;
			m_nBody->SetMassMatrix (fMass, vecMoI);*/
		}

		m_pBody->SetUserData(base); // nastav jako userdata nasi entitu

		// pozice
		P3DXMatrix mat;
		base->GetWorldMatrix(mat);
		m_pBody->SetMatrix(mat);

		// fyzikalny povrch
		/*if (szBuf = g_pXml->Attr(body, "surface"))
		m_nBody->SetGameMaterialGroupID (szBuf);
		else
		m_nBody->SetGameMaterialGroupID ("common");
		// linear/angular damping
		if (szBuf = g_pXml->Attr(body, "damping"))
		{
		float fDamping = (float)atof (szBuf);
		if (fDamping > 0.0f)
		m_nBody->SetDamping (fDamping);
		}*/

		bLoaded = true;

		return true;
}

void eBasePhysObj::SetMoveType(eMoveType mt)
{
	m_moveType = mt;

	if (m_moveType == MOVE_PARENT) 
		m_pBody->SetKinematic(true); 
	else 
		m_pBody->SetKinematic(false); 
}