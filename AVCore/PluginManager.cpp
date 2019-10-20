#include "PluginManager.h"
#include <Windows.h>

typedef IPlugin* (* GetPlugin)();

IPlugin* PluginManager::loadPlugin(std::string path)
{
	HMODULE pluginDll = LoadLibraryA(path.c_str());
	if (pluginDll == 0)
	{
		return nullptr;
	}
	GetPlugin getPlugin = (GetPlugin)GetProcAddress(pluginDll, "GetPlugin");
	if (getPlugin == 0)
		return nullptr;
	this->moduleLoadMutex.lock();
	IPlugin * plugin = getPlugin();
	plugin->init(this);
	this->loadedPlugins.insert(std::pair<std::string, IPlugin*>(plugin->getName(), plugin));
	this->moduleLoadMutex.unlock();
	return plugin;
}

void PluginManager::unloadPlugin(std::string name)
{
	this->moduleLoadMutex.lock();
	std::list<std::pair<priorityMap*, int>> toDelete;
	IPlugin* plugin = this->loadedPlugins[name];
	for (eventsMap::iterator it = this->callbacksMap.begin(); it != this->callbacksMap.end(); it++)
		for (priorityMap::iterator it2 = (*it).second->begin(); it2 != (*it).second->end(); it2++)
		{
			callback curCallback = (*it2).second;
			if (curCallback.second == plugin)
				toDelete.push_back(std::pair<priorityMap*, int>((*it).second, (*it2).first));
		}
	for (std::list<std::pair<priorityMap*, int>>::iterator it = toDelete.begin(); it != toDelete.end(); it++)
		(*it).first->erase((*it).second);
	this->loadedPlugins.erase(name);
	this->moduleLoadMutex.unlock();
}

IPlugin* PluginManager::getPluginByName(std::string name)
{
	if (this->loadedPlugins.find(name) == this->loadedPlugins.end())
		return nullptr;
	return this->loadedPlugins[name];
}

int PluginManager::registerCallback(IPlugin * plugin, int callbackId, AV_EVENT_TYPE eventType, int priority)
{
	callback newCallback = callback(callbackId, plugin);
	this->callbacksMap[eventType]->insert(std::pair<int, callback>(priority, newCallback));
	return 0;
}

AV_EVENT_RETURN_STATUS PluginManager::processEvent(AV_EVENT_TYPE eventType, void* event)
{
	try
	{
		this->moduleLoadMutex.lock_shared();
		EventParser* eventParser = this->parsersMap[eventType];
		AvEvent* parsedEvent = eventParser->parse(event);
		priorityMap* eventPriorityMap = this->callbacksMap[eventType];
		for (priorityMap::iterator it = eventPriorityMap->begin(); it != eventPriorityMap->end(); it++)
		{
			int priority = (*it).first;
			callback curCallback = (*it).second;
			std::cout << "Processing callback with priority " << priority << " in plugin " << curCallback.second->getName() << "\n";
			AV_EVENT_RETURN_STATUS status = curCallback.second->callback(curCallback.first, parsedEvent);
			if (status == AvEventStatusBlock)
				return status;
		}
		this->moduleLoadMutex.unlock_shared();
		return AvEventStatusAllow;
	}
	catch (const std::string& ex)
	{
		std::cout << "Exception: " << ex << "\n";
		return AvEventStatusAllow;
	}
	
}

void PluginManager::addEventParser(AV_EVENT_TYPE eventType, EventParser* parser)
{
	this->parsersMap.insert(std::pair<int, EventParser*>(eventType, parser));
	this->callbacksMap.insert(std::pair<AV_EVENT_TYPE, priorityMap*>(eventType, new priorityMap()));
}
