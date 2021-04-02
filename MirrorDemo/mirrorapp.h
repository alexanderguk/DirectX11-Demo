#pragma once

#include "../Common/d3dapp.h"
#include "../Common/lighthelper.h"

using namespace DirectX;

enum class ERenderOption
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class CMirrorApp : public CD3DApp
{
public:
	CMirrorApp(HINSTANCE hInstance);
	virtual ~CMirrorApp() override;

	virtual bool initialize() override;

protected:
	virtual void update(const CGameTimer& timer) override;
	virtual void draw(const CGameTimer& timer) override;
	virtual void onResize() override;

	virtual void onMouseDown(WPARAM btnState, int x, int y) override;
	virtual void onMouseUp(WPARAM btnState, int x, int y) override;
	virtual void onMouseMove(WPARAM btnState, int x, int y) override;

private:
	void buildRoomGeometryBuffers();
	void buildSkullGeometryBuffers();

protected:
	ComPtr<ID3D11Buffer> m_roomVB;

	ComPtr<ID3D11Buffer> m_skullVB;
	ComPtr<ID3D11Buffer> m_skullIB;

	ComPtr<ID3D11ShaderResourceView> m_floorDiffuseMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_wallDiffuseMapSRV;
	ComPtr<ID3D11ShaderResourceView> m_mirrorDiffuseMapSRV;

	SDirectionalLight m_dirLights[3];
	SMaterial m_roomMat;
	SMaterial m_skullMat;
	SMaterial m_mirrorMat;
	SMaterial m_shadowMat;

	XMFLOAT4X4 m_roomWorld;
	XMFLOAT4X4 m_skullWorld;

	UINT m_skullIndexCount;
	XMFLOAT3 m_skullTranslation;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	ERenderOption m_renderOption;

	XMFLOAT3 m_eyePosW;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
