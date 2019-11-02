#pragma once
#include "PluginInterface.h"

class ICoreImage
{
public:
	virtual ~ICoreImage() {}
	virtual ILogger* getLogger() = 0;
};