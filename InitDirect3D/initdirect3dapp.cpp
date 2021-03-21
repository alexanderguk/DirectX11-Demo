#include "initdirect3dapp.h"

#include <DirectXColors.h>

using namespace DirectX;

CInitDirect3DApp::CInitDirect3DApp(HINSTANCE hInstance) :
	CD3DApp(hInstance)
{

}

CInitDirect3DApp::~CInitDirect3DApp()
{

}

bool CInitDirect3DApp::initialize()
{
	if (!CD3DApp::initialize())
	{
		return false;
	}

	return true;
}

void CInitDirect3DApp::update(const CGameTimer& timer)
{

}

void CInitDirect3DApp::draw(const CGameTimer& timer)
{
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

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

	ThrowIfFailed(m_swapChain->Present(0, 0));
}

void CInitDirect3DApp::onResize()
{
	CD3DApp::onResize();
}
