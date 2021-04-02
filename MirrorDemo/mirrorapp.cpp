#include "mirrorapp.h"

#include <fstream>
#include <array>
#include <DirectXColors.h>
#include "DDSTextureLoader.h"

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"
#include "effects.h"
#include "vertex.h"
#include "renderstates.h"

CMirrorApp::CMirrorApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_skullIndexCount(0),
	m_skullTranslation(0.0f, 1.0f, -5.0f),
	m_theta(1.24f * XM_PI),
	m_phi(0.42f * XM_PI),
	m_radius(12.0f)
{
	m_mainWndCaption = L"Mirror Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_roomWorld, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	m_dirLights[0].m_ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[0].m_diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLights[0].m_specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLights[0].m_direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_dirLights[1].m_ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[1].m_diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	m_dirLights[1].m_specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_dirLights[1].m_direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	m_dirLights[2].m_ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].m_diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[2].m_specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].m_direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	m_roomMat.m_ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_roomMat.m_diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_roomMat.m_specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	m_skullMat.m_ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_skullMat.m_diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_skullMat.m_specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	m_mirrorMat.m_ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_mirrorMat.m_diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	m_mirrorMat.m_specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	m_shadowMat.m_ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_shadowMat.m_diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_shadowMat.m_specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

CMirrorApp::~CMirrorApp()
{

}

bool CMirrorApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	CEffects::initAll(m_d3dDevice.Get());
	CInputLayouts::initAll(m_d3dDevice.Get());
	CRenderStates::initAll(m_d3dDevice.Get());

	ThrowIfFailed(CreateDDSTextureFromFile(
		m_d3dDevice.Get(),
		L"Textures/checkboard.dds",
		nullptr,
		m_floorDiffuseMapSRV.GetAddressOf()
	));

	ThrowIfFailed(CreateDDSTextureFromFile(
		m_d3dDevice.Get(),
		L"Textures/brick01.dds",
		nullptr,
		m_wallDiffuseMapSRV.GetAddressOf()
	));

	ThrowIfFailed(CreateDDSTextureFromFile(
		m_d3dDevice.Get(),
		L"Textures/ice.dds",
		nullptr,
		m_mirrorDiffuseMapSRV.GetAddressOf()
	));

	buildRoomGeometryBuffers();
	buildSkullGeometryBuffers();

	return true;
}

void CMirrorApp::update(const CGameTimer& timer)
{
	float x = m_radius * sinf(m_phi) * cosf(m_theta);
	float z = m_radius * sinf(m_phi) * sinf(m_theta);
	float y = m_radius * cosf(m_phi);

	m_eyePosW = XMFLOAT3(x, y, z);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);

	if (GetAsyncKeyState('1') & 0x8000)
	{
		m_renderOption = ERenderOption::Lighting;
	}
	if (GetAsyncKeyState('2') & 0x8000)
	{
		m_renderOption = ERenderOption::Textures;
	}
	if (GetAsyncKeyState('3') & 0x8000)
	{
		m_renderOption = ERenderOption::TexturesAndFog;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		m_skullTranslation.x -= 1.0f * timer.getDeltaTime();
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		m_skullTranslation.x += 1.0f * timer.getDeltaTime();
	}
	if (GetAsyncKeyState('W') & 0x8000)
	{
		m_skullTranslation.y += 1.0f * timer.getDeltaTime();
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		m_skullTranslation.y -= 1.0f * timer.getDeltaTime();
	}

	m_skullTranslation.y = MathHelper::max(m_skullTranslation.y, 0.0f);

	XMMATRIX skullRotate = XMMatrixRotationY(XM_PIDIV2);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(
		m_skullTranslation.x,
		m_skullTranslation.y,
		m_skullTranslation.z
	);
	XMStoreFloat4x4(&m_skullWorld, skullRotate * skullScale * skullOffset);
}

void CMirrorApp::draw(const CGameTimer& timer)
{
	m_d3dImmediateContext->ClearRenderTargetView(
		m_renderTargetView.Get(),
		Colors::LightSteelBlue
	);
	m_d3dImmediateContext->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);

	m_d3dImmediateContext->IASetInputLayout(
		CInputLayouts::ms_basic32.Get()
	);
	m_d3dImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(Vertex::SBasic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX viewProj = view * proj;

	CEffects::ms_basicFX->setDirLights(m_dirLights);
	CEffects::ms_basicFX->setEyePosW(m_eyePosW);
	CEffects::ms_basicFX->setFogColor(Colors::Black);
	CEffects::ms_basicFX->setFogStart(2.0f);
	CEffects::ms_basicFX->setFogRange(40.0f);

	ComPtr<ID3DX11EffectTechnique> activeTech;
	ComPtr<ID3DX11EffectTechnique> activeSkullTech;

	switch (m_renderOption)
	{
	case ERenderOption::Lighting:
		activeTech = CEffects::ms_basicFX->m_light3Tech;
		activeSkullTech = CEffects::ms_basicFX->m_light3Tech;
		break;
	case ERenderOption::Textures:
		activeTech = CEffects::ms_basicFX->m_light3TexTech;
		activeSkullTech = CEffects::ms_basicFX->m_light3Tech;
		break;
	case ERenderOption::TexturesAndFog:
		activeTech = CEffects::ms_basicFX->m_light3TexFogTech;
		activeSkullTech = CEffects::ms_basicFX->m_light3FogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_roomVB.GetAddressOf(), &stride, &offset
		);

		XMMATRIX world = XMLoadFloat4x4(&m_roomWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setTexTransform(XMMatrixIdentity());
		CEffects::ms_basicFX->setMaterial(m_roomMat);

		CEffects::ms_basicFX->setDiffuseMap(m_floorDiffuseMapSRV.Get());
		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->Draw(6, 0);

		CEffects::ms_basicFX->setDiffuseMap(m_wallDiffuseMapSRV.Get());
		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->Draw(18, 6);
	}

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_skullVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_skullIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMMATRIX world = XMLoadFloat4x4(&m_skullWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setMaterial(m_skullMat);

		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_skullIndexCount, 0, 0);
	}

	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_roomVB.GetAddressOf(), &stride, &offset
		);

		XMMATRIX world = XMLoadFloat4x4(&m_roomWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setTexTransform(XMMatrixIdentity());

		m_d3dImmediateContext->OMSetBlendState(
			CRenderStates::ms_noRenderTargetWriteBS.Get(),
			blendFactor,
			0xffffffff
		);
		m_d3dImmediateContext->OMSetDepthStencilState(
			CRenderStates::ms_markMirrorDSS.Get(),
			1
		);

		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->Draw(6, 24);

		m_d3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
		m_d3dImmediateContext->OMSetBlendState(
			nullptr,
			blendFactor,
			0xffffffff
		);
	}

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_skullVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_skullIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMMATRIX R = XMMatrixReflect(mirrorPlane);
		XMMATRIX world = XMLoadFloat4x4(&m_skullWorld) * R;
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setMaterial(m_skullMat);

		XMFLOAT3 oldLightDirections[3];
		for (int i = 0; i < 3; ++i)
		{
			oldLightDirections[i] = m_dirLights[i].m_direction;

			XMVECTOR lightDir = XMLoadFloat3(&m_dirLights[i].m_direction);
			XMVECTOR reflectedLightDir = XMVector3TransformNormal(
				lightDir, R
			);
			XMStoreFloat3(&m_dirLights[i].m_direction, reflectedLightDir);
		}

		CEffects::ms_basicFX->setDirLights(m_dirLights);

		m_d3dImmediateContext->RSSetState(
			CRenderStates::ms_cullClockwiseRS.Get()
		);

		m_d3dImmediateContext->OMSetDepthStencilState(
			CRenderStates::ms_drawReflectionDSS.Get(), 1
		);
		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_skullIndexCount, 0, 0);

		m_d3dImmediateContext->RSSetState(nullptr);
		m_d3dImmediateContext->OMSetDepthStencilState(nullptr, 0);

		for (int i = 0; i < 3; ++i)
		{
			m_dirLights[i].m_direction = oldLightDirections[i];
		}

		CEffects::ms_basicFX->setDirLights(m_dirLights);
	}

	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_roomVB.GetAddressOf(), &stride, &offset
		);

		XMMATRIX world = XMLoadFloat4x4(&m_roomWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setTexTransform(XMMatrixIdentity());
		CEffects::ms_basicFX->setMaterial(m_mirrorMat);
		CEffects::ms_basicFX->setDiffuseMap(m_mirrorDiffuseMapSRV.Get());

		m_d3dImmediateContext->OMSetBlendState(
			CRenderStates::ms_transparentBS.Get(),
			blendFactor,
			0xffffffff
		);
		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->Draw(6, 24);
	}

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_skullVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_skullIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR toMainLight = -XMLoadFloat3(&m_dirLights[0].m_direction);
		XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
		XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

		XMMATRIX world = XMLoadFloat4x4(&m_skullWorld) * S * shadowOffsetY;
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setMaterial(m_shadowMat);

		m_d3dImmediateContext->OMSetDepthStencilState(
			CRenderStates::ms_noDoubleBlendDSS.Get(), 0
		);
		pass->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_skullIndexCount, 0, 0);

		m_d3dImmediateContext->OMSetBlendState(
			nullptr,
			blendFactor,
			0xffffffff
		);
		m_d3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	}


	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CMirrorApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CMirrorApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CMirrorApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CMirrorApp::onMouseMove(WPARAM btnState, int x, int y)
{
	if (btnState & MK_LBUTTON)
	{
		float dx = XMConvertToRadians(
			0.25f * static_cast<float>(x - m_lastMousePos.x));
		float dy = XMConvertToRadians(
			0.25f * static_cast<float>(y - m_lastMousePos.y));

		m_theta += dx;
		m_phi += dy;

		m_phi = MathHelper::clamp(m_phi, 0.1f, XM_PI - 0.1f);
	}
	else if (btnState & MK_RBUTTON)
	{
		float dx = 0.01f * static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - m_lastMousePos.y);

		m_radius += dx - dy;

		m_radius = MathHelper::clamp(m_radius, 3.0f, 50.0f);
	}

	m_lastMousePos = { x, y };
}

void CMirrorApp::buildRoomGeometryBuffers()
{
	Vertex::SBasic32 v[30];

	v[0] = Vertex::SBasic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = Vertex::SBasic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex::SBasic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

	v[3] = Vertex::SBasic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = Vertex::SBasic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = Vertex::SBasic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	v[6] = Vertex::SBasic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7] = Vertex::SBasic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = Vertex::SBasic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

	v[9] = Vertex::SBasic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = Vertex::SBasic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	v[11] = Vertex::SBasic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

	v[12] = Vertex::SBasic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = Vertex::SBasic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = Vertex::SBasic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[15] = Vertex::SBasic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = Vertex::SBasic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = Vertex::SBasic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[18] = Vertex::SBasic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex::SBasic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex::SBasic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

	v[21] = Vertex::SBasic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex::SBasic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	v[23] = Vertex::SBasic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

	v[24] = Vertex::SBasic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[25] = Vertex::SBasic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[26] = Vertex::SBasic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[27] = Vertex::SBasic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[28] = Vertex::SBasic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[29] = Vertex::SBasic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(Vertex::SBasic32) * 30;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = v;

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		&vInitData,
		m_roomVB.GetAddressOf()
	));
}


void CMirrorApp::buildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(nullptr, L"Models/skull.txt not found", 0, 0);
		return;
	}

	UINT vCount = 0;
	UINT tCount = 0;
	std::string ignore;

	fin >> ignore >> vCount;
	fin >> ignore >> tCount;
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	std::vector<Vertex::SBasic32> vertices(vCount);
	for (UINT i = 0; i < vCount; ++i)
	{
		fin >>
			vertices[i].m_pos.x >>
			vertices[i].m_pos.y >>
			vertices[i].m_pos.z;
		fin >>
			vertices[i].m_normal.x >>
			vertices[i].m_normal.y >>
			vertices[i].m_normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	m_skullIndexCount = 3 * tCount;
	std::vector<UINT> indices(m_skullIndexCount);
	for (UINT i = 0; i < tCount; ++i)
	{
		fin >>
			indices[i * 3 + 0] >>
			indices[i * 3 + 1] >>
			indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = vCount * sizeof(Vertex::SBasic32);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		&vInitData,
		m_skullVB.GetAddressOf()
	));

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = m_skullIndexCount * sizeof(UINT);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&ibDesc,
		&iInitData,
		m_skullIB.GetAddressOf()
	));
}
