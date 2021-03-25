#include "vertex.h"

#include "effects.h"

const D3D11_INPUT_ELEMENT_DESC CInputLayoutDesc::ms_posNormal[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

ComPtr<ID3D11InputLayout> CInputLayouts::ms_posNormal = nullptr;

void CInputLayouts::initAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;
	CEffects::ms_basicFX->m_light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	ThrowIfFailed(device->CreateInputLayout(
		CInputLayoutDesc::ms_posNormal,
		2,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		ms_posNormal.GetAddressOf()
	));
}
