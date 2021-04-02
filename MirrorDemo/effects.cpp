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

	m_light0TexTech = m_FX->GetTechniqueByName("Light0Tex");
	m_light1TexTech = m_FX->GetTechniqueByName("Light1Tex");
	m_light2TexTech = m_FX->GetTechniqueByName("Light2Tex");
	m_light3TexTech = m_FX->GetTechniqueByName("Light3Tex");

	m_light0TexAlphaClipTech = m_FX->GetTechniqueByName("Light0TexAlphaClip");
	m_light1TexAlphaClipTech = m_FX->GetTechniqueByName("Light1TexAlphaClip");
	m_light2TexAlphaClipTech = m_FX->GetTechniqueByName("Light2TexAlphaClip");
	m_light3TexAlphaClipTech = m_FX->GetTechniqueByName("Light3TexAlphaClip");

	m_light1FogTech = m_FX->GetTechniqueByName("Light1Fog");
	m_light2FogTech = m_FX->GetTechniqueByName("Light2Fog");
	m_light3FogTech = m_FX->GetTechniqueByName("Light3Fog");

	m_light0TexFogTech = m_FX->GetTechniqueByName("Light0TexFog");
	m_light1TexFogTech = m_FX->GetTechniqueByName("Light1TexFog");
	m_light2TexFogTech = m_FX->GetTechniqueByName("Light2TexFog");
	m_light3TexFogTech = m_FX->GetTechniqueByName("Light3TexFog");

	m_light0TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light0TexAlphaClipFog");
	m_light1TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light1TexAlphaClipFog");
	m_light2TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light2TexAlphaClipFog");
	m_light3TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light3TexAlphaClipFog");

	m_worldViewProj = m_FX->GetVariableByName("g_worldViewProj")->AsMatrix();
	m_world = m_FX->GetVariableByName("g_world")->AsMatrix();
	m_worldInvTranspose = m_FX->GetVariableByName("g_worldInvTranspose")->AsMatrix();
	m_texTransform = m_FX->GetVariableByName("g_texTransform")->AsMatrix();
	m_eyePosW = m_FX->GetVariableByName("g_eyePosW")->AsVector();
	m_fogColor = m_FX->GetVariableByName("g_fogColor")->AsVector();
	m_fogStart = m_FX->GetVariableByName("g_fogStart")->AsScalar();
	m_fogRange = m_FX->GetVariableByName("g_fogRange")->AsScalar();
	m_dirLights = m_FX->GetVariableByName("g_dirLights");
	m_mat = m_FX->GetVariableByName("g_material");
	m_diffuseMap = m_FX->GetVariableByName("g_diffuseMap")->AsShaderResource();
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

void CBasicEffect::setTexTransform(CXMMATRIX M)
{
	m_texTransform->SetMatrix(reinterpret_cast<const float*>(&M));
}

void CBasicEffect::setEyePosW(const XMFLOAT3& v)
{
	m_eyePosW->SetRawValue(&v, 0, sizeof(v));
}

void CBasicEffect::setFogColor(const FXMVECTOR v)
{
	m_fogColor->SetFloatVector(reinterpret_cast<const float*>(&v));
}

void CBasicEffect::setFogStart(float f)
{
	m_fogStart->SetFloat(f);
}

void CBasicEffect::setFogRange(float f)
{
	m_fogRange->SetFloat(f);
}

void CBasicEffect::setDirLights(const SDirectionalLight* lights)
{
	m_dirLights->SetRawValue(lights, 0, 3 * sizeof(SDirectionalLight));
}

void CBasicEffect::setMaterial(const SMaterial& mat)
{
	m_mat->SetRawValue(&mat, 0, sizeof(mat));
}

void CBasicEffect::setDiffuseMap(ID3D11ShaderResourceView* tex)
{
	m_diffuseMap->SetResource(tex);
}

std::unique_ptr<CBasicEffect> CEffects::ms_basicFX = nullptr;

void CEffects::initAll(ID3D11Device* device)
{
	ms_basicFX = std::make_unique<CBasicEffect>(device, L"FX/Basic.fxo");
}
