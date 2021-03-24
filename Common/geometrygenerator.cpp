#include "geometrygenerator.h"

GeometryGenerator::SVertex::SVertex()
{

}

GeometryGenerator::SVertex::SVertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv) :
	m_position(p),
	m_normal(n),
	m_tangentU(t),
	m_texC(uv)
{

}

GeometryGenerator::SVertex::SVertex(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty, float tz, float u, float v) :
	m_position(px, py, pz),
	m_normal(nx, ny, nz),
	m_tangentU(tx, ty, tz),
	m_texC(u, v)
{

}

void GeometryGenerator::createGrid(float width, float depth, UINT m, UINT n, SMeshData& meshData)
{
	const UINT vertexCount = m * n;
	const UINT faceCount = (m - 1) * (n - 1) * 2;

	const float halfWidth = 0.5f * width;
	const float halfDepth = 0.5 * depth;

	const float dx = width / (n - 1);
	const float dz = depth / (m - 1);

	const float du = 1.0f / (n - 1);
	const float dv = 1.0f / (m - 1);

	meshData.m_vertices.resize(vertexCount);
	for (UINT i = 0; i < m; ++i)
	{
		const float z = halfDepth - i * dz;
		for (UINT j = 0; j < n; ++j)
		{
			const float x = -halfWidth + j * dx;

			meshData.m_vertices[i * n + j].m_position = XMFLOAT3(x, 0.0f, z);

			meshData.m_vertices[i * n + j].m_normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.m_vertices[i * n + j].m_tangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			meshData.m_vertices[i * n + j].m_texC.x = j * du;
			meshData.m_vertices[i * n + j].m_texC.y = i * dv;
		}
	}

	meshData.m_indices.resize(faceCount * 3);
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			meshData.m_indices[k] = i * n + j;
			meshData.m_indices[k + 1] = i * n + j + 1;
			meshData.m_indices[k + 2] = (i + 1) * n + j;

			meshData.m_indices[k + 3] = (i + 1) * n + j;
			meshData.m_indices[k + 4] = i * n + j + 1;
			meshData.m_indices[k + 5] = (i + 1) * n + j + 1;

			k += 6;
		}
	}
}
