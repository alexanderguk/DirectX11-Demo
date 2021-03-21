#pragma once

#include "../Common/d3dapp.h"

class CInitDirect3DApp : public CD3DApp
{
public:
	CInitDirect3DApp(HINSTANCE hInstance);
	virtual ~CInitDirect3DApp() override;

	virtual bool initialize() override;

protected:
	virtual void update(const CGameTimer& timer) override;
	virtual void draw(const CGameTimer& timer) override;
	virtual void onResize() override;
};
