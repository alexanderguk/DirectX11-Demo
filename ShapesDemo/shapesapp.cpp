#include "shapesapp.h"

#include <array>
#include <DirectXColors.h>

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"

CShapesApp::CShapesApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_boxVertexOffset(0),
	m_gridVertexOffset(0),
	m_sphereVertexOffset(0),
	m_cylinderVertexOffset(0),
	m_boxIndexCount(0),
	m_gridIndexCount(0),
	m_sphereIndexCount(0),
	m_cylinderIndexCount(0),
	m_boxIndexOffset(0),
	m_gridIndexOffset(0),
	m_sphereIndexOffset(0),
	m_cylinderIndexOffset(0),

	m_theta(1.5f * XM_PI),
	m_phi(XM_PIDIV4),
	m_radius(5.0f)
{
	m_mainWndCaption = L"Box Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_world, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMStoreFloat4x4(&m_gridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&m_boxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&m_centerSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&m_cylWorld[i * 2 + 0],
			XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&m_cylWorld[i * 2 + 1],
			XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&m_sphereWorld[i * 2 + 0],
			XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&m_sphereWorld[i * 2 + 1],
			XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}
}

CShapesApp::~CShapesApp()
{

}

bool CShapesApp::initialize()
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

void CShapesApp::update(const CGameTimer& timer)
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

void CShapesApp::draw(const CGameTimer& timer)
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

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX viewProj = view * proj;

	D3DX11_TECHNIQUE_DESC techDesc;
	m_tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		XMMATRIX world;

		world = XMLoadFloat4x4(&m_gridWorld);
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_gridIndexCount, m_gridIndexOffset, m_gridVertexOffset);

		world = XMLoadFloat4x4(&m_boxWorld);
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_boxIndexCount, m_boxIndexOffset, m_boxVertexOffset);

		world = XMLoadFloat4x4(&m_centerSphere);
		m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
		m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(m_sphereIndexCount, m_sphereIndexOffset, m_sphereVertexOffset);

		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&m_cylWorld[i]);
			m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
			m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
			m_d3dImmediateContext->DrawIndexed(m_cylinderIndexCount, m_cylinderIndexOffset, m_cylinderVertexOffset);
		}

		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&m_sphereWorld[i]);
			m_fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world * viewProj)));
			m_tech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
			m_d3dImmediateContext->DrawIndexed(m_sphereIndexCount, m_sphereIndexOffset, m_sphereVertexOffset);
		}
	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CShapesApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CShapesApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CShapesApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CShapesApp::onMouseMove(WPARAM btnState, int x, int y)
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

		m_radius = MathHelper::clamp(m_radius, 3.0f, 200.0f);
	}

	m_lastMousePos = { x, y };
}

float CShapesApp::getHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

void CShapesApp::buildGeometryBuffers()
{
	GeometryGenerator::SMeshData box;
	GeometryGenerator::SMeshData grid;
	GeometryGenerator::SMeshData sphere;
	GeometryGenerator::SMeshData cylinder;

	createBox(1.0f, 1.0f, 1.0f, box);
	createGrid(20.0f, 30.0f, 60, 40, grid);
	createSphere(0.5f, 20, 20, sphere);
	createCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	m_boxVertexOffset = 0;
	m_gridVertexOffset = box.m_vertices.size();
	m_sphereVertexOffset = m_gridVertexOffset + grid.m_vertices.size();
	m_cylinderVertexOffset = m_sphereVertexOffset + sphere.m_vertices.size();

	m_boxIndexCount = box.m_indices.size();
	m_gridIndexCount = grid.m_indices.size();
	m_sphereIndexCount = sphere.m_indices.size();
	m_cylinderIndexCount = cylinder.m_indices.size();

	m_boxIndexOffset = 0;
	m_gridIndexOffset = m_boxIndexCount;
	m_sphereIndexOffset = m_gridIndexOffset + m_gridIndexCount;
	m_cylinderIndexOffset = m_sphereIndexOffset + m_sphereIndexCount;

	const UINT totalVertexCount =
		box.m_vertices.size() +
		grid.m_vertices.size() +
		sphere.m_vertices.size() +
		cylinder.m_vertices.size();

	const UINT totalIndexCount =
		m_boxIndexCount +
		m_gridIndexCount +
		m_sphereIndexCount +
		m_cylinderIndexCount;

	std::vector<SVertex> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;

	for (size_t i = 0; i < box.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = box.m_vertices[i].m_position;
		vertices[k].m_color = black;
	}

	for (size_t i = 0; i < grid.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = grid.m_vertices[i].m_position;
		vertices[k].m_color = black;
	}

	for (size_t i = 0; i < sphere.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = sphere.m_vertices[i].m_position;
		vertices[k].m_color = black;
	}

	for (size_t i = 0; i < cylinder.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = cylinder.m_vertices[i].m_position;
		vertices[k].m_color = black;
	}

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(SVertex) * totalVertexCount;
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

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.m_indices.begin(), box.m_indices.end());
	indices.insert(indices.end(), grid.m_indices.begin(), grid.m_indices.end());
	indices.insert(indices.end(), sphere.m_indices.begin(), sphere.m_indices.end());
	indices.insert(indices.end(), cylinder.m_indices.begin(), cylinder.m_indices.end());

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
		m_IB.GetAddressOf()
	));
}

void CShapesApp::buildFX()
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

void CShapesApp::buildVertexLayout()
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
