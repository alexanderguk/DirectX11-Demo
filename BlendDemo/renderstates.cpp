#include "renderstates.h"

ComPtr<ID3D11RasterizerState> CRenderStates::ms_wireframeRS = nullptr;
ComPtr<ID3D11RasterizerState> CRenderStates::ms_noCullRS = nullptr;

ComPtr<ID3D11BlendState> CRenderStates::ms_alphaToCoverageBS = nullptr;
ComPtr<ID3D11BlendState> CRenderStates::ms_transparentBS = nullptr;

void CRenderStates::initAll(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(wireframeDesc));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	ThrowIfFailed(device->CreateRasterizerState(
		&wireframeDesc, ms_wireframeRS.GetAddressOf()
	));

	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(noCullDesc));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	ThrowIfFailed(device->CreateRasterizerState(
		&noCullDesc, ms_noCullRS.GetAddressOf()
	));

	D3D11_BLEND_DESC alphaToCoverageDesc = {};
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	ThrowIfFailed(device->CreateBlendState(
		&alphaToCoverageDesc, ms_alphaToCoverageBS.GetAddressOf()
	));

	D3D11_BLEND_DESC transparentDesc = {};
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	ThrowIfFailed(device->CreateBlendState(
		&transparentDesc, ms_transparentBS.GetAddressOf()
	));
}
