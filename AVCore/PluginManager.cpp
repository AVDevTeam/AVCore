#include "PluginManager.h"
#include <Windows.h>

typedef IPlugin* (* GetPlugin)();

IPlugin* PluginManager::loadPlugin(char* path)
{
	HMODULE pluginDll = LoadLibraryA(path);
	if (pluginDll == 0)
	{
		DWORD lastError = GetLastError();
		throw "PLUGIN NOT FOUND";
	}
	GetPlugin getPlugin = (GetPlugin)GetProcAddress(pluginDll, "GetPlugin");
	if (getPlugin == 0)
		throw "PLUGIN NOT FOUND";
	IPlugin * plugin = getPlugin();
	plugin->init(this);
	return plugin;
}

int PluginManager::registerCallback(IPlugin * plugin, int callbackId, int eventType, int priority)
{
	callback newCallback = callback(callbackId, plugin);
	this->callbacksMap[eventType]->insert(std::pair<int, callback>(priority, newCallback));
	return 0;
}

AV_EVENT_RETURN_STATUS PluginManager::processEvent(int eventType, void* event)
{
	try
	{
		EventParser* eventParser = this->parsersMap[eventType];
		AvEvent* parsedEvent = eventParser->parse(event);
		priorityMap* eventPriorityMap = this->callbacksMap[eventType];
		for (priorityMap::iterator it = eventPriorityMap->begin(); it != eventPriorityMap->end(); it++)
		{
			int priority = (*it).first;
			callback curCallback = (*it).second;
			std::cout << "Processing callback with priority " << priority << "\n";
			AV_EVENT_RETURN_STATUS status = curCallback.second->callback(curCallback.first, parsedEvent);
			if (status == AvEventStatusBlock)
				return status;
		}
		return AvEventStatusAllow;
	}
	catch (const std::string& ex)
	{
		std::cout << "Exception: " << ex << "\n";
		return AvEventStatusAllow;
	}
	
}

void PluginManager::addEventParser(int eventType, EventParser* parser)
{
	this->parsersMap.insert(std::pair<int, EventParser*>(eventType, parser));
	this->callbacksMap.insert(std::pair<int, priorityMap*>(eventType, new priorityMap()));
}
