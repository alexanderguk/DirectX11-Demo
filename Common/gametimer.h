#pragma once

class CGameTimer
{
public:
	CGameTimer();

	float getTotalTime() const;
	float getDeltaTime() const;

	void reset();
	void start();
	void stop();
	void tick();

private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_currTime;

	bool m_isStopped;
};
