#include "waveapp.h"

#include <array>
#include <DirectXColors.h>

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"

CWaveApp::CWaveApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_gridIndexCount(0),
	m_theta(1.5f * XM_PI),
	m_phi(XM_PIDIV4),
	m_radius(5.0f)
{
	m_mainWndCaption = L"Wave Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMStoreFloat4x4(&m_gridWorld, I);
	XMStoreFloat4x4(&m_wavesWorld, I);
}

CWaveApp::~CWaveApp()
{

}

bool CWaveApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	m_waves.initialize(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	buildGridGeometryBuffers();
	buildWavesGeometryBuffers();
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

void CWaveApp::update(const CGameTimer& timer)
{
	float x = m_radius * sinf(m_phi) * cosf(m_theta);
	float z = m_radius * sinf(m_phi) * sinf(m_theta);
	float y = m_radius * cosf(m_phi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, V);


	static float t_base = 0.0f;
	if (timer.getTotalTime() - t_base >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

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

	SVertex* v = reinterpret_cast<SVertex*>(mappedData.pData);
	for (UINT i = 0; i < m_waves.getVertexCount(); ++i)
	{
		v[i].m_pos = m_waves[i];
		v[i].m_color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	m_d3dImmediateContext->Unmap(m_wavesVB.Get(), 0);
}

void CWaveApp::draw(const CGameTimer& timer)
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

	UINT stride = sizeof(SVertex);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_gridVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_gridIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMMATRIX world = XMLoadFloat4x4(&m_gridWorld);
		XMMATRIX worldViewProj = world * view * proj;
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_gridIndexCount, 0, 0);

		m_d3dImmediateContext->RSSetState(m_wireframeRS.Get());

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_wavesVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_wavesIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		world = XMLoadFloat4x4(&m_wavesWorld);
		worldViewProj = world * view * proj;
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(3 * m_waves.getTriangleCount(), 0, 0);

		m_d3dImmediateContext->RSSetState(0);

	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CWaveApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CWaveApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CWaveApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CWaveApp::onMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.2f * static_cast<float>(x - m_lastMousePos.x);
		float dy = 0.2f * static_cast<float>(y - m_lastMousePos.y);

		m_radius += dx - dy;

		m_radius = MathHelper::clamp(m_radius, 50.0f, 500.0f);
	}

	m_lastMousePos = { x, y };
}

float CWaveApp::getHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

void CWaveApp::buildGridGeometryBuffers()
{
	GeometryGenerator::SMeshData grid;
	GeometryGenerator::createGrid(160.0f, 160.0f, 50, 50, grid);

	m_gridIndexCount = grid.m_indices.size();

	std::vector<SVertex> vertices(grid.m_vertices.size());
	for (size_t i = 0; i < grid.m_vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.m_vertices[i].m_position;

		p.y = getHeight(p.x, p.z);

		vertices[i].m_pos = p;

		if (p.y < -10.0f)
		{
			vertices[i].m_color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			vertices[i].m_color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			vertices[i].m_color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			vertices[i].m_color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			vertices[i].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(SVertex) * grid.m_vertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc, &vInitData, m_gridVB.GetAddressOf()
	));

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(UINT) * m_gridIndexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = grid.m_indices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&ibDesc, &iInitData, m_gridIB.GetAddressOf()
	));
}


void CWaveApp::buildWavesGeometryBuffers()
{
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(SVertex) * m_waves.getVertexCount();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.MiscFlags = 0;

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc, nullptr, m_wavesVB.GetAddressOf()
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

void CWaveApp::buildFX()
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

void CWaveApp::buildVertexLayout()
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
