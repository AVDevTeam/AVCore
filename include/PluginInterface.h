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

class IPlugin;

// Plugin manager interface
class IManager
{
public:
	virtual int registerCallback(IPlugin*, int, AV_EVENT_TYPE, int) = 0;
	virtual AV_EVENT_RETURN_STATUS processEvent(AV_EVENT_TYPE, void*) = 0;
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
	virtual void init(IManager*, HMODULE) = 0;

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
};

#endif