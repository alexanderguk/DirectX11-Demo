#pragma once

#include "../Common/d3dapp.h"

using namespace DirectX;

struct SVertex
{
	XMFLOAT3 m_pos;
	XMFLOAT4 m_color;
};

class CShapesApp : public CD3DApp
{
public:
	CShapesApp(HINSTANCE hInstance);
	virtual ~CShapesApp() override;

	virtual bool initialize() override;

protected:
	virtual void update(const CGameTimer& timer) override;
	virtual void draw(const CGameTimer& timer) override;
	virtual void onResize() override;

	virtual void onMouseDown(WPARAM btnState, int x, int y) override;
	virtual void onMouseUp(WPARAM btnState, int x, int y) override;
	virtual void onMouseMove(WPARAM btnState, int x, int y) override;

private:
	float getHeight(float x, float z) const;
	void buildGeometryBuffers();
	void buildFX();
	void buildVertexLayout();

protected:
	ComPtr<ID3D11Buffer> m_VB;
	ComPtr<ID3D11Buffer> m_IB;

	ComPtr<ID3DX11Effect> m_FX;
	ComPtr<ID3DX11EffectTechnique> m_tech;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldViewProj;

	ComPtr<ID3D11InputLayout> m_inputLayout;

	ComPtr<ID3D11RasterizerState> m_wireframeRS;

	XMFLOAT4X4 m_sphereWorld[10];
	XMFLOAT4X4 m_cylWorld[10];
	XMFLOAT4X4 m_boxWorld;
	XMFLOAT4X4 m_gridWorld;
	XMFLOAT4X4 m_centerSphere;

	UINT m_boxVertexOffset;
	UINT m_gridVertexOffset;
	UINT m_sphereVertexOffset;
	UINT m_cylinderVertexOffset;

	UINT m_boxIndexCount;
	UINT m_gridIndexCount;
	UINT m_sphereIndexCount;
	UINT m_cylinderIndexCount;

	UINT m_boxIndexOffset;
	UINT m_gridIndexOffset;
	UINT m_sphereIndexOffset;
	UINT m_cylinderIndexOffset;

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
