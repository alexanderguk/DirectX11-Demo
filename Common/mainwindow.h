#pragma once

#include "basewindow.h"

class CMainWindow : public CBaseWindow
{
public:
	virtual PCWSTR getClassName() const override;
	virtual LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

	float getAspectRatio() const;

protected:
	virtual void onResize();
	virtual void onMouseDown(WPARAM btnState, int x, int y);
	virtual void onMouseUp(WPARAM btnState, int x, int y);
	virtual void onMouseMove(WPARAM btnState, int x, int y);

protected:
	bool m_isAppPaused = false;
	bool m_isMinimized = false;
	bool m_isMaximized = false;
	bool m_isResizing = false;
	bool m_isFullscreen = false;

	int m_clientWidth;
	int m_clientHeight;
};
