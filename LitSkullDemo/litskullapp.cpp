#include "litskullapp.h"

#include <fstream>
#include <array>
#include <DirectXColors.h>

#include "../Common/mathhelper.h"
#include "../Common/geometrygenerator.h"
#include "effects.h"
#include "vertex.h"

CLitSkullApp::CLitSkullApp(HINSTANCE hInstance) :
	CD3DApp(hInstance),
	m_gridIndexCount(0),
	m_theta(1.5f * XM_PI),
	m_phi(0.1f * XM_PI),
	m_radius(5.0f)
{
	m_mainWndCaption = L"LitSkull Demo";

	m_lastMousePos = { 0, 0 };

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_gridWorld, I);
	XMStoreFloat4x4(&m_view, I);
	XMStoreFloat4x4(&m_proj, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&m_boxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&m_skullWorld, XMMatrixMultiply(skullScale, skullOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&m_cylinderWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&m_cylinderWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&m_sphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&m_sphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}

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

	m_gridMat.m_ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_gridMat.m_diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	m_gridMat.m_specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_cylinderMat.m_ambient = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	m_cylinderMat.m_diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	m_cylinderMat.m_specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	m_sphereMat.m_ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
	m_sphereMat.m_diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
	m_sphereMat.m_specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

	m_boxMat.m_ambient = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	m_boxMat.m_diffuse = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	m_boxMat.m_specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_skullMat.m_ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_skullMat.m_diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_skullMat.m_specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

CLitSkullApp::~CLitSkullApp()
{

}

bool CLitSkullApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	CEffects::initAll(m_d3dDevice.Get());
	CInputLayouts::initAll(m_d3dDevice.Get());

	buildShapeGeometryBuffers();
	buildSkullGeometryBuffers();

	return true;
}

void CLitSkullApp::update(const CGameTimer& timer)
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

	if (GetAsyncKeyState('0') & 0x8000)
	{
		m_lightCount = 0;
	}
	if (GetAsyncKeyState('1') & 0x8000)
	{
		m_lightCount = 1;
	}
	if (GetAsyncKeyState('2') & 0x8000)
	{
		m_lightCount = 2;
	}
	if (GetAsyncKeyState('3') & 0x8000)
	{
		m_lightCount = 3;
	}
}

void CLitSkullApp::draw(const CGameTimer& timer)
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

	UINT stride = sizeof(Vertex::SPosNormal);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX viewProj = view * proj;

	CEffects::ms_basicFX->setDirLights(m_dirLights);
	CEffects::ms_basicFX->setEyePosW(m_eyePosW);

	ComPtr<ID3DX11EffectTechnique> activeTech = CEffects::ms_basicFX->m_light1Tech;
	switch (m_lightCount)
	{
	case 1:
		activeTech = CEffects::ms_basicFX->m_light1Tech;
		break;
	case 2:
		activeTech = CEffects::ms_basicFX->m_light2Tech;
		break;
	case 3:
		activeTech = CEffects::ms_basicFX->m_light3Tech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_shapesVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_shapesIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		XMMATRIX world = XMLoadFloat4x4(&m_gridWorld);
		XMMATRIX worldInvTranspose = MathHelper::inverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setMaterial(m_gridMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(
			m_gridIndexCount, m_gridIndexOffset, m_gridVertexOffset);

		world = XMLoadFloat4x4(&m_boxWorld);
		worldInvTranspose = MathHelper::inverseTranspose(world);
		worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setMaterial(m_boxMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(
			m_boxIndexCount, m_boxIndexOffset, m_boxVertexOffset);

		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&m_cylinderWorld[i]);
			worldInvTranspose = MathHelper::inverseTranspose(world);
			worldViewProj = world * view * proj;

			CEffects::ms_basicFX->setWorld(world);
			CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
			CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
			CEffects::ms_basicFX->setMaterial(m_cylinderMat);

			activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
			m_d3dImmediateContext->DrawIndexed(
				m_cylinderIndexCount, m_cylinderIndexOffset, m_cylinderVertexOffset);
		}

		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&m_sphereWorld[i]);
			worldInvTranspose = MathHelper::inverseTranspose(world);
			worldViewProj = world * view * proj;

			CEffects::ms_basicFX->setWorld(world);
			CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
			CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
			CEffects::ms_basicFX->setMaterial(m_sphereMat);

			activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
			m_d3dImmediateContext->DrawIndexed(
				m_sphereIndexCount, m_sphereIndexOffset, m_sphereVertexOffset);
		}

		m_d3dImmediateContext->IASetVertexBuffers(
			0, 1, m_skullVB.GetAddressOf(), &stride, &offset
		);
		m_d3dImmediateContext->IASetIndexBuffer(
			m_skullIB.Get(), DXGI_FORMAT_R32_UINT, 0
		);

		world = XMLoadFloat4x4(&m_skullWorld);
		worldInvTranspose = MathHelper::inverseTranspose(world);
		worldViewProj = world * view * proj;

		CEffects::ms_basicFX->setWorld(world);
		CEffects::ms_basicFX->setWorldInvTranspose(worldInvTranspose);
		CEffects::ms_basicFX->setWorldViewProj(worldViewProj);
		CEffects::ms_basicFX->setMaterial(m_skullMat);

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext.Get());
		m_d3dImmediateContext->DrawIndexed(
			m_skullIndexCount, 0, 0);
	}

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CLitSkullApp::onResize()
{
	CD3DApp::onResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		XM_PIDIV4, getAspectRatio(), 1.0f, 1000.0f
	);
	XMStoreFloat4x4(&m_proj, P);
}

void CLitSkullApp::onMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos = { x, y };
	SetCapture(m_mainHwnd);
}

void CLitSkullApp::onMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CLitSkullApp::onMouseMove(WPARAM btnState, int x, int y)
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

void CLitSkullApp::buildShapeGeometryBuffers()
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
	m_gridIndexOffset = m_boxIndexOffset;
	m_sphereIndexOffset = m_gridIndexOffset + m_gridIndexCount;
	m_cylinderIndexOffset = m_sphereIndexOffset + m_sphereIndexCount;

	UINT totalVertexCount =
		box.m_vertices.size() +
		grid.m_vertices.size() +
		sphere.m_vertices.size() +
		cylinder.m_vertices.size();

	UINT totalIndexCount =
		m_boxIndexCount +
		m_gridIndexCount +
		m_sphereIndexCount +
		m_cylinderIndexCount;

	std::vector<Vertex::SPosNormal> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = box.m_vertices[i].m_position;
		vertices[k].m_normal = box.m_vertices[i].m_normal;
	}

	for (size_t i = 0; i < grid.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = grid.m_vertices[i].m_position;
		vertices[k].m_normal = grid.m_vertices[i].m_normal;
	}

	for (size_t i = 0; i < sphere.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = sphere.m_vertices[i].m_position;
		vertices[k].m_normal = sphere.m_vertices[i].m_normal;
	}

	for (size_t i = 0; i < cylinder.m_vertices.size(); ++i, ++k)
	{
		vertices[k].m_pos = cylinder.m_vertices[i].m_position;
		vertices[k].m_normal = cylinder.m_vertices[i].m_normal;
	}

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(Vertex::SPosNormal) * totalVertexCount;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = vertices.data();

	ThrowIfFailed(m_d3dDevice->CreateBuffer(
		&vbDesc,
		&vInitData,
		m_shapesVB.GetAddressOf()
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
		m_shapesIB.GetAddressOf()
	));
}

void CLitSkullApp::buildSkullGeometryBuffers()
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

	std::vector<Vertex::SPosNormal> vertices(vCount);
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
	vbDesc.ByteWidth = vCount * sizeof(Vertex::SPosNormal);
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
