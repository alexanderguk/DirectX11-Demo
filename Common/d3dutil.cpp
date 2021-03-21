#include "d3dutil.h"

#include <comdef.h>

SDxException::SDxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber) :
	m_errorCode(hr),
	m_functionName(functionName),
	m_fileName(fileName),
	m_lineNumber(lineNumber)
{

}

std::wstring SDxException::toString() const
{
	_com_error err(m_errorCode);
	std::wstring msg = err.ErrorMessage();

	return m_functionName + L" failed in " + m_fileName + L"; line " +
		std::to_wstring(m_lineNumber) + L"; error: " + msg;
}
