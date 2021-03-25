#pragma once

#include "../Common/d3dapp.h"

#include "../Common/waves.h"
#include "../Common/lighthelper.h"

using namespace DirectX;

struct SVertex
{
	XMFLOAT3 m_pos;
	XMFLOAT3 m_normal;
};

class CLightApp : public CD3DApp
{
public:
	CLightApp(HINSTANCE hInstance);
	virtual ~CLightApp() override;

	virtual bool initialize() override;

protected:
	virtual void update(const CGameTimer& timer) override;
	virtual void draw(const CGameTimer& timer) override;
	virtual void onResize() override;

	virtual void onMouseDown(WPARAM btnState, int x, int y) override;
	virtual void onMouseUp(WPARAM btnState, int x, int y) override;
	virtual void onMouseMove(WPARAM btnState, int x, int y) override;

private:
	float getHillHeight(float x, float z) const;
	XMFLOAT3 getHillNormal(float x, float z) const;
	void buildGridGeometryBuffers();
	void buildWavesGeometryBuffers();
	void buildFX();
	void buildVertexLayout();

protected:
	ComPtr<ID3D11Buffer> m_gridVB;
	ComPtr<ID3D11Buffer> m_gridIB;
	ComPtr<ID3D11Buffer> m_wavesVB;
	ComPtr<ID3D11Buffer> m_wavesIB;

	CWaves m_waves;
	SDirectionalLight m_dirLight;
	SPointLight m_pointLight;
	SSpotLight m_spotLight;
	SMaterial m_landMat;
	SMaterial m_wavesMat;

	ComPtr<ID3DX11Effect> m_FX;
	ComPtr<ID3DX11EffectTechnique> m_tech;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldViewProj;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorld;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldInvTranspose;
	ComPtr<ID3DX11EffectVectorVariable> m_fxEyePosW;
	ComPtr<ID3DX11EffectVariable> m_fxDirLight;
	ComPtr<ID3DX11EffectVariable> m_fxPointLight;
	ComPtr<ID3DX11EffectVariable> m_fxSpotLight;
	ComPtr<ID3DX11EffectVariable> m_fxMaterial;

	ComPtr<ID3D11InputLayout> m_inputLayout;

	ComPtr<ID3D11RasterizerState> m_wireframeRS;

	UINT m_gridIndexCount;

	XMFLOAT4X4 m_gridWorld;
	XMFLOAT4X4 m_wavesWorld;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	XMFLOAT3 m_eyePosW;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
