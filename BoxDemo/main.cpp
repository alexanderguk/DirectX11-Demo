#include "boxapp.h"

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	LPWSTR lpCmdLine, INT nCmdShow)
{
	try
	{
		CBoxApp app(hInstance);
		if (!app.initialize())
		{
			return -1;
		}

		return app.run();
	}
	catch (SDxException& e)
	{
		MessageBox(nullptr, e.toString().c_str(), L"HR Failed", MB_OK);
		return -1;
	}
}
