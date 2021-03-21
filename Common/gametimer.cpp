#include "gametimer.h"

#include <windows.h>

CGameTimer::CGameTimer() :
	m_secondsPerCount(0.0),
	m_deltaTime(-1.0),
	m_baseTime(0),
	m_pausedTime(0),
	m_stopTime(0),
	m_prevTime(0),
	m_currTime(0),
	m_isStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0f / (double)countsPerSec;
}

float CGameTimer::getTotalTime() const
{
	if (m_isStopped)
	{
		return (float)(((m_stopTime - m_pausedTime) -
			m_baseTime) * m_secondsPerCount);
	}
	else
	{
		return (float)(((m_currTime - m_pausedTime) -
			m_baseTime) * m_secondsPerCount);
	}
}

float CGameTimer::getDeltaTime() const
{
	return (float)m_deltaTime;
}

void CGameTimer::reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_isStopped = false;
}

void CGameTimer::start()
{
	if (!m_isStopped)
	{
		return;
	}

	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	m_pausedTime += (startTime - m_stopTime);

	m_prevTime = startTime;

	m_stopTime = 0;
	m_isStopped = false;
}

void CGameTimer::stop()
{
	if (m_isStopped)
	{
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_stopTime = currTime;
	m_isStopped = true;
}

void CGameTimer::tick()
{
	if (m_isStopped)
	{
		m_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;

	m_deltaTime = (m_currTime - m_prevTime) * m_secondsPerCount;

	m_prevTime = m_currTime;

	if (m_deltaTime < 0.0)
	{
		m_deltaTime = 0.0;
	}
}
