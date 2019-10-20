#pragma once
#include "PluginInterface.h"
#include "EventsUMInterfaces.h"

class TestPlugin : public IPlugin
{
public:
	AV_EVENT_RETURN_STATUS callback(int, void*);
	void init(IManager* manager);
};