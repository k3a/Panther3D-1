//-------------- Copyright © 2005, 7thSquad, all rights reserved --------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Small but very effective 3 and 4 dimensional vector structures
//-----------------------------------------------------------------------------

#pragma once

#include <math.h>
#include "types.h"
#include "symbols.h"

//////////////////////////////////////////////////////////////////////////
//										P3DXVector3D
//////////////////////////////////////////////////////////////////////////

//--------------------------------
struct P3DXVector3D : public P3DVector3D
{
	//////////////////////////////////////////////////////////////////////////
	// constructors
	P3DXVector3D () {};
	P3DXVector3D (const float *ptr)
	{x = ptr[0]; y = ptr[1]; z = ptr[2];}
	P3DXVector3D (const P3DVector3D &vec) : P3DVector3D(vec) {};
	P3DXVector3D (float fx, float fy, float fz)
	{x = fx; y = fy; z = fz;}
	//////////////////////////////////////////////////////////////////////////
	// operators
	float& operator[] (int i)
	{return (&x)[i];}
	const float& operator[] (int i) const
	{return (&x)[i];}

	P3DXVector3D operator+ (const P3DXVector3D &A) const
	{return P3DXVector3D (x + A.x, y + A.y, z + A.z);}
	P3DXVector3D operator- (const P3DXVector3D &A) const
	{return P3DXVector3D (x - A.x, y - A.y, z - A.z);}
	P3DXVector3D &operator+= (const P3DXVector3D &A)
	{x += A.x; y += A.y; z += A.z; return *this;}
	P3DXVector3D &operator-= (const P3DXVector3D &A)
	{x -= A.x; y -= A.y; z -= A.z; return *this;}
	P3DXVector3D operator- () const								// unarny operator
	{return P3DXVector3D (-x, -y, -z);}

	P3DXVector3D operator+ (const float s) const
	{return P3DXVector3D (x + s, y + s, z + s);}
	P3DXVector3D operator- (const float s) const
	{return P3DXVector3D (x - s, y - s, z - s);}
	P3DXVector3D operator* (const float s) const
	{return P3DXVector3D (x * s, y * s, z * s);}

	P3DXVector3D &operator+= (const float s)
	{x += s, y += s, z += s; return *this;}
	P3DXVector3D &operator-= (const float s)
	{x -= s, y -= s, z -= s; return *this;}
	P3DXVector3D &operator*= (const float s)
	{x *= s, y *= s, z *= s; return *this;}

	bool operator== (const P3DXVector3D &A)
	{return ((x == A.x) && (y == A.y) && (z == A.z));}

	// dot product
	float operator* (const P3DXVector3D &A) const
	{return x * A.x + y * A.y + z * A.z;}
	// cross product
	P3DXVector3D operator% (const P3DXVector3D &A) const
	{
		return P3DXVector3D (y * A.z - z * A.y,
			z * A.x - x * A.z,
			x * A.y - y * A.x);
	}
	P3DXVector3D &operator%= (const P3DXVector3D &A)
	{
		register float xBuf = x, yBuf = y;
		x = y * A.z - z * A.y;
		y = z * A.x - xBuf * A.z;
		z = xBuf * A.y - yBuf * A.x;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	// functions

	inline void Assign (float nx, float ny, float nz)
	{x=nx; y=ny; z=nz;}
	// normalization
	inline P3DXVector3D &Normalize ()
	{
		float fDivider = sqrtf (x * x + y * y + z * z);
		if (fDivider == 0) return *this;
		register float fMultiplier = 1.0f / fDivider;
		x *= fMultiplier;
		y *= fMultiplier;
		z *= fMultiplier;
		return *this;
	}
	// length
	inline float Length () const
	{return sqrtf (x * x + y * y + z * z);}
	// length2 - casto pro porovnavani vzdalenosti
	inline float LengthSqr () const
	{return x*x + y*y + z*z;}
	// minimum value
	inline float GetMin () const
	{return pmin(x,pmin(y,z));}
	// maximum value
	inline float GetMax () const
	{return pmax(x,pmax(y,z));}
	// absolute values
	inline P3DXVector3D &Abs()
	{
		x = fabs(x); y = fabs(y); z = fabs(z);
		return *this;
	}
	// transformacia maticou (predpoklada ze stvrty element=1), obdoba D3DXVec3TransformCoord()
	inline P3DXVector3D &TransformAsPoint (const P3DMatrix &B)
	{
		float fx, fy, fz;
		fx = x * B._11 + y * B._21 + z * B._31 + B._41;
		fy = x * B._12 + y * B._22 + z * B._32 + B._42;
		fz = x * B._13 + y * B._23 + z * B._33 + B._43;
		x = fx; y = fy; z = fz;
		return *this;
	}
	// transformacia maticou (predpoklada ze stvrty element=0), obdoba D3DXVec3TransformNormal()
	inline P3DXVector3D &TransformAsVector (const P3DMatrix &B)
	{
		float fx, fy, fz;
		fx = x * B._11 + y * B._21 + z * B._31;
		fy = x * B._12 + y * B._22 + z * B._32;
		fz = x * B._13 + y * B._23 + z * B._33;
		x = fx; y = fy; z = fz;
		return *this;
	}
	// X-rotation
	inline void P3DXVector3D::RotX (float angle)
	{
		float s = (float) sinf(angle );
		float c = (float) cosf(angle );
		float Y=y;
		y =  y*c - z*s;
		z =  Y*s + z*c;
	}
	// Y-rotation
	inline void P3DXVector3D::RotY (float angle)
	{
		float s = (float) sinf(angle );
		float c = (float) cosf(angle );
		float X=x;
		x =  x*c + z*s;
		z = -X*s + z*c;
	}
	// Z-rotation
	inline void P3DXVector3D::RotZ (float angle)
	{
		float s = (float) sinf(angle );
		float c = (float) cosf(angle );
		float X=x;
		x =  x*c - y*s;
		y =  X*s + y*c;
	}
};



//////////////////////////////////////////////////////////////////////////
//										P3DXVector4D
//////////////////////////////////////////////////////////////////////////

//--------------------------------
struct P3DXVector4D : public P3DVector4D
{
	//////////////////////////////////////////////////////////////////////////
	// constructors
	P3DXVector4D () {};
	P3DXVector4D (const float *ptr)
	{x = ptr[0]; y = ptr[1]; z = ptr[2]; w = 1.0f;}
	P3DXVector4D (const P3DXVector3D &vec)
	{x = vec.x; y = vec.y; z = vec.z; w = 1.0f;}
	P3DXVector4D (const P3DVector4D &vec) : P3DVector4D(vec) {};
	P3DXVector4D (float fx, float fy, float fz, float fw = 1.0f)
	{x = fx; y = fy; z = fz; w = fw;}
	//////////////////////////////////////////////////////////////////////////
	// operators
	float& operator[] (int i)
	{return (&x)[i];}
	const float& operator[] (int i) const
	{return (&x)[i];}

	P3DXVector4D operator+ (const P3DXVector4D &A) const
	{return P3DXVector4D (x + A.x, y + A.y, z + A.z, w);}
	P3DXVector4D operator- (const P3DXVector4D &A) const
	{return P3DXVector4D (x - A.x, y - A.y, z - A.z, w);}
	P3DXVector4D &operator+= (const P3DXVector4D &A)
	{x += A.x; y += A.y; z += A.z; return *this;}
	P3DXVector4D &operator-= (const P3DXVector4D &A)
	{x -= A.x; y -= A.y; z -= A.z; return *this;}
	P3DXVector4D operator- () const								// unarny operator
	{return P3DXVector4D (-x, -y, -z, -w);}

	P3DXVector4D operator+ (const float s) const
	{return P3DXVector4D (x + s, y + s, z + s, w + s);}
	P3DXVector4D operator- (const float s) const
	{return P3DXVector4D (x - s, y - s, z - s, w - s);}
	P3DXVector4D operator* (const float s) const
	{return P3DXVector4D (x * s, y * s, z * s, w * s);}

	P3DXVector4D &operator+= (const float s)
	{x += s, y += s, z += s; w += s; return *this;}
	P3DXVector4D &operator-= (const float s)
	{x -= s, y -= s, z -= s; w -= s; return *this;}
	P3DXVector4D &operator*= (const float s)
	{x *= s, y *= s, z *= s; w *= s; return *this;}

	bool operator== (const P3DXVector4D &A)
	{
		if ((x == A.x) && (y == A.y) && (z == A.z) && (w == A.w))
			return true;
		return false;
	}

	// dot product
	float operator* (const P3DXVector4D &A) const
	{return x * A.x + y * A.y + z * A.z + w * A.w;}

	//////////////////////////////////////////////////////////////////////////
	// functions

	inline void Assign (float nx, float ny, float nz, float nw)
	{x=nx; y=ny; z=nz; w=nw;}
	// normalization
	inline P3DXVector4D &Normalize ()
	{
		register float fMultiplier;
		float fDivider = sqrtf (x*x + y*y + z*z + w*w);
		if (fDivider == 0.0f) return *this;
		fMultiplier = 1.0f / fDivider;
		x *= fMultiplier;
		y *= fMultiplier;
		z *= fMultiplier;
		w *= fMultiplier;
		return *this;
	}
	// length
	inline float Length () const
	{return sqrtf (x*x + y*y + z*z + w*w);}
	// length2 - casto pro porovnavani vzdalenosti
	inline float LengthSqr () const
	{return x*x + y*y + z*z + w*w;}

	// minimum value
	inline float GetMin () const
	{return pmin(x,pmin(y,z));}
	// maximum value
	inline float GetMax () const
	{return pmax(x,pmax(y,z));}
	// absolute values
	inline P3DXVector4D &Abs()
	{
		x = fabs(x); y = fabs(y); z = fabs(z); w = fabs(w);
		return *this;
	}
	// transformacia maticou
	inline P3DXVector4D &Transform (const P3DMatrix &B)
	{
		float fx, fy, fz;
		fx = x * B._11 + y * B._21 + z * B._31 + w * B._41;
		fy = x * B._12 + y * B._22 + z * B._32 + w * B._42;
		fz = x * B._13 + y * B._23 + z * B._33 + w * B._43;
		x = fx; y = fy; z = fz;
		return *this;
	}
};
