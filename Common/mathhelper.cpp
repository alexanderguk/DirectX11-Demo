#include "mathhelper.h"

#include <DirectXMath.h>

using namespace DirectX;

float MathHelper::angleFromXY(float x, float y)
{
	float theta = 0.0f;

	if (x >= 0.0f)
	{
		theta = atanf(y / x);

		if (theta < 0.0f)
		{
			theta += XM_2PI;
		}
	}
	else
	{
		theta = atanf(y / x) + XM_PI;
	}

	return theta;
}
