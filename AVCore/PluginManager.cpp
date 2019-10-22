#include "PluginManager.h"
#include "ConfigManager.h"
#include <Windows.h>

typedef IPlugin* (* GetPlugin)();

IPlugin* PluginManager::loadPlugin(std::string path)
{
	// load plugin dll
	HMODULE pluginModule = LoadLibraryA(path.c_str());
	if (pluginModule == 0)
	{
		return nullptr;
	}
	// get plugin entry point.
	GetPlugin getPlugin = (GetPlugin)GetProcAddress(pluginModule, "GetPlugin");
	if (getPlugin == 0)
		return nullptr;

	// start of the critical section
	// we use lock (not shared_lock) because we need to halt
	// all listener threads.
	this->eventProcessingMutex.lock();
	// retreive IPlugin interface from the entry point.
	IPlugin * plugin = getPlugin();
	UMModuleConfig* configManager = new UMModuleConfig();
	configManager->init(std::string("TestPlugin"));
	plugin->init(this, pluginModule, configManager);
	this->loadedPlugins.insert(std::pair<std::string, IPlugin*>(plugin->getName(), plugin));
	// leaving critical section
	this->eventProcessingMutex.unlock();

	return plugin;
}

void PluginManager::unloadPlugin(std::string name)
{
	// start of critical section
	this->eventProcessingMutex.lock();
	// this list will hold all callbacks that were registered
	// by the unloading plugin.
	std::list<std::pair<priorityMap*, int>> toDelete;

	IPlugin* plugin = this->loadedPlugins[name];
	// iterate over callbackMap and gather all callbacks of the unloadin plugin
	for (eventsMap::iterator it = this->callbacksMap.begin(); it != this->callbacksMap.end(); it++)
		for (priorityMap::iterator it2 = (*it).second->begin(); it2 != (*it).second->end(); it2++)
		{
			callback curCallback = (*it2).second;
			if (curCallback.second == plugin)
				toDelete.push_back(std::pair<priorityMap*, int>((*it).second, (*it2).first));
		}
	// do the actual erasing of the callbacks from the map.
	for (std::list<std::pair<priorityMap*, int>>::iterator it = toDelete.begin(); it != toDelete.end(); it++)
		(*it).first->erase((*it).second);
	this->loadedPlugins.erase(name);
	// free plugin DLL
	HMODULE pluginModule = plugin->getModule();
	// here the plugin should free the memory (including 
	// IPlugin instance that was returned from DLL entry point).
	plugin->deinit();
	FreeLibrary(pluginModule);
	// leaving critical section
	this->eventProcessingMutex.unlock();
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

/*
Method description:
	This method is used in event-streams listeners to handle events.
	First the eventParsers are used to represent event as an
	instance of AvEvent class.
	Method iterates over callback map for the given event type
	from the callback with highest priority (the smallest int).

	If callback returns BlockStatus than the event processing stops.
	If not, the next callback for this event is called.
Arguments:
	eventType - known AV_EVENT_TYPE retreived from event source (KM, UM)
	event - poitner to raw event buffer.
Return value:
	event processing status that will be sent to interceptor module (KM, UM).
*/
AV_EVENT_RETURN_STATUS PluginManager::processEvent(AV_EVENT_TYPE eventType, void* event)
{
	try
	{
		this->eventProcessingMutex.lock_shared();
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
		this->eventProcessingMutex.unlock_shared();
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
