#pragma once

#include "../Common/d3dapp.h"
#include "../Common/lighthelper.h"

#include <vector>

using namespace DirectX;

class CCrateApp : public CD3DApp
{
public:
	CCrateApp(HINSTANCE hInstance);
	virtual ~CCrateApp() override;

	virtual bool initialize() override;

protected:
	virtual void update(const CGameTimer& timer) override;
	virtual void draw(const CGameTimer& timer) override;
	virtual void onResize() override;

	virtual void onMouseDown(WPARAM btnState, int x, int y) override;
	virtual void onMouseUp(WPARAM btnState, int x, int y) override;
	virtual void onMouseMove(WPARAM btnState, int x, int y) override;

private:
	void buildGeometryBuffers();

protected:
	ComPtr<ID3D11Buffer> m_boxVB;
	ComPtr<ID3D11Buffer> m_boxIB;

	std::vector<ComPtr<ID3D11ShaderResourceView>> m_fireMapSRVs;
	ComPtr<ID3D11ShaderResourceView> m_fireMapSRV;

	SDirectionalLight m_dirLights[3];
	SMaterial m_boxMat;

	XMFLOAT4X4 m_texTransform;
	XMFLOAT4X4 m_boxWorld;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	int m_boxVertexOffset;
	UINT m_boxIndexOffset;
	UINT m_boxIndexCount;

	XMFLOAT3 m_eyePosW;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
