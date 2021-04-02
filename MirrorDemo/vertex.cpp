#include "vertex.h"

#include "effects.h"

Vertex::SBasic32::SBasic32() :
	m_pos(0.0f, 0.0f, 0.0f),
	m_normal(0.0f, 0.0f, 0.0f),
	m_tex(0.0f, 0.0f)
{

}

Vertex::SBasic32::SBasic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v) :
	m_pos(px, py, pz),
	m_normal(nx, ny, nz),
	m_tex(u, v)
{

}

const D3D11_INPUT_ELEMENT_DESC CInputLayoutDesc::ms_basic32[3] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

ComPtr<ID3D11InputLayout> CInputLayouts::ms_basic32 = nullptr;

void CInputLayouts::initAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;
	CEffects::ms_basicFX->m_light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	ThrowIfFailed(device->CreateInputLayout(
		CInputLayoutDesc::ms_basic32,
		3,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		ms_basic32.GetAddressOf()
	));
}
