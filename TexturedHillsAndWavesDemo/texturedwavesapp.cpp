#include "texturedwavesapp.h"

#include <fstream>
#include <array>
#include <DirectXColors.h>
#include "DDSTextureLoader.h"

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"
#include "effects.h"
#include "vertex.h"

CTexturedWavesApp::CTexturedWavesApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_landIndexCount(0),
	m_theta(1.5f * XM_PI),
	m_phi(0.1f * XM_PI),
	m_radius(5.0f)
{
	m_mainWndCaption = L"TexturedHillsAndWaves Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_landWorld, I);
	XMStoreFloat4x4(&m_wavesWorld, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&m_grassTexTransform, grassTexScale);

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

	m_landMat.m_ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_landMat.m_diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_landMat.m_specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_wavesMat.m_ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_wavesMat.m_diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_wavesMat.m_specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
}

CTexturedWavesApp::~CTexturedWavesApp()
{

}

bool CTexturedWavesApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	m_waves.initialize(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	CEffects::initAll(m_d3dDevice.Get());
	CInputLayouts::initAll(m_d3dDevice.Get());

	ThrowIfFailed(CreateDDSTextureFromFile(
		m_d3dDevice.Get(),
		L"Textures/grass.dds",
		nullptr,
		m_grassMapSRV.GetAddressOf()
	));

	ThrowIfFailed(CreateDDSTextureFromFile(
		m_d3dDevice.Get(),
		L"Textures/water2.dds",
		nullptr,
		m_wavesMapSRV.GetAddressOf()
	));

	buildLandGeometryBuffers();
	buildWaveGeometryBuffers();

	return true;
}

void CTexturedWavesApp::update(const CGameTimer& timer)
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

	static float t_base = 0.0f;
	if (timer.getTotalTime() - t_base >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % (m_waves.getRowCount() - 10);
		DWORD j = 5 + rand() % (m_waves.getColumnCount() - 10);

		float r = MathHelper::randF(1.0f, 2.0f);

		m_waves.disturb(i, j, r);
	}

	m_waves.update(timer.getDeltaTime());

	D3D11_MAPPED_SUBRESOURCE mappedData;
	ThrowIfFailed(m_d3dImmediateContext->Map(
		m_wavesVB.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedData
	));

	Vertex::SBasic32* v = reinterpret_cast<Vertex::SBasic32*>(mappedData.pData);
	for (UINT i = 0; i < m_waves.getVertexCount(); ++i)
	{
		v[i].m_pos = m_waves[i];
		v[i].m_normal = m_waves.getNormal(i);

		v[i].m_tex.x = 0.5f + m_waves[i].x / m_waves.getWidth();
		v[i].m_tex.y = 0.5f - m_waves[i].z / m_waves.getDepth();
	}

	m_d3dImmediateContext->Unmap(m_wavesVB.Get(), 0);

	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 5.0f);

	m_waterTexOffset.y += 0.05f * timer.getDeltaTime();
	m_waterTexOffset.x += 0.1f * timer.getDeltaTime();
	XMMATRIX wavesOffset = XMMatrixTranslation(
		m_waterTexOffset.x, m_waterTexOffset.y, 0.0f
	);

	XMStoreFloat4x4(&m_waterTexTransform, wavesScale * wavesOffset);
}

void CTexturedWavesApp::draw(const CGameTimer& timer)
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
		CInputLayouts::ms_posNormal.Get()
	);
	m_d3dImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	UINT stride = sizeof(Vertex::SBasic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX viewProj = view * proj;

	CEffects::ms_basicFX->setDirLights(m_dirLights);
	CEffects::ms_basicFX->setEyePosW(m_eyePosW);

	ComPtr<ID3DX11EffectTechnique> activeTech = CEffects::ms_basicFX->m_light3TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_landVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_landIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMMATRIX world = XMLoadFloat4x4(&m_landWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setTexTransform(XMLoadFloat4x4(&m_grassTexTransform));
		CEffects::ms_basicFX->setMaterial(m_landMat);
		CEffects::ms_basicFX->setDiffuseMap(m_grassMapSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(
			m_landIndexCount, 0, 0);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_wavesVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_wavesIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		world = XMLoadFloat4x4(&m_wavesWorld);
		worldInvTranspose = MathHelper::inverseTranspose(world);
		worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setTexTransform(XMLoadFloat4x4(&m_waterTexTransform));
		CEffects::ms_basicFX->setMaterial(m_wavesMat);
		CEffects::ms_basicFX->setDiffuseMap(m_wavesMapSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(
			3 * m_waves.getTriangleCount(), 0, 0);
	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CTexturedWavesApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CTexturedWavesApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CTexturedWavesApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CTexturedWavesApp::onMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.05f * static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - m_lastMousePos.y);

		m_radius += dx - dy;

		m_radius = MathHelper::clamp(m_radius, 50.0f, 500.0f);
	}

	m_lastMousePos = { x, y };
}

float CTexturedWavesApp::getHillHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

DirectX::XMFLOAT3 CTexturedWavesApp::getHillNormal(float x, float z) const
{
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z)
	);

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void CTexturedWavesApp::buildLandGeometryBuffers()
{
	GeometryGenerator::SMeshData grid;

	createGrid(160.0f, 160.0f, 50, 50, grid);

	m_landIndexCount = grid.m_indices.size();

	std::vector<Vertex::SBasic32> vertices(grid.m_vertices.size());
	for (size_t i = 0; i < grid.m_vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.m_vertices[i].m_position;

		p.y = getHillHeight(p.x, p.z);

		vertices[i].m_pos = p;
		vertices[i].m_normal = getHillNormal(p.x, p.z);
		vertices[i].m_tex = grid.m_vertices[i].m_texC;
	}

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(Vertex::SBasic32) * grid.m_vertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		&vInitData,
		m_landVB.GetAddressOf()
	));

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(UINT) * m_landIndexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = grid.m_indices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&ibDesc,
		&iInitData,
		m_landIB.GetAddressOf()
	));
}

void CTexturedWavesApp::buildWaveGeometryBuffers()
{
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(Vertex::SBasic32) * m_waves.getVertexCount();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.MiscFlags = 0;

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		nullptr,
		m_wavesVB.GetAddressOf()
	));

	std::vector<UINT> indices(3 * m_waves.getTriangleCount());

	const UINT m = m_waves.getRowCount();
	const UINT n = m_waves.getColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6;
		}
	}

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(UINT) * indices.size();
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&ibDesc, &iInitData, m_wavesIB.GetAddressOf()
	));
}
