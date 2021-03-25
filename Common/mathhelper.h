#pragma once

#undef min
#undef max

#include <DirectXMath.h>

using namespace DirectX;

namespace MathHelper
{
	float randF();

	float randF(float a, float b);

	template<class T>
	T min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<class T>
	T max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template<class T>
	T clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	float angleFromXY(float x, float y);

	XMMATRIX inverseTranspose(CXMMATRIX M);
}
