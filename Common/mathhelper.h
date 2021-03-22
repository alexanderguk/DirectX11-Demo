#pragma once

namespace MathHelper
{
	template<class T>
	T clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}
}
