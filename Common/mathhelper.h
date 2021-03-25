#pragma once

#undef min

namespace MathHelper
{
	template<class T>
	T min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<class T>
	T clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	float angleFromXY(float x, float y);
}
