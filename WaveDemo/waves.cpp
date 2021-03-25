#include "waves.h"

CWaves::CWaves() :
	m_numRows(0),
	m_numCols(0),
	m_vertexCount(0),
	m_triangleCount(0),
	m_k1(0.0f),
	m_k2(0.0f),
	m_k3(0.0f),
	m_timeStep(0.0f),
	m_spatialStep(0.0f)
{

}

CWaves::~CWaves()
{

}

UINT CWaves::getRowCount() const
{
	return m_numRows;
}

UINT CWaves::getColumnCount() const
{
	return m_numCols;
}

UINT CWaves::getVertexCount() const
{
	return m_vertexCount;
}

UINT CWaves::getTriangleCount() const
{
	return m_triangleCount;
}

const DirectX::XMFLOAT3& CWaves::operator[](int i) const
{
	return m_currSolution[i];
}

void CWaves::initialize(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	m_numRows = m;
	m_numCols = n;

	m_vertexCount = m * n;
	m_triangleCount = (m - 1) * (n - 1) * 2;

	m_timeStep = dt;
	m_spatialStep = dx;

	const float d = damping * dt + 2.0f;
	const float e = (speed * speed) * (dt * dt) / (dx * dx);
	m_k1 = (damping * dt - 2.0f) / d;
	m_k2 = (4.0f - 8.0f * e) / d;
	m_k3 = (2.0f * e) / d;

	m_prevSolution.resize(m * n);
	m_currSolution.resize(m * n);

	const float halfWidth = (n - 1) * dx * 0.5f;
	const float halfDepth = (m - 1) * dx * 0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		const float z = halfDepth - i * dx;
		for (UINT j = 0; j < n; ++j)
		{
			const float x = -halfWidth + j * dx;

			m_prevSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
			m_currSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
		}
	}
}

void CWaves::update(float dt)
{
	static float t = 0.0f;

	t += dt;

	if (t >= m_timeStep)
	{
		for (DWORD i = 1; i < m_numRows - 1; ++i)
		{
			for (DWORD j = 1; j < m_numCols - 1; ++j)
			{
				m_prevSolution[i * m_numCols + j].y =
					m_k1 * m_prevSolution[i * m_numCols + j].y +
					m_k2 * m_currSolution[i * m_numCols + j].y +
					m_k3 * (
						m_currSolution[(i + 1) * m_numCols + j].y +
						m_currSolution[(i - 1) * m_numCols + j].y +
						m_currSolution[i * m_numCols + j + 1].y +
						m_currSolution[i * m_numCols + j - 1].y
						);
			}
		}

		std::swap(m_prevSolution, m_currSolution);

		t = 0.0f;
	}
}

void CWaves::disturb(UINT i, UINT j, float magnitude)
{
	assert(i > 1 && i < m_numRows - 2);
	assert(j > 1 && j < m_numCols - 2);

	const float halfMag = 0.5f * magnitude;

	m_currSolution[i * m_numCols + j].y += magnitude;
	m_currSolution[i * m_numCols + j + 1].y += halfMag;
	m_currSolution[i * m_numCols + j - 1].y += halfMag;
	m_currSolution[(i + 1) * m_numCols + j].y += halfMag;
	m_currSolution[(i - 1) * m_numCols + j].y += halfMag;
}
