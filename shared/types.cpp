//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Common declaration of types, structs and collision functions
//-----------------------------------------------------------------------------
#include "HP3DVector.h"

float Ray2SphereDist(const P3DRay &ray, const P3DSphere &sphere)
{
	P3DXVector3D sphereOrigin = sphere.pointOrigin;
	P3DXVector3D rayOrigin = ray.rayOrigin;
	P3DXVector3D rayVector = ray.rayVector;
	float sphereRadius = sphere.fRadius;

	P3DXVector3D Q = sphereOrigin - rayOrigin;
	float c = Q.Length();
	float v = Q * rayVector;
	float b = pow(c, 2);
	b -= pow(v, 2);
	float d = sphereRadius * sphereRadius - b;

	// If there was no intersection, return -1
	if (d < 0.0) return -1.0;

	// Return the distance to the [first] intersecting point
	return v - sqrt(d);
} 

bool Ray2SphereTest(const P3DRay &ray, const P3DSphere &sphere) // thx to Kulik ;)
{ 
	P3DXVector3D sphereOrigin = sphere.pointOrigin;
	P3DXVector3D rayOrigin = ray.rayOrigin;
	P3DXVector3D direction = ray.rayVector;
	P3DXVector3D origin_relative = rayOrigin - sphereOrigin;

	if ( origin_relative.LengthSqr() <= sphere.fRadius * sphere.fRadius )
	{
		return true;
	}

	// solve using quadratic equation
	const float a = direction * direction;
	float b = origin_relative * direction;
	b *= 2.0f;
	const float c = origin_relative * origin_relative  - sphere.fRadius * sphere.fRadius;

	// compute discriminant
	const float discriminant = ( b * b ) - ( 4.0f * a * c );
	if ( discriminant < 0 )
	{
		// can't solve quadratic equation, the sphere and ray aren't intersecting
		return false;
	}
	else
	{
		return true;
	}

}

// TODO: testnout jestli fici a udelat dle toho Ray2CylinderTest()
#define RCT_EPSILON 0.0f
bool Ray2CylinderIntersect(const P3DRay &ray, const P3DCylinder& cylinder, float& lambda, OUT P3DIntersect &info)
{
	P3DXVector3D position = ray.rayOrigin;
	P3DXVector3D direction = ray.rayVector;
	P3DXVector3D axis = cylinder.axis;
	P3DXVector3D cylpos = cylinder.position;

	P3DXVector3D RC;
	float d;
	float t,s;
	P3DXVector3D n,D,O;
	float ln;
	float in,out;


	RC = position-cylinder.position;
	n = direction%axis;

	ln=n.Length();

	if ( (ln<RCT_EPSILON)&&(ln>-RCT_EPSILON) ) return false;

	n.Normalize();

	d = fabs( RC*n );

	if (d<=cylinder.fRadius)
	{
		O = RC%axis;
		t = -O*n/ln;
		O = n%axis;
		O.Normalize();
		s=fabs( sqrt(cylinder.fRadius*cylinder.fRadius - d*d) / (direction*O) );

		in=t-s;
		out=t+s;

		if (in<-RCT_EPSILON){
			if (out<-RCT_EPSILON) return false;
			else lambda=out;
		}
		else
			if (out<-RCT_EPSILON) {
				lambda=in;
			}
			else
				if (in<out) lambda=in;
				else lambda=out;

				P3DXVector3D pos=position+direction*lambda;
				info.pos = pos;
				P3DXVector3D HB=pos-cylpos;
				P3DXVector3D vecNormal=HB-axis*(HB*axis);
				vecNormal.Normalize();
				info.normal = vecNormal;

				return true;
	}

	return false;
}

bool Ray2AABBTest(const P3DRay& ray, const P3DAABB& aabb)
{
	P3DXAABB aabbInternal = aabb;

	P3DXVector3D BoxExtents, Diff;

	Diff.x = ray.rayOrigin.x - aabbInternal.GetCenter(0);
	BoxExtents.x = aabbInternal.GetExtent(0);
	if(fabsf(Diff.x)>BoxExtents.x && Diff.x*ray.rayVector.x>=0.0f)	return false;

	Diff.y = ray.rayOrigin.y - aabbInternal.GetCenter(1);
	BoxExtents.y = aabbInternal.GetExtent(1);
	if(fabsf(Diff.y)>BoxExtents.y && Diff.y*ray.rayVector.y>=0.0f)	return false;

	Diff.z = ray.rayOrigin.z - aabbInternal.GetCenter(2);
	BoxExtents.z = aabbInternal.GetExtent(2);
	if(fabsf(Diff.z)>BoxExtents.z && Diff.z*ray.rayVector.z>=0.0f)	return false;

	float fAWdU[3];
	fAWdU[0] = fabsf(ray.rayVector.x);
	fAWdU[1] = fabsf(ray.rayVector.y);
	fAWdU[2] = fabsf(ray.rayVector.z);

	float f;
	f = ray.rayVector.y * Diff.z - ray.rayVector.z * Diff.y;	if(fabsf(f)>BoxExtents.y*fAWdU[2] + BoxExtents.z*fAWdU[1])	return false;
	f = ray.rayVector.z * Diff.x - ray.rayVector.x * Diff.z;	if(fabsf(f)>BoxExtents.x*fAWdU[2] + BoxExtents.z*fAWdU[0])	return false;
	f = ray.rayVector.x * Diff.y - ray.rayVector.y * Diff.x;	if(fabsf(f)>BoxExtents.x*fAWdU[1] + BoxExtents.y*fAWdU[0])	return false;

	return true;
}