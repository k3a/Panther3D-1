//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics body class
//-----------------------------------------------------------------------------
#include "physbody.h"
#include "physengine.h"
#include "NxCooking.h"

REGISTER_CLASS(CP3DPhysBody, IP3DPHYS_PHYSBODY); // zaregistruj tuto tøídu

CP3DPhysBody::CP3DPhysBody()
{
	bShapes = false;
	bInitialized = false;
}

CP3DPhysBody::~CP3DPhysBody()
{
	if (!g_pScene) return;
	RemoveAllShapes();
}

inline void CopyShapeDesc(NxShapeDesc *shape, const PhysShapeDesc &desc)
{
	shape->group = desc.collisionGroup;
	if (shape->group==0) shape->group = PHYSGROUP_COLLIDABLE; // if group not set, default to always collidable
	shape->density = desc.density;
	shape->mass = desc.mass;
	shape->materialIndex = desc.materialID;
	shape->localPose = Mat2Nx(desc.matLocalPose);
	shape->shapeFlags = desc.shapeFlags;
	shape->skinWidth = desc.skinWidth;
	shape->userData = desc.userData;
	shape->density = 10.0f;
}

void CP3DPhysBody::AddShapeBox(const PhysShapeDesc &desc, float dimX, float dimY, float dimZ)
{
	NxBoxShapeDesc *shape = new NxBoxShapeDesc();
	CopyShapeDesc(shape, desc);
	shape->dimensions.set(dimX/2.0f, dimY/2.0f, dimZ/2.0f);

	m_desc.shapes.push_back(shape);
	bShapes = true;
}

void CP3DPhysBody::AddShapeSphere(const PhysShapeDesc &desc, float radius)
{
	NxSphereShapeDesc *shape = new NxSphereShapeDesc();
	CopyShapeDesc(shape, desc);
	shape->radius = radius;

	m_desc.shapes.push_back(shape);
	bShapes = true;
}

void CP3DPhysBody::AddShapePlane(const PhysShapeDesc &desc, const P3DXVector3D &normal, float distFromOrigin)
{
	NxPlaneShapeDesc *shape = new NxPlaneShapeDesc();
	CopyShapeDesc(shape, desc);
	shape->normal = NxVec3(normal.x, normal.y, normal.z);
	shape->d = distFromOrigin;

	m_desc.shapes.push_back(shape);
	bShapes = true;
}

void CP3DPhysBody::AddShapeCapsule(const PhysShapeDesc &desc, float height, float radius)
{
	// preved capsuli co lezi na X ose na capsuli Y kterou ma PhysX
	P3DXMatrix matLocalPose = desc.matLocalPose;
	P3DXVector3D pos = matLocalPose.m_posit3;
	matLocalPose.m_posit3.x = matLocalPose.m_posit3.y = matLocalPose.m_posit3.z = 0.0f;
	matLocalPose.RotateZ(-P3D_PI/2.0f);
	matLocalPose.m_posit3 = pos;

	NxCapsuleShapeDesc *shape = new NxCapsuleShapeDesc();
	CopyShapeDesc(shape, desc);
	shape->localPose = Mat2Nx(matLocalPose); // zkopiruj prevedenou
	shape->height = height;
	shape->radius = radius;

	m_desc.shapes.push_back(shape);
	bShapes = true;
}

void CP3DPhysBody::AddShapeCylinder(const PhysShapeDesc &desc, float height, float radius)
{
	// CYLINDER UDELAME JAKO CAPSULI A TRIMESH CYLINDER
	// preved capsuli co lezi na X ose na capsuli Y kterou ma PhysX
	P3DXMatrix matLocalPose = desc.matLocalPose;
	P3DXVector3D pos = matLocalPose.m_posit3;
	matLocalPose.m_posit3.x = matLocalPose.m_posit3.y = matLocalPose.m_posit3.z = 0.0f;
	matLocalPose.RotateZ(-P3D_PI/2.0f);
	matLocalPose.m_posit3 = pos;

	if (height > 2.0f*radius) // zde je moznost udelat capsuli...
	{
		NxCapsuleShapeDesc *shape = new NxCapsuleShapeDesc();
		CopyShapeDesc(shape, desc);
		shape->localPose = Mat2Nx(matLocalPose); // zkopiruj prevedenou
		shape->height = height-2.0f*radius;
		shape->radius = radius;

		m_desc.shapes.push_back(shape);
	}

	// udelej trimesh cylinder
	NxConvexShapeDesc *shape = new NxConvexShapeDesc();
	CopyShapeDesc(shape, desc);
	shape->localPose = Mat2Nx(matLocalPose); // zkopiruj prevedenou

	NxConvexMesh* cm = g_meshManager.LoadCylinder(height, radius-0.1f);
	if (!cm)
	{
		if (height > 2.0f*radius) // je tam alespon capsule
		{
			CON(MSG_CON_ERR, "Phys: Failed to create convex mesh for cylinder shape (h=%f, r=%f)!", height, radius);
			bShapes = true;
			return;
		}
		CON(MSG_CON_ERR, "Phys: Failed to create cylinder shape (h=%f, r=%f)!", height, radius);
		return;
	}

	shape->meshData = cm;

	m_desc.shapes.push_back(shape);

	bShapes = true;
}

void CP3DPhysBody::AddShapeTrimesh(const PhysShapeDesc &desc, const PhysTrimeshShapeDesc &tridesc, bool bSmoothCollision)
{
	NxTriangleMeshShapeDesc *shape = new NxTriangleMeshShapeDesc();
	CopyShapeDesc(shape, desc);
	
	// vytvor popis trimeshe
	NxTriangleMeshDesc meshDesc;
	meshDesc.numVertices                = tridesc.numVertices;
	meshDesc.numTriangles               = tridesc.numTriangles;
	meshDesc.pointStrideBytes           = tridesc.pointStrideBytes;
	meshDesc.triangleStrideBytes        = tridesc.triangleStrideBytes;
	meshDesc.points						= tridesc.points;
	meshDesc.triangles                  = tridesc.triangles;                           
	meshDesc.flags                      = 0;

	if (!g_pCooking->NxCookTriangleMesh(meshDesc, UserStream("tempfile.bin", false)))
	{
		CON(MSG_CON_ERR, "Phys: Can't cook triangle mesh for trimesh shape!");
		return;
	}

	NxTriangleMesh *pTriMesh = g_pSDK->createTriangleMesh(UserStream("tempfile.bin", true)); // FIXME: jak se to uvolnuje? A vytvaret jen jeden per objekt?
	if (!pTriMesh) 
	{
		CON(MSG_CON_ERR, "Phys: Can't create triangle mesh for trimesh shape!");
		return;
	}

	shape->meshData = pTriMesh;

	m_desc.shapes.push_back(shape);
	bShapes = true;
}

void CP3DPhysBody::AddShapeConvexmesh(const PhysShapeDesc &desc, const char* sPhysFile)
{
	NxConvexShapeDesc *shape = new NxConvexShapeDesc();
	CopyShapeDesc(shape, desc);
	
	// vytvor popis trimeshe
	NxConvexMesh* cm = g_meshManager.LoadPhysMesh(sPhysFile);
	if (!cm) return; // error already written in meshmanager

	shape->meshData = cm;

	m_desc.shapes.push_back(shape);
	bShapes = true;
}

void CP3DPhysBody::RemoveAllShapes()
{
	for(UINT i = 0; i < m_desc.shapes.size(); i++) SAFE_DELETE(m_desc.shapes[i]); // uvolni
	bShapes = false;
}

bool CP3DPhysBody::CreateBody(bool bDynamic)
{
	bInitialized = false;

	if (!bShapes)
	{
		CON(MSG_CON_ERR, "Phys: Trying to create physics body without shapes!");
		return false;
	}

	NxBodyDesc bodyDesc;
	if (bDynamic) 
	{
		// apply default values
		bodyDesc.mass = 1;
		m_desc.body = &bodyDesc;
	}

	m_pActor = g_pScene->createActor(m_desc);

	RemoveAllShapes();

	if (!m_pActor) return false;

	m_pActor->userData = this;

	bInitialized = true;
	return true;
}