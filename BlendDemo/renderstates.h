#pragma once

#include "../Common/d3dutil.h"
using Microsoft::WRL::ComPtr;

class CRenderStates
{
public:
	static void initAll(ID3D11Device* device);

	static ComPtr<ID3D11RasterizerState> ms_wireframeRS;
	static ComPtr<ID3D11RasterizerState> ms_noCullRS;

	static ComPtr<ID3D11BlendState> ms_alphaToCoverageBS;
	static ComPtr<ID3D11BlendState> ms_transparentBS;
};
