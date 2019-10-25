#ifndef __EVENTS_H__
#define __EVENTS_H__

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

#include "EventsUMInterfaces.h"
#include <Windows.h>
#include "KMUMcomm.h"
#include <map>
#include <list>

class IManager;
class IPlugin;
class IConfig;

// Plugin manager interface
class IManager
{
public:
	virtual int registerCallback(IPlugin*, int, AV_EVENT_TYPE, int) = 0;
	virtual AV_EVENT_RETURN_STATUS processEvent(AV_EVENT_TYPE, void*) = 0;

	// Syncronizationi methods
	virtual void enterCriticalEventProcessingSection() = 0;
	virtual void leaveCriticalEventProcessingSection() = 0;

	virtual void lockEventsProcessing() = 0;
	virtual void unlockEventsProcessing() = 0;
};

// Interface for plugins
class IPlugin
{
public:
	/*
	Method description:
		Method for plugin initialization.
	Arguments:
		IManager * - pointer to the pluginManager instance that will
		be used to set callbacks via registerCallback method.

		HMODULE - module handle received via LoadLibrary in loadPlugin
		method of IManager. Plugins shoud provide access to it via
		getModule method.
	*/
	virtual void init(IManager*, HMODULE, IConfig*) = 0;
	// Plugins should implement resources dealocation in this method.
	// deinit() should be called on pluginUnload in IManager.
	virtual void deinit() = 0;

	/*
	Method description:
		Entry point for plugin's event processing logic. This method
		will be called from IManager in processEvent.
	*/
	virtual AV_EVENT_RETURN_STATUS callback(int callbackId, void* event) = 0;
	
	// methods that provide access to the information about the plugin.
	virtual std::string& getName() = 0;
	virtual std::string& getDescription() = 0;
	virtual HMODULE getModule() = 0;
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

// Interface for configuration managers
class IConfig
{
public:
	/*
	Method description:
		Initializes IConfig store (opens registry key).
	Arguments:
		moduleId - string that identifies a module that will
		use this IConfig. It shoud be unique because it will
		make up the reg key path.
	*/
	virtual void init(std::string moduleId) = 0;
	// releases IConfig store.
	virtual void deinit() = 0;
	// this function is used by plugins to set their parameters' lists.
	virtual void setParamMap(paramMap*) = 0;
	// this function should be used to query parameter list for the IConfig.
	virtual paramMap* getParamMap() = 0;
	// parameter getters
	virtual DWORD getDwordParam(std::string paramName) = 0;
	virtual std::string getStringParam(std::string paramName) = 0;
	virtual std::list<std::string>* getListParam(std::string paramName) = 0;
	// parameter setters
	virtual void setDwordParam(std::string& paramName, DWORD value) = 0;
	virtual void setStringParam(std::string& paramName, std::string& value) = 0;
	virtual void setListParam(std::string& paramName, std::list<std::string>& value) = 0;
};

#endif