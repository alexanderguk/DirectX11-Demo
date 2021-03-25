#pragma once

#include <vector>
#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class CWaves
{
public:
	CWaves();
	~CWaves();

	UINT getRowCount() const;
	UINT getColumnCount() const;
	UINT getVertexCount() const;
	UINT getTriangleCount() const;

	const XMFLOAT3& operator[](int i) const;

	const XMFLOAT3& getNormal(int i) const;

	void initialize(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void update(float dt);
	void disturb(UINT i, UINT j, float magnitude);

private:
	UINT m_numRows;
	UINT m_numCols;

	UINT m_vertexCount;
	UINT m_triangleCount;

	float m_k1;
	float m_k2;
	float m_k3;

	float m_timeStep;
	float m_spatialStep;

	std::vector<XMFLOAT3> m_prevSolution;
	std::vector<XMFLOAT3> m_currSolution;
	std::vector<XMFLOAT3> m_normals;
};
