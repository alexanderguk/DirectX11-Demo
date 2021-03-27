#pragma once

#include "../Common/d3dutil.h"
using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Vertex
{
	struct SBasic32
	{
		XMFLOAT3 m_pos;
		XMFLOAT3 m_normal;
		XMFLOAT2 m_tex;
	};
}

class CInputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC ms_basic32[3];
};

class CInputLayouts
{
public:
	static void initAll(ID3D11Device* device);

	static ComPtr<ID3D11InputLayout> ms_basic32;
};
