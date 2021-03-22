#pragma once

#include "../Common/d3dapp.h"

using namespace DirectX;

struct SVertex
{
	XMFLOAT3 m_pos;
	XMFLOAT4 m_color;
};

class CBoxApp : public CD3DApp
{
public:
	CBoxApp(HINSTANCE hInstance);
	virtual ~CBoxApp() override;

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
	void buildFX();
	void buildVertexLayout();

protected:
	ComPtr<ID3D11Buffer> m_boxVB;
	ComPtr<ID3D11Buffer> m_boxIB;

	ComPtr<ID3DX11Effect> m_FX;
	ComPtr<ID3DX11EffectTechnique> m_tech;
	ComPtr<ID3DX11EffectMatrixVariable> m_fxWorldViewProj;

	ComPtr<ID3D11InputLayout> m_inputLayout;

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
