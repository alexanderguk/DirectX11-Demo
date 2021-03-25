#include "geometrygenerator.h"

#include "mathhelper.h"
using namespace GeometryGenerator;

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

void buildCylinderTopCap(
	float bottomRadius,
	float topRadius,
	float height,
	UINT sliceCount,
	UINT stackCount,
	SMeshData& meshData)
{
	const UINT baseIndex = (UINT)meshData.m_vertices.size();
	const float y = 0.5f * height;
	const float dTheta = 2.0f * XM_PI / sliceCount;

	for (UINT i = 0; i <= sliceCount; ++i)
	{
		const float x = topRadius * cosf(i * dTheta);
		const float z = topRadius * sinf(i * dTheta);

		const float u = x / height + 0.5f;
		const float v = z / height + 0.5f;

		meshData.m_vertices.push_back(SVertex(
			x, y, z,
			0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			u, v
		));
	}

	meshData.m_vertices.push_back(SVertex(
		0.0f, y, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.5f, 0.5f
	));

	const UINT centerIndex = (UINT)meshData.m_vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.m_indices.push_back(centerIndex);
		meshData.m_indices.push_back(baseIndex + i + 1);
		meshData.m_indices.push_back(baseIndex + i);
	}
}

void buildCylinderBottomCap(
	float bottomRadius,
	float topRadius,
	float height,
	UINT sliceCount,
	UINT stackCount,
	SMeshData& meshData)
{
	const UINT baseIndex = (UINT)meshData.m_vertices.size();
	const float y = -0.5f * height;
	const float dTheta = 2.0f * XM_PI / sliceCount;

	for (UINT i = 0; i <= sliceCount; ++i)
	{
		const float x = bottomRadius * cosf(i * dTheta);
		const float z = bottomRadius * sinf(i * dTheta);

		const float u = x / height + 0.5f;
		const float v = z / height + 0.5f;

		meshData.m_vertices.push_back(SVertex(
			x, y, z,
			0.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			u, v
		));
	}

	meshData.m_vertices.push_back(SVertex(
		0.0f, y, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.5f, 0.5f
	));

	const UINT centerIndex = (UINT)meshData.m_vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.m_indices.push_back(centerIndex);
		meshData.m_indices.push_back(baseIndex + i);
		meshData.m_indices.push_back(baseIndex + i + 1);
	}
}

void GeometryGenerator::createCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, SMeshData& meshData)
{
	meshData.m_vertices.clear();
	meshData.m_indices.clear();

	const float stackHeight = height / stackCount;
	const float radiusStep = (topRadius - bottomRadius) / stackCount;
	const UINT ringCount = stackCount + 1;

	for (UINT i = 0; i < ringCount; ++i)
	{
		const float y = -0.5f * height + i * stackHeight;
		const float r = bottomRadius + i * radiusStep;

		const float dTheta = XM_2PI / sliceCount;
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			SVertex vertex;

			const float c = cosf(j * dTheta);
			const float s = sinf(j * dTheta);

			vertex.m_position = XMFLOAT3(r * c, y, r * s);

			vertex.m_texC.x = (float)j / sliceCount;
			vertex.m_texC.y = (float)i / stackCount;

			vertex.m_tangentU = XMFLOAT3(-s, 0.0f, c);

			const float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr * c, -height, dr * s);

			XMVECTOR T = XMLoadFloat3(&vertex.m_tangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.m_normal, N);

			meshData.m_vertices.push_back(vertex);
		}
	}

	const UINT ringVertexCount = sliceCount + 1;

	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.m_indices.push_back(i * ringVertexCount + j);
			meshData.m_indices.push_back((i + 1) * ringVertexCount + j);
			meshData.m_indices.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.m_indices.push_back(i * ringVertexCount + j);
			meshData.m_indices.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.m_indices.push_back(i * ringVertexCount + j + 1);
		}
	}

	buildCylinderTopCap(bottomRadius, topRadius, height,
		sliceCount, stackCount, meshData);
	buildCylinderBottomCap(bottomRadius, topRadius, height,
		sliceCount, stackCount, meshData);
}

void GeometryGenerator::createBox(float width, float height, float depth, SMeshData& meshData)
{
	SVertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = SVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = SVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = SVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = SVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = SVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = SVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = SVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = SVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = SVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = SVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = SVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = SVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = SVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = SVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = SVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = SVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = SVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = SVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = SVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = SVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = SVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = SVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = SVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = SVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.m_vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.m_indices.assign(&i[0], &i[36]);
}

void GeometryGenerator::createSphere(float radius, UINT sliceCount, UINT stackCount, SMeshData& meshData)
{
	meshData.m_vertices.clear();
	meshData.m_indices.clear();

	SVertex topVertex(
		0.0f, +radius, 0.0f,
		0.0f, +1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f
	);
	SVertex bottomVertex(
		0.0f, -radius, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f
	);

	meshData.m_vertices.push_back(topVertex);

	const float phiStep = XM_PI / stackCount;
	const float thetaStep = 2.0f * XM_PI / sliceCount;

	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		const float phi = i * phiStep;

		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			SVertex v;

			v.m_position.x = radius * sinf(phi) * cosf(theta);
			v.m_position.y = radius * cosf(phi);
			v.m_position.z = radius * sinf(phi) * sinf(theta);

			v.m_tangentU.x = -radius * sinf(phi) * sinf(theta);
			v.m_tangentU.y = 0.0f;
			v.m_tangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.m_tangentU);
			XMStoreFloat3(&v.m_tangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.m_position);
			XMStoreFloat3(&v.m_normal, XMVector3Normalize(p));

			v.m_texC.x = theta / XM_2PI;
			v.m_texC.y = phi / XM_PI;

			meshData.m_vertices.push_back(v);
		}
	}

	meshData.m_vertices.push_back(bottomVertex);

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		meshData.m_indices.push_back(0);
		meshData.m_indices.push_back(i + 1);
		meshData.m_indices.push_back(i);
	}

	UINT baseIndex = 1;
	const UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.m_indices.push_back(baseIndex + i * ringVertexCount + j);
			meshData.m_indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.m_indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.m_indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.m_indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.m_indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	const UINT southPoleIndex = (UINT)meshData.m_vertices.size() - 1;

	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.m_indices.push_back(southPoleIndex);
		meshData.m_indices.push_back(baseIndex + i);
		meshData.m_indices.push_back(baseIndex + i + 1);
	}
}

void subdivide(SMeshData& meshData)
{
	SMeshData inputCopy = meshData;

	meshData.m_vertices.resize(0);
	meshData.m_indices.resize(0);

	const UINT numTris = inputCopy.m_indices.size() / 3;
	for (UINT i = 0; i < numTris; ++i)
	{
		SVertex v0 = inputCopy.m_vertices[inputCopy.m_indices[i * 3 + 0]];
		SVertex v1 = inputCopy.m_vertices[inputCopy.m_indices[i * 3 + 1]];
		SVertex v2 = inputCopy.m_vertices[inputCopy.m_indices[i * 3 + 2]];

		SVertex m0, m1, m2;

		m0.m_position = XMFLOAT3(
			0.5f * (v0.m_position.x + v1.m_position.x),
			0.5f * (v0.m_position.y + v1.m_position.y),
			0.5f * (v0.m_position.z + v1.m_position.z)
		);
		m1.m_position = XMFLOAT3(
			0.5f * (v1.m_position.x + v2.m_position.x),
			0.5f * (v1.m_position.y + v2.m_position.y),
			0.5f * (v1.m_position.z + v2.m_position.z)
		);
		m2.m_position = XMFLOAT3(
			0.5f * (v0.m_position.x + v2.m_position.x),
			0.5f * (v0.m_position.y + v2.m_position.y),
			0.5f * (v0.m_position.z + v2.m_position.z)
		);

		meshData.m_vertices.push_back(v0);
		meshData.m_vertices.push_back(v1);
		meshData.m_vertices.push_back(v2);
		meshData.m_vertices.push_back(m0);
		meshData.m_vertices.push_back(m1);
		meshData.m_vertices.push_back(m2);

		meshData.m_indices.push_back(i * 6 + 0);
		meshData.m_indices.push_back(i * 6 + 3);
		meshData.m_indices.push_back(i * 6 + 5);

		meshData.m_indices.push_back(i * 6 + 3);
		meshData.m_indices.push_back(i * 6 + 4);
		meshData.m_indices.push_back(i * 6 + 5);

		meshData.m_indices.push_back(i * 6 + 5);
		meshData.m_indices.push_back(i * 6 + 4);
		meshData.m_indices.push_back(i * 6 + 2);

		meshData.m_indices.push_back(i * 6 + 3);
		meshData.m_indices.push_back(i * 6 + 1);
		meshData.m_indices.push_back(i * 6 + 4);
	}
}

void GeometryGenerator::createGeosphere(float radius, UINT numSubdivisions, SMeshData& meshData)
{
	numSubdivisions = MathHelper::min(numSubdivisions, 5u);

	const float X = 0.525731f;
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] = {
		XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X), XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f), XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f), XMFLOAT3(-Z, -X, 0.0f),
	};

	DWORD k[60] = {
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7,
	};

	meshData.m_vertices.resize(12);
	meshData.m_indices.resize(60);

	for (size_t i = 0; i < 12; ++i)
	{
		meshData.m_vertices[i].m_position = pos[i];
	}
	for (size_t i = 0; i < 60; ++i)
	{
		meshData.m_indices[i] = k[i];
	}

	for (size_t i = 0; i < numSubdivisions; ++i)
	{
		subdivide(meshData);
	}

	for (size_t i = 0; i < meshData.m_vertices.size(); ++i)
	{
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(
			&meshData.m_vertices[i].m_position
		));

		XMVECTOR p = radius * n;

		XMStoreFloat3(&meshData.m_vertices[i].m_position, p);
		XMStoreFloat3(&meshData.m_vertices[i].m_normal, n);

		const float theta = MathHelper::angleFromXY(
			meshData.m_vertices[i].m_position.x,
			meshData.m_vertices[i].m_position.z
		);

		const float phi = acosf(meshData.m_vertices[i].m_position.y / radius);

		meshData.m_vertices[i].m_texC.x = theta / XM_2PI;
		meshData.m_vertices[i].m_texC.y = phi / XM_PI;

		meshData.m_vertices[i].m_tangentU.x = -radius * sinf(phi) * sinf(theta);
		meshData.m_vertices[i].m_tangentU.y = 0.0f;
		meshData.m_vertices[i].m_tangentU.z = +radius * sinf(phi) * cosf(theta);

		XMVECTOR T = XMLoadFloat3(&meshData.m_vertices[i].m_tangentU);
		XMStoreFloat3(
			&meshData.m_vertices[i].m_tangentU,
			XMVector3Normalize(T)
		);
	}
}
