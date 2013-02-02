//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics body class interfaces
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"
#include "HP3DMatrix.h"

/** This is Trimesh descriptor. It must be fully filled with appropriate data. This data will be copied to physics engine, so you must to free it after method call. */
struct PhysTrimeshShapeDesc
{
	UINT  numVertices;
	UINT  numTriangles;
	/** Vertex offset */
	UINT  pointStrideBytes;
	/** Triangle offset */
	UINT  triangleStrideBytes;
	/** Pointer to the first point (vertex) */
	const void *  points;
	/** Pointer to the first triangle (first vertex) in index buffer. */
	const void *  triangles;
};

struct PhysShapeDesc
{
	PhysShapeDesc()
	{
		matLocalPose.SetIdentityMatrix();
		shapeFlags = (1<<3);
		collisionGroup = 0;
		materialID = 0;
		skinWidth = -1.0f;
		density =  1.0f;
		mass = -1.0f;  // by default we let the mass be determined by its density.  
		userData = NULL;
	};

	/** Local position of this shape (position from body center). */
	P3DXMatrix matLocalPose;
	UINT shapeFlags;
	/** Collision group - number. Collision interactions can be set by IP3DPhysEngine::SetGroupCollision(). */
	WORD collisionGroup;
	WORD materialID;
	float density;
	float mass;
	float skinWidth;
	void* userData;
};

class IP3DPhysBody : public IP3DBaseInterface
{
public:
	// ### shapy
	/** Add box shape to this body... \param desc Shape description \param dimX Box width \param dimY Box height \param dimZ Box length */
	virtual void AddShapeBox(const PhysShapeDesc &desc, float dimX, float dimY, float dimZ)=0;
	/** Add sphere shape to this body... \param desc Shape description \param radius Sphere radius */
	virtual void AddShapeSphere(const PhysShapeDesc &desc, float radius)=0;
	/** Add plane shape to this body...  \param desc Shape description \param normal Plane normal vector \param distFromOrigin Distance of plane from body origin. */
	virtual void AddShapePlane(const PhysShapeDesc &desc, const P3DXVector3D &normal, float distFromOrigin)=0;
	/** Add capsule shape to this body...  \param desc Shape description \param height Capsule height \param radius Capsule radius. */
	virtual void AddShapeCapsule(const PhysShapeDesc &desc, float height, float radius)=0;
	/** Add cylinder shape to this body...  \param desc Shape description \param height Cylinder height \param radius Cylinder radius. */
	virtual void AddShapeCylinder(const PhysShapeDesc &desc, float height, float radius)=0;
	/** Add triangle mesh shape to this body...  \param desc Shape description \param tridesc Trimesh descriptor. \param bSmoothCollision Smoothed collision mesh is useful for terrain */
	virtual void AddShapeTrimesh(const PhysShapeDesc &desc, const PhysTrimeshShapeDesc &tridesc, bool bSmoothCollision=false)=0;
	/** Add convex triangle mesh shape loaded from 7PHYS to this body...  \param desc Shape description \param sPhysFile Path to 7PHYS file. */
	virtual void AddShapeConvexmesh(const PhysShapeDesc &desc, const char* sPhysFile)=0;
	/** Removes all previousy added shapes so you can create them again. */
	virtual void RemoveAllShapes()=0;
	// ### rizeni
	/** Creates body with previously defined shapes (via AddShape*). */
	virtual bool CreateBody(bool bDynamic)=0;
	/** This is used to set ISharedEntity pointer to entity associated to this body. */
	virtual void SetUserData(void *data)=0;
	/** This is used to set ISharedEntity pointer to entity associated to this body. */
	virtual void* GetUserData()const=0;
	/** This can be changed at runtime. It determines if body is kinematic or not. Kinematic bodies can be directly moved by MoveMatrix().  */
	virtual void SetKinematic(bool bKinematic)=0;
	/** Visualize this body in debug renderer? */
	virtual void SetVisualize(bool bVisualize)=0;
	/** Enable collision for this body? */
	virtual void SetCollision(bool bCollision)=0;
	// ### stavy a parametry
	virtual void SetVelocity(P3DXVector3D vel)=0;
	virtual void SetMass(float mass)=0;
	/** Relatively to center of body. */
	virtual void SetCentreOfMass(P3DXVector3D vec)=0;
	/** Hard way to set body matrix. Useful for initial position, but for runtime use MoveMatrix() \see MoveMatrix */
	virtual void SetMatrix(P3DXMatrix &mat)=0;
	/** Move kinematic actor. \see SetKinematic */
	virtual void MoveMatrix(P3DXMatrix &mat)=0;
};
#define IP3DPHYS_PHYSBODY "P3DPhysBody_1" // nazev ifacu