#pragma once

#include "../Common/d3dutil.h"
using Microsoft::WRL::ComPtr;

class CRenderStates
{
public:
	static void initAll(ID3D11Device* device);

	static ComPtr<ID3D11RasterizerState> ms_wireframeRS;
	static ComPtr<ID3D11RasterizerState> ms_noCullRS;
	static ComPtr<ID3D11RasterizerState> ms_cullClockwiseRS;

	static ComPtr<ID3D11BlendState> ms_alphaToCoverageBS;
	static ComPtr<ID3D11BlendState> ms_transparentBS;
	static ComPtr<ID3D11BlendState> ms_noRenderTargetWriteBS;

	static ComPtr<ID3D11DepthStencilState> ms_markMirrorDSS;
	static ComPtr<ID3D11DepthStencilState> ms_drawReflectionDSS;
	static ComPtr<ID3D11DepthStencilState> ms_noDoubleBlendDSS;
};
