#pragma once

#include <string>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "d3dx11effect.h"

#pragma comment(lib, "d3d11.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "Effects11d.lib")
#else
#pragma comment(lib, "Effects11.lib")
#endif

inline UINT argbToAbgr(UINT argb)
{
	BYTE A = (argb >> 24) & 0xff;
	BYTE R = (argb >> 16) & 0xff;
	BYTE G = (argb >> 8) & 0xff;
	BYTE B = (argb >> 0) & 0xff;

	return (A << 24) | (B << 16) | (G << 8) | (R << 0);
}

inline std::wstring ansiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

struct SDxException
{
	SDxException() = default;
	SDxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber);

	HRESULT m_errorCode = S_OK;
	std::wstring m_functionName;
	std::wstring m_fileName;
	int m_lineNumber = -1;

	std::wstring toString() const;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)							 \
{													 \
	HRESULT hr__ = (x);								 \
	std::wstring wfn = ansiToWString(__FILE__);		 \
	if (FAILED(hr__))								 \
	{												 \
		throw SDxException(hr__, L#x, wfn, __LINE__); \
	}												 \
}													 
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif
