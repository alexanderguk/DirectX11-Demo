#pragma once

#include <vector>
#include <DirectXMath.h>
#include <windows.h>
using namespace DirectX;

namespace GeometryGenerator
{
	struct SVertex
	{
		SVertex();
		SVertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv
		);
		SVertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v
		);

		XMFLOAT3 m_position;
		XMFLOAT3 m_normal;
		XMFLOAT3 m_tangentU;
		XMFLOAT2 m_texC;
	};

	struct SMeshData
	{
		std::vector<SVertex> m_vertices;
		std::vector<UINT> m_indices;
	};

	void createGrid(
		float width,
		float depth,
		UINT m,
		UINT n,
		SMeshData& meshData
	);

	void createCylinder(
		float bottomRadius,
		float topRadius,
		float height,
		UINT sliceCount,
		UINT stackCount,
		SMeshData& meshData
	);

	void createBox(
		float width,
		float height,
		float depth,
		SMeshData& meshData
	);

	void createSphere(
		float radius,
		UINT sliceCount,
		UINT stackCount,
		SMeshData& meshData
	);

	void createGeosphere(
		float radius,
		UINT numSubdivisions,
		SMeshData& meshData
	);
}
