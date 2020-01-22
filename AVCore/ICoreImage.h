#pragma once
#include "PluginInterface.h"
#include "SettingsManager.h"
#include "CommandsManager.h"
#include "MessageManager.h"

class ICoreImage
{
public:
	virtual ~ICoreImage() {}
	virtual ILogger * getLogger() = 0;
	virtual SettingsManager * getSettingsManager() = 0;
	virtual CommandsManager * getCommandsManager() = 0;
	virtual MessageManager * getMessageManager() = 0;
};