#include "effects.h"

#include <fstream>
#include <vector>

CEffect::CEffect(ID3D11Device* device, const std::wstring& filename)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(compiledShader.data(), size);
	fin.close();

	ThrowIfFailed(D3DX11CreateEffectFromMemory(
		compiledShader.data(), size, 0, device, m_FX.GetAddressOf()
	));
}

CEffect::~CEffect()
{

}

CBasicEffect::CBasicEffect(ID3D11Device* device, const std::wstring& filename) :
	CEffect(device, filename)
{
	m_light1Tech = m_FX->GetTechniqueByName("Light1");
	m_light2Tech = m_FX->GetTechniqueByName("Light2");
	m_light3Tech = m_FX->GetTechniqueByName("Light3");
	m_worldViewProj = m_FX->GetVariableByName("g_worldViewProj")->AsMatrix();
	m_world = m_FX->GetVariableByName("g_world")->AsMatrix();
	m_worldInvTranspose = m_FX->GetVariableByName("g_worldInvTranspose")->AsMatrix();
	m_eyePosW = m_FX->GetVariableByName("m_eyePosW")->AsVector();
	m_dirLights = m_FX->GetVariableByName("g_dirLights");
	m_mat = m_FX->GetVariableByName("g_material");
}

CBasicEffect::~CBasicEffect()
{

}

void CBasicEffect::setWorldViewProj(CXMMATRIX M)
{
	m_worldViewProj->SetMatrix(reinterpret_cast<const float*>(&M));
}

void CBasicEffect::setWorld(CXMMATRIX M)
{
	m_world->SetMatrix(reinterpret_cast<const float*>(&M));
}

void CBasicEffect::setWorldInvTranspose(CXMMATRIX M)
{
	m_worldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M));
}

void CBasicEffect::setEyePosW(const XMFLOAT3& v)
{
	m_eyePosW->SetRawValue(&v, 0, sizeof(v));
}

void CBasicEffect::setDirLights(const SDirectionalLight* lights)
{
	m_dirLights->SetRawValue(lights, 0, 3 * sizeof(SDirectionalLight));
}

void CBasicEffect::setMaterial(const SMaterial& mat)
{
	m_mat->SetRawValue(&mat, 0, sizeof(mat));
}

std::unique_ptr<CBasicEffect> CEffects::ms_basicFX = nullptr;

void CEffects::initAll(ID3D11Device* device)
{
	ms_basicFX = std::make_unique<CBasicEffect>(device, L"FX/Basic.fxo");
}
