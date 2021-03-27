#include "crateapp.h"

#include <fstream>
#include <array>
#include <sstream>
#include <iomanip>
#include <DirectXColors.h>
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"
#include "effects.h"
#include "vertex.h"

CCrateApp::CCrateApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_boxVertexOffset(0),
	m_boxIndexOffset(0),
	m_boxIndexCount(0),
	m_theta(1.5f * XM_PI),
	m_phi(0.1f * XM_PI),
	m_radius(5.0f)
{
	m_mainWndCaption = L"Crate Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_boxWorld, I);
	XMStoreFloat4x4(&m_texTransform, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	m_dirLights[0].m_ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_dirLights[0].m_diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_dirLights[0].m_specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	m_dirLights[0].m_direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	m_dirLights[1].m_ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[1].m_diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	m_dirLights[1].m_specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	m_dirLights[1].m_direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	m_dirLights[2].m_ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].m_diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[2].m_specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].m_direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	m_boxMat.m_ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_boxMat.m_diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_boxMat.m_specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

CCrateApp::~CCrateApp()
{

}

bool CCrateApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	CEffects::initAll(m_d3dDevice.Get());
	CInputLayouts::initAll(m_d3dDevice.Get());

	m_fireMapSRVs.resize(120);
	for (int i = 1; i <= 120; ++i)
	{
		std::wstringstream ss;
		ss << std::setw(3) << std::setfill(L'0') << i;
		std::wstring s = ss.str();

		std::wstring filename = L"FireAnim/Fire" + s + L".bmp";

		ThrowIfFailed(CreateWICTextureFromFile(
			m_d3dDevice.Get(),
			filename.c_str(),
			nullptr,
			m_fireMapSRVs[i - 1].GetAddressOf()
		));
	}

	buildGeometryBuffers();

	return true;
}

void CCrateApp::update(const CGameTimer& timer)
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

	const float frameTime = 1.0f / 30.0f;
	const float totalAnimTime = 4.0f;

	static float currAnimTime = 0.0f;
	currAnimTime += timer.getDeltaTime();
	if (currAnimTime > totalAnimTime)
	{
		currAnimTime -= totalAnimTime;
	}

	const int currAnimFrame = currAnimTime / totalAnimTime * 120;

	m_fireMapSRV = m_fireMapSRVs[currAnimFrame];
}

void CCrateApp::draw(const CGameTimer& timer)
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

	ComPtr<ID3DX11EffectTechnique> activeTech = CEffects::ms_basicFX->m_light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_boxVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_boxIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMMATRIX world = XMLoadFloat4x4(&m_boxWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setTexTransform(XMLoadFloat4x4(&m_texTransform));
		CEffects::ms_basicFX->setMaterial(m_boxMat);
		CEffects::ms_basicFX->setDiffuseMap(m_fireMapSRV.Get());

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(
			m_boxIndexCount, m_boxIndexOffset, m_boxVertexOffset);
	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CCrateApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 0.1f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CCrateApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CCrateApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CCrateApp::onMouseMove(WPARAM btnState, int x, int y)
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

		m_radius = MathHelper::clamp(m_radius, 1.0f, 15.0f);
	}

	m_lastMousePos = { x, y };
}

void CCrateApp::buildGeometryBuffers()
{
	GeometryGenerator::SMeshData box;

	createBox(1.0f, 1.0f, 1.0f, box);

	m_boxVertexOffset = 0;

	m_boxIndexCount = box.m_indices.size();

	m_boxIndexOffset = 0;

	UINT totalVertexCount = box.m_vertices.size();

	UINT totalIndexCount = m_boxIndexCount;

	std::vector<Vertex::SBasic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = box.m_vertices[i].m_position;
		vertices[k].m_normal = box.m_vertices[i].m_normal;
		vertices[k].m_tex = box.m_vertices[i].m_texC;
	}

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(Vertex::SBasic32) * totalVertexCount;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		&vInitData,
		m_boxVB.GetAddressOf()
	));

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.m_indices.begin(), box.m_indices.end());

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&ibDesc,
		&iInitData,
		m_boxIB.GetAddressOf()
	));
}
