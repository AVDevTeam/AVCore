#include "PluginManager.h"
#include "ConfigManager.h"
#include <Windows.h>

typedef IPlugin* (* GetPlugin)();

PluginManager::PluginManager(ILogger* logger, IMessageManager* messageManager)
{
	this->logger = logger;
	this->messageManager = messageManager;
	this->pluginManagerConfig = new UMModuleConfig(logger);
	this->pluginManagerConfig->init("PluginManager");
	paramMap* params = new paramMap();
	params->insert(paramPair("Plugins", ListParam));
	params->insert(paramPair("PluginsPath", StringParam));
	this->pluginManagerConfig->setParamMap(params);
}

PluginManager::~PluginManager()
{
	std::list<std::string> pluginsToUnload;
	for (std::map<std::string, IPlugin*>::iterator it = this->loadedPlugins.begin(); it != this->loadedPlugins.end(); it++)
	{
		pluginsToUnload.push_back((*it).first);
	}
	for (std::list<std::string>::iterator it = pluginsToUnload.begin(); it != pluginsToUnload.end(); it++)
	{
		this->unloadPlugin((*it));
	}

	for (std::map<int, EventParser*>::iterator it = this->parsersMap.begin(); it != this->parsersMap.end(); it++)
	{
		delete (*it).second;
	}

	this->pluginManagerConfig->deinit();
	delete this->pluginManagerConfig;
}

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
	// Create config store for the plugin.
	UMModuleConfig* configManager = new UMModuleConfig(this->logger);
	this->logger->log("PluginManager. Initializing plugin " + plugin->getName());
	configManager->init("Plugins\\" + plugin->getName());
	plugin->init(this, pluginModule, configManager);
	this->logger->log("PluginManager. Plugin was initialized.");
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

std::list<std::string>* PluginManager::getPluginsNames()
{
	std::list<std::string>* result = new std::list<std::string>();
	for (std::map<std::string, IPlugin*>::iterator it = this->loadedPlugins.begin(); it != this->loadedPlugins.end(); it++)
		result->push_back((*it).first);
	return result;
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
AV_EVENT_RETURN_STATUS PluginManager::processEvent(AV_EVENT_TYPE eventType, void* event, void** umMessage)
{
#ifdef NOTTRUE 
	switch (eventType)
	{
	case AvFileCreate:
		this->logger->log("Got AvFileCreate event.");
		break;
	case AvProcessHandleCreate:
		this->logger->log("Got AvProcessHandleCreate event.");
		break;
	case AvProcessHandleDublicate:
		this->logger->log("Got AvProcessHandleDublicate event.");
		break;
	case AvThreadHandleCreate:
		this->logger->log("Got AvThreadHandleCreate event.");
		break;
	case AvThreadHandleDublicate:
		this->logger->log("Got AvThreadHandleDublicate event.");
		break;
	case AvProcessCreate:
		this->logger->log("Got AvProcessCreate event.");
		break;
	case AvProcessExit:
		this->logger->log("Got AvProcessExit event.");
		break;
	case AvThreadCreate:
		this->logger->log("Got AvThreadCreate event.");
		break;
	case AvThreadExit:
		this->logger->log("Got AvThreadExit event.");
		break;
	case AvImageLoad:
		this->logger->log("Got AvImageLoad event.");
		break;
	case AvRegCreateKey:
		this->logger->log("Got AvRegCreateKey event.");
		break;
	case AvRegOpenKey:
		this->logger->log("Got AvRegOpenKey event.");
		break;
	case AvApcProcessInject:
		this->logger->log("Got AvApcProcessInject event.");
		break;
	}
#endif
	try
	{
		// enter event processing section
		this->eventProcessingMutex.lock_shared();

		priorityMap* eventPriorityMap = this->callbacksMap[eventType];
		AV_EVENT_RETURN_STATUS status = AvEventStatusAllow;
		for (priorityMap::iterator it = eventPriorityMap->begin(); it != eventPriorityMap->end(); it++)
		{
			int priority = (*it).first;
			callback curCallback = (*it).second;
#ifdef _DEBUG
			this->logger->log("Processing callback with priority " + std::to_string(priority) + " in plugin " + curCallback.second->getName());
#endif
			status = curCallback.second->callback(curCallback.first, event, umMessage);
			if (status == AvEventStatusBlock)
			{
				break;
			}
		}
		delete event;

		// leave event processing section
		this->eventProcessingMutex.unlock_shared();
		return status;
	}
	catch (...)
	{
		this->logger->log("Exception hapend during event processing.");
		return AvEventStatusAllow;
	}
	
}

void PluginManager::enterCriticalEventProcessingSection()
{
	this->eventProcessingMutex.lock_shared();
}

void PluginManager::leaveCriticalEventProcessingSection()
{
	this->eventProcessingMutex.unlock_shared();
}

void PluginManager::lockEventsProcessing()
{
	this->eventProcessingMutex.lock();
}

void PluginManager::unlockEventsProcessing()
{
	this->eventProcessingMutex.unlock();
}

ILogger* PluginManager::getLogger()
{
	return this->logger;
}

IConfig* PluginManager::getConfig()
{
	return this->pluginManagerConfig;
}

IMessageManager* PluginManager::getMessageManager()
{
	return this->messageManager;
}

void PluginManager::addEventParser(AV_EVENT_TYPE eventType, EventParser* parser)
{
	this->parsersMap.insert(std::pair<int, EventParser*>(eventType, parser));
	this->callbacksMap.insert(std::pair<AV_EVENT_TYPE, priorityMap*>(eventType, new priorityMap()));
}

void* PluginManager::parseKMEvent(AV_EVENT_TYPE eventType, void* event)
{
	EventParser* eventParser = this->parsersMap[eventType];
	AvEvent* parsedEvent = eventParser->parse(event);
	return reinterpret_cast<void*>(parsedEvent);
}
