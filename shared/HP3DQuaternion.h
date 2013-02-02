//-------------- Copyright © 2007, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Quaternion extention class
//-----------------------------------------------------------------------------

#pragma once

#include <math.h>
#include "types.h"

#define QUAT_NORMALIZE_TOLERANCE	0.001f

//////////////////////////////////////////////////////////////////////////
//											P3DXQuat
//////////////////////////////////////////////////////////////////////////


class P3DXQuat : public P3DQuaternion
{
public:
	//////////////////////////////////////////////////////////////////////////
	// constructors

	P3DXQuat() {};
	P3DXQuat (float fx, float fy, float fz, float fw)
	{x=fx; y=fy; z=fz; w=fw;}
	P3DXQuat (const float *ptr)
	{x = ptr[0]; y = ptr[1]; z = ptr[2]; w = ptr[3];}
	P3DXQuat (const P3DQuaternion &quat) : P3DQuaternion(quat) {};
	P3DXQuat (const P3DXVector3D &vec)
	{x=vec.x; y=vec.y; z=vec.z; w=0.0f;};
	P3DXQuat(const P3DXVector3D &axis, const float angle)
	{BuildFromAxisAngle (axis, angle);}
	P3DXQuat (float rx, float ry, float rz)
	{BuildFromEuler (rx, ry, rz);}
	P3DXQuat(const P3DMatrix &m)
	{BuildFromMatrix (m);}

	//////////////////////////////////////////////////////////////////////////
	// operators

	P3DXQuat operator-() const
	{return P3DXQuat(-x,-y,-z,-w);}
	P3DXQuat& operator+= (const P3DXQuat& q)
	{
		x+=q.x; y+=q.y; z+=q.z; w+=q.w;
		return *this;
	}
	P3DXQuat& operator-= (const P3DXQuat& q)
	{
		x-=q.x; y-=q.y; z-=q.z;w-=q.w;
		return *this;
	}
	P3DXQuat& operator*= (const P3DXQuat& q)
	{
		float xx[3];
		xx[0] = w*q.w - q.x*x - y*q.y - q.z*z;
		xx[1] = w*q.x + q.w*x + y*q.z - q.y*z;
		xx[2] = w*q.y + q.w*y + z*q.x - q.z*x;

		z = w*q.z + q.w*z + x*q.y - q.x*y;
		w = xx[0]; x = xx[1]; y = xx[2];
		return *this;
	}
	P3DXQuat& operator*= (const float s)
	{
		x*=s; y*=s; z*=s; w*=s;
		return *this;
	}
	P3DXQuat operator+(const P3DXQuat &q) const
	{
		return P3DXQuat(x+q.x,y+q.y,z+q.z,w+q.w);
	}
	P3DXQuat operator-(const P3DXQuat &q) const
	{
		return P3DXQuat(x-q.x,y-q.y,z-q.z,w-q.w);
	}
	P3DXQuat operator*(const P3DXQuat &q) const
	{
		return P3DXQuat(w*q.x + q.w*x + y*q.z - q.y*z,
			w*q.y + q.w*y + z*q.x - q.z*x,
			w*q.z + q.w*z + x*q.y - q.x*y,
			w*q.w - x*q.x - y*q.y - z*q.z);
	}
	P3DXQuat operator*(const float s) const
	{
		return P3DXQuat(x*s, y*s, z*s, w*s);
	}
	P3DXQuat operator!() const
	{
		return P3DXQuat(-x,-y,-z,w);
	}

	//////////////////////////////////////////////////////////////////////////
	// functions

	void Assign (float fx, float fy, float fz, float fw)
	{x=fx; y=fy; z=fz; w=fw;}

	void BuildFromAxisAngle (const P3DXVector3D &vecRotAxis, float fRotAngle)
	{
		P3DXVector3D vecTemp(vecRotAxis);
		vecTemp.Normalize ();
		fRotAngle *= 0.5f;
		float fSin = sinf (fRotAngle);
		x = fSin * vecTemp.x;
		y = fSin * vecTemp.y;
		z = fSin * vecTemp.z;
		w = cosf (fRotAngle);
	}
	// vecRotAxis musi byt normalizovana ! - potrebne ???
	void BuildFromAxisAngleFast (const P3DXVector3D &vecRotAxisNorm, float fRotAngle)
	{
		fRotAngle *= 0.5f;
		float fSin = sinf (fRotAngle);
		x = fSin * vecRotAxisNorm.x;
		y = fSin * vecRotAxisNorm.y;
		z = fSin * vecRotAxisNorm.z;
		w = cosf (fRotAngle);
	}
	// je otazne nakolko je funkcia uzitocna !
	void BuildFromEuler (float rx, float ry, float rz)
	{
		P3DXQuat qx (P3DXVector3D(1, 0, 0), rx);
		P3DXQuat qy (P3DXVector3D(0, 1, 0), ry);
		P3DXQuat qz (P3DXVector3D(0, 0, 1), rz);
		*this = qx * qy * qz;
	}

	inline void BuildFromMatrix (const P3DMatrix &matRotation);

	//////////////////////////////////////////////////////////////////////////
	void SetIdentity()
	{x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f;}
	bool IsIdentity() const
	{return x==0 && y==0 && z==0 && fabsf(w)==1;}

	void GetAxisAngle (OUT P3DXVector3D &axis, OUT float &angle) const
	{
		float scale = sqrt(x * x + y * y + z * z);
		if (scale)
		{
			scale = 1.0f / scale;
			axis.x = x * scale;
			axis.y = y * scale;
			axis.z = z * scale;
		}
		else
		{axis.x = 1.0f; axis.y = 0.0f; axis.z = 0.0f;}

		angle = acosf(w) * 2.0f;
	}

	void GetRotationMatrix (OUT P3DMatrix &mat) const
	{
		P3DXQuat qTemp(*this);
		qTemp.Normalize ();

		float x2 = qTemp.x * qTemp.x;
		float y2 = qTemp.y * qTemp.y;
		float z2 = qTemp.z * qTemp.z;
		float xy = qTemp.x * qTemp.y;
		float xz = qTemp.x * qTemp.z;
		float yz = qTemp.y * qTemp.z;
		float wx = qTemp.w * qTemp.x;
		float wy = qTemp.w * qTemp.y;
		float wz = qTemp.w * qTemp.z;

		mat._11 = 1.0f - 2.0f * (y2 + z2);
		mat._12 = 2.0f * (xy + wz);
		mat._13 = 2.0f * (xz - wy);
		mat._14 = 0.0f;

		mat._21 = 2.0f * (xy - wz);
		mat._22 = 1.0f - 2.0f * (x2 + z2);
		mat._23 = 2.0f * (yz + wx);
		mat._24 = 0.0f;

		mat._31 = 2.0f * (xz + wy);
		mat._32 = 2.0f * (yz - wx);
		mat._33 = 1.0f - 2.0f * (x2 + y2);
		mat._34 = 0.0f;

		mat._41 = 0.0f; mat._42 = 0.0f; mat._43 = 0.0f; mat._44 = 1.0f;
		/*
		return P3DXMatrix (1.0f - 2.0f * (y2 + z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
		2.0f * (xy - wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz + wx), 0.0f,
		2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
		*/
	}

	// urci rotaciu, ktorou je potrebne rotovat vektor V1 tak, aby sa dostal do polohy V2
	P3DXQuat& AimFromV1ToV2 (const P3DXVector3D &V1, const P3DXVector3D &V2)
	{
		P3DXVector3D vecV1(V1), vecV2(V2);
		vecV1.Normalize ();
		vecV2.Normalize ();
		P3DXVector3D vecCross = vecV1 % vecV2;
		float fAngle = acosf(vecV1 * vecV2);
		BuildFromAxisAngle (vecCross, fAngle);
		return *this;
	}

	// gets the angle between this quat and the identity quaternion
	float GetAngle() const
	{return acosf(w) * 2.0f;}
	// gets the angle between this quat and the argument - quaterniony asi musia byt normalizovane
	float GetAngle(const P3DXQuat &Q) const
	{return acosf (Dot(Q)) * 2.0f;}
	// squared 4D vector length
	float magnitudeSquared() const
	{return x*x + y*y + z*z + w*w;}
	float Length () const
	{return sqrt(x*x + y*y + z*z + w*w);}
	// returns the dot product of this and argument
	float Dot (const P3DXQuat &Q) const
	{return x * Q.x + y * Q.y + z * Q.z  + w * Q.w;}


	//modifiers:
	void Normalize ()
	{
		float mag2 = w * w + x * x + y * y + z * z;
		if (fabs(mag2 - 1.0f) > QUAT_NORMALIZE_TOLERANCE)	// don't normalize if we don't have to
		{
			if (mag2 == 0.0f)
				{x=0.0f; y=0.0f; z=0.0f; w=1.0f; return;}
			float fDiv = sqrt(mag2);
			fDiv = 1.0f / fDiv;
			x *= fDiv; y *= fDiv; z *= fDiv; w *= fDiv;
		}
	}

	void Conjugate()
		{x = -x; y = -y; z = -z;}
	void Invert()
		{x = -x; y = -y; z = -z; Normalize();}

	// negates all the elements, q and -q represent the same rotation.
	void Negate()
		{x = -x; y = -y; z = -z; w = -w;}

	// dava ine vysledky ako D3DXQuaternionMultiply() ???
	// this = left * right
	void Multiply (const P3DXQuat& left, const P3DXQuat& right)
	{
		float a,b,c,d;
		a = left.w*right.w - left.x*right.x - left.y*right.y - left.z*right.z;
		b = left.w*right.x + right.w*left.x + left.y*right.z - right.y*left.z;
		c = left.w*right.y + right.w*left.y + left.z*right.x - right.z*left.x;
		d = left.w*right.z + right.w*left.z + left.x*right.y - right.x*left.y;

		w = a; x = b; y = c; z = d;
	}

	// this = Slerp(t, a, b)
	inline void Slerp(const float t, const P3DXQuat& a, const P3DXQuat& b);

	// linearly interpolate each component, then normalize the Quaternion
	// Unlike spherical interpolation, this does not rotate at a constant velocity,
	// although that's not necessarily a bad thing
	void NLerp (const P3DXQuat &A, const P3DXQuat &B, float t)
	{
		float w1 = 1.0f - t;
		x = w1*A.x + t*B.x;
		y = w1*A.y + t*B.y;
		z = w1*A.z + t*B.z;
		w = w1*A.w + t*B.w;
		Normalize();
	}

	/*
	// TODO: pozriet sa blizsie na obe moznosti RotateVector()
	// rotates passed vec by rot expressed by unit quaternion, overwrites arg with the result
	void RotateVector (IN OUT P3DXVector3D & v) const
	{
	//float msq = 1.0f/magnitudeSquared();	//assume unit quat!
	P3DXQuat myInverse;
	myInverse.x = -x;// *msq;
	myInverse.y = -y;// *msq;
	myInverse.z = -z;// *msq;
	myInverse.w =  w;// *msq;

	//v = ((*this) * v) ^ myInverse;

	P3DXQuat left;
	left.Multiply(*this,v);
	v.x =left.w*myInverse.x + myInverse.w*left.x + left.y*myInverse.z - myInverse.y*left.z;
	v.y =left.w*myInverse.y + myInverse.w*left.y + left.z*myInverse.x - myInverse.z*left.x;
	v.z =left.w*myInverse.z + myInverse.w*left.z + left.x*myInverse.y - myInverse.x*left.y;
	}
	*/
	void RotateVector (IN OUT P3DXVector3D &vec) const
	{
		P3DXVector3D vn(vec);
		vn.Normalize ();
		P3DXQuat vecQuat (vn);
		vecQuat = vecQuat * P3DXQuat(-x, -y, -z, w);
		vecQuat = (*this) * vecQuat;
		vec.x = vecQuat.x;
		vec.y = vecQuat.y;
		vec.z = vecQuat.z;
	}

	const P3DXQuat& Exp()
	{
		float Mul;
		float Length = sqrt(x*x + y*y + z*z);

		if (Length > 1.0e-4)
			Mul = sin(Length)/Length;
		else
			Mul = 1.0f;

		w = cos(Length);
		x *= Mul;
		y *= Mul;
		z *= Mul; 
		return *this;
	}

	const P3DXQuat& Log()
	{
		float Length;
		Length = sqrt(x*x + y*y + z*z);
		Length = atan(Length/w);
		x *= Length;
		y *= Length;
		z *= Length;
		w = 0.0;
		return *this;
	}

};

// create quaternion from rotation matrix 
// martix must contain only rotation (not scale or shear)
// the result quaternion length is numerical stable 

inline void P3DXQuat::BuildFromMatrix (const P3DMatrix &m)
{
	float tr = m._11 + m._22 + m._33; // trace of matrix

	if (tr > 0.0f)
	{                         // if trace positive than "w" is biggest component
		x = m._23 - m._32; y = m._31 - m._13; z = m._12 - m._21; w = tr + 1.0f;
	}
	// Some of vector components is bigger
	else if ((m._11 > m._22 ) && ( m._11 > m._33))
	{
		x = 1.0f + m._11 - m._22 - m._33; y = m._21 + m._12;
		z = m._31 + m._13; w = m._23 - m._32;
	}
	else if (m._22 > m._33)
	{
		x = m._21 + m._12; y = 1.0f + m._22 - m._11 - m._33;
		z = m._32 + m._23; w = m._31 - m._13;
	}
	else
	{
		x = m._31 + m._13; y = m._32 + m._23;
		z = 1.0f + m._33 - m._11 - m._22; w = m._12 - m._21;
	}
	Normalize ();
}

/*
// tato verzia podla vsetkeho nepracuje spravne
inline void P3DXQuat::BuildFromMatrix (const P3DMatrix &matRotation)
{
	float tr = m._11 + m._22 + m._33; // trace of matrix

	if (tr > 0.0f)
	{                         // if trace positive than "w" is biggest component
		x = m._23 - m._32; y = m._31 - m._13; z = m._12 - m._21; w = tr + 1.0f;
	}
	// Some of vector components is bigger
	else if ((m._11 > m._22 ) && ( m._11 > m._33))
	{
		x = 1.0f + m._11 - m._22 - m._33; y = m._21 + m._12;
		z = m._31 + m._13; w = m._23 - m._32;
	}
	else if (m._22 > m._33)
	{
		x = m._21 + m._12; y = 1.0f + m._22 - m._11 - m._33;
		z = m._32 + m._23; w = m._31 - m._13;
	}
	else
	{
		x = m._31 + m._13; y = m._32 + m._23;
		z = 1.0f + m._33 - m._11 - m._22; w = m._12 - m._21;
	}
	Normalize ();
}

/*
// tato verzia podla vsetkeho nepracuje spravne
inline void P3DXQuat::BuildFromMatrix (const P3DMatrix &matRotation)
{
float fRoot, fTrace;
float matRot[4][4];

memcpy (matRot, &matRotation, sizeof (P3DMatrix));
fTrace = matRot[0][0] + matRot[1][1] + matRot[2][2];

if (fTrace > 0.0f)
{
// |w| > 1/2, may as well choose w > 1/2
fRoot = sqrtf (fTrace + 1.0f);  // 2w
w = 0.5f * fRoot;

fRoot = 0.5f / fRoot;  // 1/(4w) == 1/ 4* sqrt(1/4(trace))
x = (matRot[1][2] - matRot[2][1]) * fRoot;
y = (matRot[2][0] - matRot[0][2]) * fRoot;
z = (matRot[0][1] - matRot[1][0]) * fRoot;
return;
}
else
{
// |w| <= 1/2
static size_t s_iNext[3] = { 1, 2, 0 };
size_t i = 0;
if (matRot[1][1] > matRot[0][0])   i = 1;
if (matRot[2][2] > matRot[i][i])   i = 2;

size_t j = s_iNext[i];
size_t k = s_iNext[j];

fRoot = sqrtf(matRot[i][i] - matRot[j][j] - matRot[k][k] + 1.0f);

// Ease of addressing
float* apkQuat[3] = { &x, &y, &z };

*apkQuat[i] = 0.5f * fRoot;
fRoot = 0.5f / fRoot;

w = (matRot[k][j] - matRot[j][k]) * fRoot;
*apkQuat[j] = (matRot[j][i] + matRot[i][j]) * fRoot;
*apkQuat[k] = (matRot[k][i] + matRot[i][k]) * fRoot;
return;
}
*/

// this = Slerp(t, a, b)
inline void P3DXQuat::Slerp (const float t, const P3DXQuat& left, const P3DXQuat& right)
{
#define QUAT_SLERP_EPSILON1	(float(1.0e-8f))

	*this = left;
	//this is left.dot(right)
	float cosine =  x * right.x + y * right.y + z * right.z + w * right.w;

	float sign = 1.0f;
	if (cosine < 0)
	{
		cosine = - cosine;
		sign = -1.0f;
	}

	float Sin = 1.0f - cosine*cosine;

	if (Sin >= QUAT_SLERP_EPSILON1 * QUAT_SLERP_EPSILON1)	
	{
		Sin = sqrtf (Sin);
		const float angle = atan2(Sin, cosine);
		const float i_sin_angle = 1.0f / Sin;

		float lower_weight = sinf(angle*(1.0f-t)) * i_sin_angle;
		float upper_weight = sinf(angle * t) * i_sin_angle * sign;

		w = (w * (lower_weight)) + (right.w * (upper_weight));
		x = (x * (lower_weight)) + (right.x * (upper_weight));
		y = (y * (lower_weight)) + (right.y * (upper_weight));
		z = (z * (lower_weight)) + (right.z * (upper_weight));
	}
}
