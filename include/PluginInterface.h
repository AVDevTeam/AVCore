/**
\file
\brief Defines interface of principal AV components.
*/

#pragma once
#include "EventsUMInterfaces.h"
#include <Windows.h>
#include "KMUMcomm.h"
#include <map>
#include <list>

class IManager;
class IPlugin;
class IConfig;
class ILogger;

/**
\class
\brief Plugin manager interface

Defines interface of PluginManager.
Plugin manager processes events flow in the system
by passing them through the registered plugins' callbacks.
*/
class IManager
{
public:
	virtual ~IManager() {}
	/**
	\brief Method for callbacks registration

	Enables plugin to register custom callbacks for supported events.

	\param[in] plugin Pointer to plugin instance that registers the callback.
	\param[in] callbackId Arbitrary integer value that will server to identify the callback within the plugin.
	This value will be passed to plugin from IManager when the event occurs.

	\param[in] eventType Type of event to which the callback will be attached.
	\param[in] priority Integer value of callback priority. Callbacks with lesser priority will be called first.

	\return reserved (always 0)
	*/
	virtual int registerCallback(IPlugin* plugin, int callbackId, AV_EVENT_TYPE eventType, int priority) = 0;
	
	virtual IPlugin* loadPlugin(std::string path) = 0;
	virtual void unloadPlugin(std::string name) = 0;

	// returns IPlugin from loadedPlugins map.
	virtual IPlugin* getPluginByName(std::string name) = 0;
	// returns list of pugins' IDs (names)
	virtual std::list<std::string>* getPluginsNames() = 0;

	virtual void* parseKMEvent(AV_EVENT_TYPE, void*) = 0;
	virtual AV_EVENT_RETURN_STATUS processEvent(AV_EVENT_TYPE, void*, void**) = 0;

	// Syncronizationi methods
	virtual void enterCriticalEventProcessingSection() = 0;
	virtual void leaveCriticalEventProcessingSection() = 0;

	virtual void lockEventsProcessing() = 0;
	virtual void unlockEventsProcessing() = 0;

	virtual ILogger* getLogger() = 0;
	virtual IConfig* getConfig() = 0;
};

// Interface for plugins
class IPlugin
{
public:
	virtual ~IPlugin() {}
	/**
	\brief Method for plugin initialization.
	\param[in] manager pointer to the pluginManager instance that will be used to set callbacks via registerCallback method.
	\param[in] module module handle received via LoadLibrary in loadPlugin method of IManager. Plugins shoud provide access to it via getModule method.
	\param[in] configManager instant of configuration manager that is used to access plugin's registry key.
	*/
	virtual void init(IManager* manager, HMODULE module, IConfig* configManager) = 0;
	/**
	Plugins should implement resources dealocation in this method.
	// deinit() should be called on pluginUnload in IManager.
	*/
	virtual void deinit() = 0;

	/**
	\brief Enables access to registered callbacks from IManager
	
	Entry point for plugin's event processing logic. This method
	will be called from IManager in processEvent.

	\param[in] callbackId callback identifier that was registered via registerCallback method.
	\param[in] pointer to the event instance.
	\param[out] pointer to the message that will be passed back to the event source.
	*/
	virtual AV_EVENT_RETURN_STATUS callback(int callbackId, void* event, void** umMessage) = 0;

	virtual int processCommand(std::string name, std::string args) = 0;
		
	/**
	\brief methods that provide access to the information about the plugin.
	*/
	virtual std::string& getName() = 0;
	/**
	\brief returns brief plugin description.
	*/
	virtual std::string& getDescription() = 0;
	/**
	\brief returns plugin version.

	This method is used to implement plugins updates.
	*/
	virtual unsigned int getVersion() = 0;
	/**
	\brief returns stored module handle.
	*/
	virtual HMODULE getModule() = 0;
	/**
	\brief returns plugin's configuration manager.
	*/
	virtual IConfig* getConfig() = 0;
};

// Type of supported parameters.
typedef enum _ConfigParamType {
	DwordParam = 0,
	StringParam,
	ListParam
} ConfigParamType;

typedef std::map<std::string, ConfigParamType> paramMap;
typedef std::pair<std::string, ConfigParamType> paramPair;

/**
\class
\brief Interface for configuration managers

This interface defines basic functionality of config stores.
Different implementations of this interface should implement
configs serrialization in different formats (different storages)
such as: registry, file system, etc.
*/
class IConfig
{
public:
	virtual ~IConfig() {}
	/**
	\brief Initializes IConfig store
	
	\param[in] moduleId string that identifies a module that will
		use this IConfig. It shoud be unique because it will
		make up the reg key path.
	*/
	virtual void init(std::string moduleId) = 0;
	/**
	\brief releases IConfig store.
	*/
	virtual void deinit() = 0;
	/**
	\brief this function is used by plugins to set their parameters' lists.
	*/
	virtual void setParamMap(paramMap*) = 0;
	/**
	\brief this function should be used to query parameter list for the IConfig.
	*/
	virtual paramMap* getParamMap() = 0;
	
	virtual DWORD getDwordParam(std::string paramName) = 0;
	virtual std::string getStringParam(std::string paramName) = 0;
	virtual std::list<std::string>* getListParam(std::string paramName) = 0;
	
	virtual void setDwordParam(std::string& paramName, DWORD value) = 0;
	virtual void setStringParam(std::string& paramName, std::string& value) = 0;
	virtual void setListParam(std::string& paramName, std::list<std::string>& value) = 0;
};

/**
\class
\brief Interface for debug loggers.
*/
class ILogger
{
public:
	virtual ~ILogger() {}

	virtual void log(std::string) = 0;
};