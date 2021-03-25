#include "mathhelper.h"

#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;


float MathHelper::randF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

float MathHelper::randF(float a, float b)
{
	return a + randF() * (b - a);
}

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

DirectX::XMMATRIX MathHelper::inverseTranspose(CXMMATRIX M)
{
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}
