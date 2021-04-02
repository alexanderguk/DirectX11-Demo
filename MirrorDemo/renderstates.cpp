#include "renderstates.h"

ComPtr<ID3D11RasterizerState> CRenderStates::ms_wireframeRS = nullptr;
ComPtr<ID3D11RasterizerState> CRenderStates::ms_noCullRS = nullptr;
ComPtr<ID3D11RasterizerState> CRenderStates::ms_cullClockwiseRS = nullptr;

ComPtr<ID3D11BlendState> CRenderStates::ms_alphaToCoverageBS = nullptr;
ComPtr<ID3D11BlendState> CRenderStates::ms_transparentBS = nullptr;
ComPtr<ID3D11BlendState> CRenderStates::ms_noRenderTargetWriteBS = nullptr;

ComPtr<ID3D11DepthStencilState> CRenderStates::ms_markMirrorDSS = nullptr;
ComPtr<ID3D11DepthStencilState> CRenderStates::ms_drawReflectionDSS = nullptr;
ComPtr<ID3D11DepthStencilState> CRenderStates::ms_noDoubleBlendDSS = nullptr;

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

	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(cullClockwiseDesc));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;

	ThrowIfFailed(device->CreateRasterizerState(
		&cullClockwiseDesc, ms_cullClockwiseRS.GetAddressOf()
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

	D3D11_BLEND_DESC noRenderTargetWriteDesc = {};
	noRenderTargetWriteDesc.AlphaToCoverageEnable = false;
	noRenderTargetWriteDesc.IndependentBlendEnable = false;
	noRenderTargetWriteDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWriteDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWriteDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWriteDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWriteDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWriteDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWriteDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWriteDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	ThrowIfFailed(device->CreateBlendState(
		&noRenderTargetWriteDesc, ms_noRenderTargetWriteBS.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ThrowIfFailed(device->CreateDepthStencilState(
		&mirrorDesc, ms_markMirrorDSS.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawReflectionDesc.StencilEnable = true;
	drawReflectionDesc.StencilReadMask = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	ThrowIfFailed(device->CreateDepthStencilState(
		&drawReflectionDesc, ms_drawReflectionDSS.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable = true;
	noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable = true;
	noDoubleBlendDesc.StencilReadMask = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	ThrowIfFailed(device->CreateDepthStencilState(
		&noDoubleBlendDesc, ms_noDoubleBlendDSS.GetAddressOf()
	));
}
