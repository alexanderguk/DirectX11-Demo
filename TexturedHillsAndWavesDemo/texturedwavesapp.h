#pragma once

#include "../Common/d3dapp.h"
#include "../Common/lighthelper.h"
#include "../Common/waves.h"

using namespace DirectX;

class CTexturedWavesApp : public CD3DApp
{
public:
	CTexturedWavesApp(HINSTANCE hInstance);
	virtual ~CTexturedWavesApp() override;

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
	void buildLandGeometryBuffers();
	void buildWaveGeometryBuffers();

protected:
	ComPtr<ID3D11Buffer> m_landVB;
	ComPtr<ID3D11Buffer> m_landIB;

	ComPtr<ID3D11Buffer> m_wavesVB;
	ComPtr<ID3D11Buffer> m_wavesIB;

	ComPtr<ID3D11ShaderResourceView> m_grassMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_wavesMapSRV;

	CWaves m_waves;

	SDirectionalLight m_dirLights[3];
	SMaterial m_landMat;
	SMaterial m_wavesMat;

	XMFLOAT4X4 m_grassTexTransform;
	XMFLOAT4X4 m_waterTexTransform;
	XMFLOAT4X4 m_landWorld;
	XMFLOAT4X4 m_wavesWorld;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	UINT m_landIndexCount;
	XMFLOAT2 m_waterTexOffset;

	XMFLOAT3 m_eyePosW;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
