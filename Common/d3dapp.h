#pragma once

#include "d3dutil.h"
#include "gametimer.h"

using Microsoft::WRL::ComPtr;

class CD3DApp
{
public:
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CD3DApp(HINSTANCE hInstance);
	virtual ~CD3DApp();

	HINSTANCE getAppInst() const;
	HWND getMainWnd() const;
	float getAspectRatio() const;

	int run();

	virtual bool initialize();

protected:
	virtual LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void update(const CGameTimer& timer) = 0;
	virtual void draw(const CGameTimer& timer) = 0;

	virtual void onResize();
	virtual void onMouseDown(WPARAM btnState, int x, int y) {};
	virtual void onMouseUp(WPARAM btnState, int x, int y) {};
	virtual void onMouseMove(WPARAM btnState, int x, int y) {};

protected:
	bool initMainWindow();
	bool initDirect3D();

	void calculateFrameStats();

protected:
	HINSTANCE m_appInst;
	HWND m_mainHwnd;
	bool m_isAppPaused;
	bool m_isMinimized;
	bool m_isMaximized;
	bool m_isResizing;
	UINT m_4xMsaaQuality;

	CGameTimer m_timer;

	ComPtr<ID3D11Device> m_d3dDevice;
	ComPtr<ID3D11DeviceContext> m_d3dImmediateContext;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	D3D11_VIEWPORT m_screenViewport;

	std::wstring m_mainWndCaption;
	D3D_DRIVER_TYPE m_d3dDriverType;

	int m_clientWidth;
	int m_clientHeight;

	bool m_is4xMsaaEnabled;
};
