#pragma once

#include <memory>

#include "../Common/d3dutil.h"
#include "../Common/lighthelper.h"
using namespace DirectX;

using Microsoft::WRL::ComPtr;

class CEffect
{
public:
	CEffect(ID3D11Device* device, const std::wstring& filename);
	virtual ~CEffect();

protected:
	ComPtr<ID3DX11Effect> m_FX;
};

class CBasicEffect : public CEffect
{
public:
	CBasicEffect(ID3D11Device* device, const std::wstring& filename);
	virtual ~CBasicEffect() override;

	void setWorldViewProj(CXMMATRIX M);
	void setWorld(CXMMATRIX M);
	void setWorldInvTranspose(CXMMATRIX M);
	void setTexTransform(CXMMATRIX M);
	void setEyePosW(const XMFLOAT3& v);
	void setDirLights(const SDirectionalLight* lights);
	void setMaterial(const SMaterial& mat);
	void setDiffuseMap(ID3D11ShaderResourceView* tex);

	ComPtr<ID3DX11EffectTechnique> m_light1Tech;
	ComPtr<ID3DX11EffectTechnique> m_light2Tech;
	ComPtr<ID3DX11EffectTechnique> m_light3Tech;

	ComPtr<ID3DX11EffectTechnique> m_light0TexTech;
	ComPtr<ID3DX11EffectTechnique> m_light1TexTech;
	ComPtr<ID3DX11EffectTechnique> m_light2TexTech;
	ComPtr<ID3DX11EffectTechnique> m_light3TexTech;

	ComPtr<ID3DX11EffectMatrixVariable> m_worldViewProj;
	ComPtr<ID3DX11EffectMatrixVariable> m_world;
	ComPtr<ID3DX11EffectMatrixVariable> m_worldInvTranspose;
	ComPtr<ID3DX11EffectMatrixVariable> m_texTransform;
	ComPtr<ID3DX11EffectVectorVariable> m_eyePosW;
	ComPtr<ID3DX11EffectVariable> m_dirLights;
	ComPtr<ID3DX11EffectVariable> m_mat;

	ComPtr<ID3DX11EffectShaderResourceVariable> m_diffuseMap;
};

class CEffects
{
public:
	static void initAll(ID3D11Device* device);

	static std::unique_ptr<CBasicEffect> ms_basicFX;
};
