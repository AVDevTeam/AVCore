#pragma once
#include "PluginInterface.h"
#include <string>
#include <list>

class PluginManager : public IManager
{
public:
	IPlugin * loadPlugin(char* path);
	int registerCallback(IPlugin * plugin, int callbackId);
	AV_EVENT_RETURN_STATUS processEvent(void*);


private:
	std::list<std::pair<IPlugin *, int>> callbacks;
};