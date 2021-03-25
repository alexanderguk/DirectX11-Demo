#include "lighthelper.h"

#include <windows.h>

SDirectionalLight::SDirectionalLight()
{
	ZeroMemory(this, sizeof(this));
}

SPointLight::SPointLight()
{
	ZeroMemory(this, sizeof(this));
}

SSpotLight::SSpotLight()
{
	ZeroMemory(this, sizeof(this));
}

SMaterial::SMaterial()
{
	ZeroMemory(this, sizeof(this));
}
