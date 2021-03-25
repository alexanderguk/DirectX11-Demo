#include "lightapp.h"

#include <fstream>
#include <array>
#include <DirectXColors.h>

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"

CLightApp::CLightApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_gridIndexCount(0),
	m_theta(1.5f * XM_PI),
	m_phi(0.1f * XM_PI),
	m_radius(5.0f)
{
	m_mainWndCaption = L"Lighting Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMStoreFloat4x4(&m_gridWorld, I);
	XMStoreFloat4x4(&m_wavesWorld, I);

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMStoreFloat4x4(&m_wavesWorld, wavesOffset);

	m_dirLight.m_ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLight.m_diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLight.m_specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLight.m_direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	m_pointLight.m_ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pointLight.m_diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pointLight.m_specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pointLight.m_attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_pointLight.m_range = 25.0f;

	m_spotLight.m_ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_spotLight.m_diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_spotLight.m_specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_spotLight.m_attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_spotLight.m_spotExp = 96.0f;
	m_spotLight.m_range = 10000.0f;


	m_landMat.m_ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_landMat.m_diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_landMat.m_specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_wavesMat.m_ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_wavesMat.m_diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	m_wavesMat.m_specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
}

CLightApp::~CLightApp()
{

}

bool CLightApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	m_waves.initialize(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

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

void CLightApp::update(const CGameTimer& timer)
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

	SVertex* v = reinterpret_cast<SVertex*>(mappedData.pData);
	for (UINT i = 0; i < m_waves.getVertexCount(); ++i)
	{
		v[i].m_pos = m_waves[i];
		v[i].m_normal = m_waves.getNormal(i);
	}

	m_d3dImmediateContext->Unmap(m_wavesVB.Get(), 0);

	m_pointLight.m_position.x = 70.0f * cosf(0.2f * timer.getTotalTime());
	m_pointLight.m_position.z = 70.0f * sinf(0.2f * timer.getTotalTime());
	m_pointLight.m_position.y = MathHelper::max(
		getHillHeight(m_pointLight.m_position.x, m_pointLight.m_position.z),
		-3.0f
	) + 10.0f;

	m_spotLight.m_position = m_eyePosW;
	XMStoreFloat3(&m_spotLight.m_direction, XMVector3Normalize(target - pos));
}

void CLightApp::draw(const CGameTimer& timer)
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
	XMMATRIX viewProj = view * proj;

	m_fxDirLight->SetRawValue(&m_dirLight, 0, sizeof(m_dirLight));
	m_fxPointLight->SetRawValue(&m_pointLight, 0, sizeof(m_pointLight));
	m_fxSpotLight->SetRawValue(&m_spotLight, 0, sizeof(m_spotLight));
	m_fxEyePosW->SetRawValue(&m_eyePosW, 0, sizeof(m_eyePosW));

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
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		m_fxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		m_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		m_fxMaterial->SetRawValue(&m_landMat, 0, sizeof(m_landMat));

		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_gridIndexCount, 0, 0);

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_wavesVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_wavesIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		world = XMLoadFloat4x4(&m_wavesWorld);
		worldInvTranspose = MathHelper::inverseTranspose(world);
		worldViewProj = world * view * proj;


		m_fxWorld->SetMatrix(reinterpret_cast<float*>(&world));
		m_fxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		m_fxMaterial->SetRawValue(&m_wavesMat, 0, sizeof(m_wavesMat));

		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(3 * m_waves.getTriangleCount(), 0, 0);
	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CLightApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CLightApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CLightApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CLightApp::onMouseMove(WPARAM btnState, int x, int y)
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

float CLightApp::getHillHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

DirectX::XMFLOAT3 CLightApp::getHillNormal(float x, float z) const
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

void CLightApp::buildGridGeometryBuffers()
{
	GeometryGenerator::SMeshData grid;
	GeometryGenerator::createGrid(160.0f, 160.0f, 50, 50, grid);

	m_gridIndexCount = grid.m_indices.size();

	std::vector<SVertex> vertices(grid.m_vertices.size());
	for (size_t i = 0; i < grid.m_vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.m_vertices[i].m_position;

		p.y = getHillHeight(p.x, p.z);

		vertices[i].m_pos = p;
		vertices[i].m_normal = getHillNormal(p.x, p.z);
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


void CLightApp::buildWavesGeometryBuffers()
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

void CLightApp::buildFX()
{
	std::ifstream fin("fx/light.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	const int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(compiledShader.data(), size);
	fin.close();

	ThrowIfFailed(D3DX11CreateEffectFromMemory(
		compiledShader.data(),
		size,
		0,
		m_d3dDevice.Get(),
		m_FX.GetAddressOf()
	));

	m_tech = m_FX->GetTechniqueByName("LightTech");
	m_fxWorldViewProj = m_FX->GetVariableByName("g_worldViewProj")->AsMatrix();
	m_fxWorld = m_FX->GetVariableByName("g_world")->AsMatrix();
	m_fxWorldInvTranspose = m_FX->GetVariableByName("g_worldInvTranspose")->AsMatrix();
	m_fxEyePosW = m_FX->GetVariableByName("g_eyePosW")->AsVector();
	m_fxDirLight = m_FX->GetVariableByName("g_dirLight");
	m_fxPointLight = m_FX->GetVariableByName("g_pointLight");
	m_fxSpotLight = m_FX->GetVariableByName("g_spotLight");
	m_fxMaterial = m_FX->GetVariableByName("g_material");
}

void CLightApp::buildVertexLayout()
{
	std::array<D3D11_INPUT_ELEMENT_DESC, 2> vertexDesc = {
		D3D11_INPUT_ELEMENT_DESC({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0}),
		D3D11_INPUT_ELEMENT_DESC({"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
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
