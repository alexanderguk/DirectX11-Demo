#include "skullapp.h"

#include <array>
#include <fstream>
#include <vector>
#include <DirectXColors.h>

#include "../Common/mathhelper.h"

using namespace DirectX;

CSkullApp::CSkullApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_skullIndexCount(0),
	m_theta(1.5f * XM_PI),
	m_phi(XM_PIDIV4),
	m_radius(5.0f)
{
	m_mainWndCaption = L"Skull Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_world, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);
}

CSkullApp::~CSkullApp()
{

}

bool CSkullApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	buildGeometryBuffers();
	buildFX();
	buildVertexLayout();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(wireframeDesc));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	ThrowIfFailed(m_d3dDevice->CreateRasterizerState(
		&wireframeDesc,
		m_wireframeRS.GetAddressOf()
	));

	return true;
}

void CSkullApp::update(const CGameTimer& timer)
{
	float x = m_radius * sinf(m_phi) * cosf(m_theta);
	float z = m_radius * sinf(m_phi) * sinf(m_theta);
	float y = m_radius * cosf(m_phi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);
}

void CSkullApp::draw(const CGameTimer& timer)
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

	m_d3dImmediateContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	m_d3dImmediateContext->RSSetState(m_wireframeRS.Get());

	UINT stride = sizeof(SVertex);
	UINT offset = 0;
	m_d3dImmediateContext->IASetVertexBuffers(
		0, 1, m_VB.GetAddressOf(), &stride, &offset
	);
	m_d3dImmediateContext->IASetIndexBuffer(
		m_IB.Get(), DXGI_FORMAT_R32_UINT, 0
	);

	XMMATRIX world = XMLoadFloat4x4(&m_world);
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX worldViewProj = world * view * proj;

	m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	m_tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());

		m_d3dImmediateContext->DrawIndexed(m_skullIndexCount, 0, 0);
	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CSkullApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CSkullApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CSkullApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CSkullApp::onMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.005f * static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - m_lastMousePos.y);

		m_radius += dx - dy;

		m_radius = MathHelper::clamp(m_radius, 3.0f, 15.0f);
	}

	m_lastMousePos = { x, y };
}

void CSkullApp::buildGeometryBuffers()
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

	std::vector<SVertex> vertices(vCount);
	for (UINT i = 0; i < vCount; ++i)
	{
		fin >>
			vertices[i].m_pos.x >>
			vertices[i].m_pos.y >>
			vertices[i].m_pos.z;

		vertices[i].m_color = black;

		fin >> nx >> ny >> nz;
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
	vbDesc.ByteWidth = vCount * sizeof(SVertex);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		&vInitData,
		m_VB.GetAddressOf()
	));

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = indices.size() * sizeof(UINT);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = indices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&ibDesc,
		&iInitData,
		m_IB.GetAddressOf()
	));
}

void CSkullApp::buildFX()
{
	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3D10Blob> compiledShader;
	ComPtr<ID3D10Blob> compilationMsgs;
	HRESULT hr = D3DCompileFromFile(
		L"FX/color.fx",
		nullptr,
		nullptr,
		nullptr,
		"fx_5_0",
		shaderFlags,
		0,
		compiledShader.GetAddressOf(),
		compilationMsgs.GetAddressOf()
	);

	if (compilationMsgs)
	{
		OutputDebugStringA((char*)compilationMsgs->GetBufferPointer());
	}

	ThrowIfFailed(hr);

	ThrowIfFailed(D3DX11CreateEffectFromMemory(
		compiledShader->GetBufferPointer(),
		compiledShader->GetBufferSize(),
		0,
		m_d3dDevice.Get(),
		m_FX.GetAddressOf()
	));

	m_tech = m_FX->GetTechniqueByName("ColorTech");
	m_fxWorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void CSkullApp::buildVertexLayout()
{
	std::array<D3D11_INPUT_ELEMENT_DESC, 2> vertexDesc = {
		D3D11_INPUT_ELEMENT_DESC({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0}),
		D3D11_INPUT_ELEMENT_DESC({"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
			D3D11_INPUT_PER_VERTEX_DATA, 0})
	};

	D3DX11_PASS_DESC passDesc;
	m_tech->GetPassByIndex(0)->GetDesc(&passDesc);
	ThrowIfFailed(m_d3dDevice->CreateInputLayout(
		vertexDesc.data(),
		vertexDesc.size(),
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		m_inputLayout.GetAddressOf()
	));
}
