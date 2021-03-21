#include "mainwindow.h"

#include <windowsx.h>

PCWSTR CMainWindow::getClassName() const
{
	return L"Main window class";
}

LRESULT CMainWindow::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_isAppPaused = true;
		}
		else
		{
			m_isAppPaused = false;
		}
		return 0;

	case WM_SIZE:
		m_clientWidth = LOWORD(lParam);
		m_clientHeight = LOWORD(lParam);
		return 0;

	case WM_ENTERSIZEMOVE:
		m_isAppPaused = true;
		m_isResizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_isAppPaused = false;
		m_isResizing = false;
		onResize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		onMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		onMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		onMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

float CMainWindow::getAspectRatio() const
{
	return static_cast<float>(m_clientWidth) / m_clientHeight;
}


void CMainWindow::onResize()
{

}

void CMainWindow::onMouseDown(WPARAM btnState, int x, int y)
{

}

void CMainWindow::onMouseUp(WPARAM btnState, int x, int y)
{

}

void CMainWindow::onMouseMove(WPARAM btnState, int x, int y)
{

}
