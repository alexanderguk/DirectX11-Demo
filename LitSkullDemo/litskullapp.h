#pragma once

#include "../Common/d3dapp.h"
#include "../Common/lighthelper.h"

using namespace DirectX;

class CLitSkullApp : public CD3DApp
{
public:
	CLitSkullApp(HINSTANCE hInstance);
	virtual ~CLitSkullApp() override;

	virtual bool initialize() override;

protected:
	virtual void update(const CGameTimer& timer) override;
	virtual void draw(const CGameTimer& timer) override;
	virtual void onResize() override;

	virtual void onMouseDown(WPARAM btnState, int x, int y) override;
	virtual void onMouseUp(WPARAM btnState, int x, int y) override;
	virtual void onMouseMove(WPARAM btnState, int x, int y) override;

private:
	void buildShapeGeometryBuffers();
	void buildSkullGeometryBuffers();

protected:
	ComPtr<ID3D11Buffer> m_shapesVB;
	ComPtr<ID3D11Buffer> m_shapesIB;

	ComPtr<ID3D11Buffer> m_skullVB;
	ComPtr<ID3D11Buffer> m_skullIB;

	SDirectionalLight m_dirLights[3];
	SMaterial m_gridMat;
	SMaterial m_boxMat;
	SMaterial m_cylinderMat;
	SMaterial m_sphereMat;
	SMaterial m_skullMat;

	XMFLOAT4X4 m_sphereWorld[10];
	XMFLOAT4X4 m_cylinderWorld[10];
	XMFLOAT4X4 m_boxWorld;
	XMFLOAT4X4 m_gridWorld;
	XMFLOAT4X4 m_skullWorld;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	int m_boxVertexOffset;
	int m_gridVertexOffset;
	int m_sphereVertexOffset;
	int m_cylinderVertexOffset;

	UINT m_boxIndexOffset;
	UINT m_gridIndexOffset;
	UINT m_sphereIndexOffset;
	UINT m_cylinderIndexOffset;

	UINT m_boxIndexCount;
	UINT m_gridIndexCount;
	UINT m_sphereIndexCount;
	UINT m_cylinderIndexCount;

	UINT m_skullIndexCount;

	UINT m_lightCount;

	XMFLOAT3 m_eyePosW;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
