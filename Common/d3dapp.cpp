#include "d3dapp.h"

#include <cassert>
#include <windowsx.h>
#include <vector>

LRESULT CALLBACK CD3DApp::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CD3DApp* thisPtr = nullptr;

	if (msg == WM_CREATE)
	{
		CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
		thisPtr = (CD3DApp*)createStruct->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)thisPtr);

		thisPtr->m_mainHwnd = hwnd;
	}
	else
	{
		thisPtr = (CD3DApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (thisPtr)
	{
		return thisPtr->handleMessage(msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

CD3DApp::CD3DApp(HINSTANCE hInstance) :
	m_appInst(hInstance),
	m_mainHwnd(nullptr),
	m_isAppPaused(false),
	m_isMinimized(false),
	m_isMaximized(false),
	m_isResizing(false),
	m_4xMsaaQuality(0),
	m_mainWndCaption(L"D3D11 App"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_clientWidth(720),
	m_clientHeight(480),
	m_is4xMsaaEnabled(false),
	m_isAltEnterDisabled(false)
{
	// TODO: Double check if this is necessary
	ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
}

CD3DApp::~CD3DApp()
{
	// TODO: Check reasoning
	if (m_d3dImmediateContext)
	{
		m_d3dImmediateContext->ClearState();
	}

	// TODO: Investigate the issue
	SetWindowLongPtr(m_mainHwnd, GWLP_USERDATA, (LONG_PTR)nullptr);
}

HINSTANCE CD3DApp::getAppInst() const
{
	return m_appInst;
}

HWND CD3DApp::getMainWnd() const
{
	return m_mainHwnd;
}

float CD3DApp::getAspectRatio() const
{
	return static_cast<float>(m_clientWidth) / m_clientHeight;
}

int CD3DApp::run()
{
	MSG msg = {};

	m_timer.reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer.tick();
			if (!m_isAppPaused)
			{
				calculateFrameStats();
				update(m_timer);
				draw(m_timer);
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool CD3DApp::initialize()
{
	if (!initMainWindow())
	{
		return false;
	}

	if (!initDirect3D())
	{
		return false;
	}

	return true;
}

LRESULT CD3DApp::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_isAppPaused = true;
			m_timer.stop();
		}
		else
		{
			m_isAppPaused = false;
			m_timer.start();
		}
		return 0;

	case WM_SIZE:
		m_clientWidth = LOWORD(lParam);
		m_clientHeight = HIWORD(lParam);
		if (m_d3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_isAppPaused = true;
				m_isMinimized = true;
				m_isMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_isAppPaused = false;
				m_isMinimized = false;
				m_isMaximized = true;
				onResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (m_isMinimized)
				{
					m_isAppPaused = false;
					m_isMinimized = false;
					onResize();
				}
				else if (m_isMaximized)
				{
					m_isAppPaused = false;
					m_isMaximized = false;
					onResize();
				}
				else if (m_isResizing)
				{
					// pass
				}
				else
				{
					onResize();
				}
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		m_isAppPaused = true;
		m_isResizing = true;
		m_timer.stop();
		return 0;

	case WM_EXITSIZEMOVE:
		m_isAppPaused = false;
		m_isResizing = false;
		m_timer.start();
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


	return DefWindowProc(m_mainHwnd, msg, wParam, lParam);
}

void CD3DApp::onResize()
{
	assert(m_d3dDevice);
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_depthStencilBuffer.Reset();

	ThrowIfFailed(m_swapChain->ResizeBuffers(
		1,
		m_clientWidth,
		m_clientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0
	));

	// 5. Create the Render Target View

	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
	ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		m_renderTargetView.GetAddressOf()
	));

	// 6. Create the Depth/Stencil Buffer and View

	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = m_clientWidth;
	dsDesc.Height = m_clientHeight;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.SampleDesc.Count = m_is4xMsaaEnabled ? 4 : 1;
	dsDesc.SampleDesc.Quality = m_is4xMsaaEnabled ? (m_4xMsaaQuality - 1) : 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	ThrowIfFailed(m_d3dDevice->CreateTexture2D(
		&dsDesc,
		nullptr,
		m_depthStencilBuffer.GetAddressOf()
	));
	ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(
		m_depthStencilBuffer.Get(),
		nullptr,
		m_depthStencilView.GetAddressOf()
	));

	// 7. Bind the Views to the Output Merger Stage

	m_d3dImmediateContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
	);

	// 8. Set the Viewport

	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_d3dImmediateContext->RSSetViewports(1, &m_screenViewport);
}

bool CD3DApp::initMainWindow()
{
	WNDCLASS wc = {};
	wc.lpfnWndProc = CD3DApp::windowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return false;
	}

	m_mainHwnd = CreateWindowEx(
		0,
		L"D3DWndClassName",
		m_mainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		this
	);
	if (!m_mainHwnd)
	{
		MessageBox(0, L"CreateWindowEx Failed", 0, 0);
		return false;
	}

	ShowWindow(m_mainHwnd, SW_SHOW);

	return true;
}

bool CD3DApp::initDirect3D()
{
	// 1. Create the Device and Context

	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	ThrowIfFailed(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&m_d3dDevice,
		&featureLevel,
		&m_d3dImmediateContext
	));

	if (featureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(nullptr, L"Direct3D Feature Level 11 unsupported", 0, 0);
		return false;
	}

#ifdef _DEBUG
	logAdapters();
#endif

	// 2. Check 4X MSAA Quality Support

	ThrowIfFailed(m_d3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		4,
		&m_4xMsaaQuality
	));
	assert(m_4xMsaaQuality > 0);

	// 3. Describe the Swap Chain

	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = m_clientWidth;
	scDesc.BufferDesc.Height = m_clientHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.SampleDesc.Count = m_is4xMsaaEnabled ? 4 : 1;
	scDesc.SampleDesc.Quality = m_is4xMsaaEnabled ? (m_4xMsaaQuality - 1) : 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 1;
	scDesc.OutputWindow = m_mainHwnd;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = 0;

	// 4. Create the Swap Chain

	ComPtr<IDXGIFactory> dxgiFactory = getFactory();
	ThrowIfFailed(dxgiFactory->CreateSwapChain(
		m_d3dDevice.Get(),
		&scDesc,
		m_swapChain.GetAddressOf()
	));

	if (m_isAltEnterDisabled)
	{
		ThrowIfFailed(dxgiFactory->MakeWindowAssociation(
			m_mainHwnd,
			DXGI_MWA_NO_WINDOW_CHANGES
		));
	}

	onResize();

	return true;
}

void CD3DApp::calculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if (m_timer.getTotalTime() - timeElapsed >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = m_mainWndCaption +
			L" fps: " + fpsStr +
			L" mspf" + mspfStr;

		SetWindowText(m_mainHwnd, windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void CD3DApp::logAdapters()
{
	ComPtr<IDXGIFactory> dxgiFactory = getFactory();

	UINT i = 0;
	ComPtr<IDXGIAdapter> adapter;
	std::vector<ComPtr<IDXGIAdapter>> adapterList;
	while (dxgiFactory->EnumAdapters(i, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);
		i++;
	}

	for (const auto& adapter : adapterList)
	{
		logAdapterOutputs(adapter.Get());
	}
}

void CD3DApp::logAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	ComPtr<IDXGIOutput> output;
	while (adapter->EnumOutputs(i, output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";

		OutputDebugString(text.c_str());

		logOutputDisplayModes(output.Get());
		i++;
	}
}

void CD3DApp::logOutputDisplayModes(IDXGIOutput* output)
{
	UINT num = 0;
	output->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&num,
		nullptr
	);

	std::vector<DXGI_MODE_DESC> modeList(num);
	output->GetDisplayModeList(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0,
		&num,
		modeList.data()
	);

	for (const auto& mode : modeList)
	{
		UINT n = mode.RefreshRate.Numerator;
		UINT d = mode.RefreshRate.Denominator;

		std::wstring text =
			L"Width: " + std::to_wstring(mode.Width) + L" " +
			L"Height: " + std::to_wstring(mode.Height) + L" " +
			L"Refresh: " + std::to_wstring(n) + L"/" + std::to_wstring(d) + L"\n";

		OutputDebugString(text.c_str());

	}
}

ComPtr<IDXGIFactory> CD3DApp::getFactory() const
{
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter)));

	ComPtr<IDXGIFactory> dxgiFactory;
	ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

	return dxgiFactory;
}
