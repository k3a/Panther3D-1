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
#pragma once

#include "windows.h"

// *****************************************************************************************************
// STRUCT

struct P3DScreenPoint2D
{
	int x, y;
};

struct P3DPoint2D
{
	float x, y;
};

struct P3DXPoint2D : public P3DPoint2D
{
	P3DXPoint2D(){x=y=0;};
	P3DXPoint2D(float xx, float yy){x=xx; y=yy;};
	P3DXPoint2D(float pos[2]){x=pos[0]; y=pos[1];};
	P3DXPoint2D(const float *pos){x=pos[0]; y=pos[1];};
};

struct P3DVector3D
{
	float x, y, z;
};
/*
struct P3DXPoint3D : public P3DXVector3D
{
	P3DXPoint3D(){x=y=z=0;};
	P3DXPoint3D(float xx, float yy, float zz){x=xx; y=yy; z=zz;};
	P3DXPoint3D(float pos[3]){x=pos[0]; y=pos[1]; z=pos[2];};
	P3DXPoint3D(const float *pos){x=pos[0]; y=pos[1]; z=pos[2];};
};*/
struct P3DPoint4D
{
	float x, y, z, w;
};

typedef P3DPoint2D P3DVector2D;
typedef P3DPoint4D P3DVector4D;

struct P3DQuaternion
{
	float x, y, z, w;
};

struct P3DRect2D
{
	long x1, y1, x2, y2;
};

struct P3DColorValueRGBA
{
	float r, g, b, a;
	P3DColorValueRGBA() {};
	P3DColorValueRGBA (float inR, float inG, float inB, float inA) : r(inR), g(inG), b(inB), a(inA) {};
};

struct P3DColorValueRGB
{
	float r, g, b;
	P3DColorValueRGB() {};
	P3DColorValueRGB (float inR, float inG, float inB) : r(inR), g(inG), b(inB) {};
};

struct P3DSphere {
	float			fRadius;
	P3DVector3D	pointOrigin;
	P3DSphere() {fRadius=0; pointOrigin.x=0; pointOrigin.y=0; pointOrigin.z=0;}
	P3DSphere(P3DVector3D origin, float radius) {fRadius=radius; pointOrigin=origin;}
	P3DSphere(float radius) {fRadius=radius; pointOrigin.x=0; pointOrigin.y=0; pointOrigin.z=0;}
};

struct P3DRay {
	P3DVector3D	rayOrigin;
	P3DVector3D	rayVector;
	P3DRay() {rayOrigin.x=rayOrigin.y=rayOrigin.z=0; rayVector.x=rayVector.y=rayVector.z=0;}
	P3DRay(P3DVector3D	origin, P3DVector3D	vector) {rayOrigin=origin; rayVector=vector;}
};

struct P3DCylinder
{
	float		fRadius;
	P3DVector3D	position;
	P3DVector3D axis;
	P3DCylinder(){fRadius=0.0f; position.x=position.y=position.z=0.0f; axis.x=axis.y=axis.z=0.0f;};
	P3DCylinder(P3DVector3D pos, P3DVector3D vecAxis, float radius){fRadius=radius; position=pos; axis=vecAxis;};
};

struct P3DAABB {
	P3DVector3D	min;
	P3DVector3D	max;
	P3DAABB() {min.x=min.y=min.z=0; max.x=max.y=max.z=0;}
	P3DAABB(P3DVector3D pntMin, P3DVector3D pntMax) {min=pntMin; max=pntMax;}
	P3DAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
		min.x=minX; min.y=minY; min.z=minZ;    max.x=maxX; max.y=maxY; max.z=maxZ;
	}
};

struct P3DXAABB : public P3DAABB 
{
	P3DXAABB (const P3DAABB &vec) : P3DAABB(vec) {};
	void GetCenter(P3DVector3D &center)
	{
		center.x = GetCenter(0); center.y = GetCenter(1); center.z = GetCenter(2);
	}
	float GetCenter(int axisIndex) 
	{
		switch(axisIndex)
		{
			case 0: return min.x + (max.x - min.x)/2.0f;
			case 1: return min.y + (max.y - min.y)/2.0f;
			case 2: return min.z + (max.z - min.z)/2.0f;
		}
		return 0.0f;
	}
	void GetExtent(P3DVector3D &extent)
	{
		extent.x = GetExtent(0); extent.y = GetExtent(1); extent.z = GetExtent(2);
	}
	float GetExtent(int axisIndex) // vzdalenosti od stredu
	{
		switch(axisIndex)
		{
			case 0: return (max.x - min.x)/2.0f;
			case 1: return (max.y - min.y)/2.0f;
			case 2: return (max.z - min.z)/2.0f;
		}
		return 0.0f;
	}
};


struct P3DIntersect
{
	P3DVector3D pos;
	P3DVector3D normal;
};

// zoznam clusterov, v ktorych sa prave objekt nachadza
#define MAX_CLUSTER_LIST_COUNT	10		// malo by stacit
struct BSPCLUSTER_LIST {
	DWORD		dwNumClusters;
	int		intCluster[MAX_CLUSTER_LIST_COUNT];
	BSPCLUSTER_LIST() {dwNumClusters = 0; for (register DWORD i=0; i<MAX_CLUSTER_LIST_COUNT; i++) intCluster[i] = -1;}
};


// ***************************************************************************
// -------------------------- P3DMatrix --------------------------------------

// definicia (takmer) zhodna s D3DMATRIX
struct P3DMatrix{
	union {
		struct {
			union
			{
				struct {float        _11, _12, _13, _14;};
				P3DVector4D	m_front;
				P3DVector3D	m_front3;
			};
			union
			{
				struct {float        _21, _22, _23, _24;};
				P3DVector4D	m_up;
				P3DVector3D	m_up3;
			};
			union
			{
				struct {float        _31, _32, _33, _34;};
				P3DVector4D	m_right;
				P3DVector3D	m_right3;
			};
			union
			{
				struct {float        _41, _42, _43, _44;};
				P3DVector4D	m_posit;
				P3DVector3D	m_posit3;
			};
		};
		float m[4][4];
		float m16[16];
	};
};

// ***************************************************************************
// -------------------------- CBitset ----------------------------------------
class CBitset
{
public:

	CBitset() : m_bits(0), m_size(0) {}

	~CBitset() 
	{
		if(m_bits) 
		{delete[] m_bits; m_bits = NULL;}
	}

	void Resize(int count) 
	{
		m_size = count/32 + 1;
		if(m_bits)
			delete[] m_bits;

		m_bits = new unsigned int[m_size];
		ClearAll();
	}

	void Set(int i)
	{
		m_bits[i >> 5] |= (1 << (i & 31));
	}

	int On(int i)
	{
		return m_bits[i >> 5] & (1 << (i & 31 ));
	}

	void Clear(int i)
	{
		m_bits[i >> 5] &= ~(1 << (i & 31));
	}

	void ClearAll() 
	{
		if (m_bits)
			memset(m_bits, 0, sizeof(unsigned int) * m_size);
	}

	void SetAll()
	{
		if (m_bits)
			memset(m_bits, 0xFFFFFFFF, sizeof(unsigned int) * m_size);
	}

	bool IsAllZero()
	{
		if (m_bits)
			for (register int i=0; i<m_size; i++)
				if (m_bits[i] != 0)
					return false;
		return true;
	}

	// zluci dve bitsety, musia mat rovnaku velkost
	void Or (const CBitset &A)
	{
		if ((m_bits!=NULL) && (A.m_bits!=NULL) && (m_size == A.m_size))
			for (register int i=0; i<m_size; i++)
				m_bits[i] |= A.m_bits[i];
	}

	// bitove nasobenie bitsetov, musia mat rovnaku velkost
	void And (const CBitset &A)
	{
		if ((m_bits!=NULL) && (A.m_bits!=NULL) && (m_size == A.m_size))
			for (register int i=0; i<m_size; i++)
				m_bits[i] &= A.m_bits[i];
	}

	// bitovy XOR bitsetov, musia mat rovnaku velkost
	void Xor (const CBitset &A)
	{
		if ((m_bits!=NULL) && (A.m_bits!=NULL) && (m_size == A.m_size))
			for (register int i=0; i<m_size; i++)
				m_bits[i] ^= A.m_bits[i];
	}

	// negacia hodnot bitsetu
	void Negate ()
	{
		if (m_bits)
			for (register int i=0; i<m_size; i++)
				m_bits[i] = ~m_bits[i];
	}

private:
	unsigned int *m_bits;
	int m_size;
};

// pouzivane vo viacerych projektoch
struct BSP_LIGHTCELL_INFO
{
	P3DColorValueRGB	Ambient;
	P3DColorValueRGB	Directional;
	P3DVector3D			Direction;
	BSP_LIGHTCELL_INFO() {memset (this, 0, sizeof(BSP_LIGHTCELL_INFO));};
};

// *****************************************************************************************************
// KOLIZNI FUNKCE (Test-bool, Dist-float, Intersection)
float Ray2SphereDist(const P3DRay &ray, const P3DSphere &sphere);
bool  Ray2SphereTest(const P3DRay &ray, const P3DSphere &sphere);
bool  Ray2CylinderIntersect(const P3DRay &ray, const P3DCylinder& cylinder, float& lambda, OUT P3DIntersect &info);
bool  Ray2AABBTest(const P3DRay& ray, const P3DAABB& aabb);

// *****************************************************************************************************
// INPUT
#define DIRECTINPUT_VERSION  0x0800 // Use DirectInput 8
#include "dinput.h" // pro DIK_...