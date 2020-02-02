#pragma once
#include "PluginInterface.h"
#include "EventsParser.h"
#include "CommPortServer.h"
#include "UMEventsListener.h"

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <atomic>
#include <shared_mutex>

// <int - callbackId, IPlugin* - plugin that registered the callback>
typedef std::pair<int, IPlugin*> callback;
// <int - callback priority, specified by the plugin, callback>
typedef std::map<int, callback> priorityMap;
typedef std::map <AV_EVENT_TYPE, priorityMap*> eventsMap;

class PluginManager : public IManager
{
public:
	PluginManager(ILogger* logger, IMessageManager *messageManager);
	virtual ~PluginManager() override;

	virtual IPlugin * loadPlugin(std::string path) override;
	virtual void unloadPlugin(std::string name) override;

	// returns IPlugin from loadedPlugins map.
	virtual IPlugin* getPluginByName(std::string name) override;
	// returns list of pugins' IDs (names)
	virtual std::list<std::string>* getPluginsNames() override;
	// this function is used from plugins (in IPlugin init())
	// to register events callbacks.
	virtual int registerCallback(IPlugin * plugin, int callbackId, AV_EVENT_TYPE eventType, int priority) override;
	// this function is used on PluginManager initialization in order
	// to register parsers for implemented events.
	void addEventParser(AV_EVENT_TYPE, EventParser*);

	void* parseKMEvent(AV_EVENT_TYPE, void*);
	// this function is called from event listeners in order to
	// process an event by iterating through registered callbacks.
	AV_EVENT_RETURN_STATUS processEvent(AV_EVENT_TYPE eventType, void*, void**);

	// Syncronization methods (use shared_mutex)
	virtual void enterCriticalEventProcessingSection() override;
	virtual void leaveCriticalEventProcessingSection() override;
	virtual void lockEventsProcessing() override;
	virtual void unlockEventsProcessing() override;

	virtual ILogger* getLogger() override;
	virtual IConfig* getConfig() override;
	virtual IMessageManager* getMessageManager() override;

private:
	IConfig* pluginManagerConfig;

	/*
	Callbacks map (two levels):
	{
		"eventType" : 
		{
			Priority1 : (callbackId, Plutin)
			...
		}
	}
	Each event type has it's priority callback map.
	Each callback in priority map is identified by callbackId and priority.
	Priorities should be unique within priorityMaps.
	Here we use the fact that map::iterator iterates keys
	from the smallest to the biggest in order to implement
	callback prioritization.
	*/
	eventsMap callbacksMap;
	// map of loaded modules. string - module ID.
	std::map<std::string, IPlugin*> loadedPlugins;
	// map of event parsers.
	std::map<int, EventParser*> parsersMap;
	// mutex to syncronize operations that modify
	// cruatial structures processed in event listeners
	// (it is used to pause all event listening threads).
	std::shared_mutex eventProcessingMutex;

	// logger
	ILogger* logger;
	IMessageManager* messageManager;
};