#pragma once

#include <string>
#include <wrl.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

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
