#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct SDirectionalLight
{
	SDirectionalLight();

	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular;

	XMFLOAT3 m_direction;
	float m_padding;
};

struct SPointLight
{
	SPointLight();

	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular;

	XMFLOAT3 m_position;
	float m_range;

	XMFLOAT3 m_attenuation;
	float m_padding;
};

struct SSpotLight
{
	SSpotLight();

	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular;

	XMFLOAT3 m_position;
	float m_range;

	XMFLOAT3 m_direction;
	float m_spotExp;

	XMFLOAT3 m_attenuation;
	float m_padding;
};

struct SMaterial
{
	SMaterial();

	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular;
	XMFLOAT4 m_reflect;
};
