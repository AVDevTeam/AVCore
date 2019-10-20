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
	virtual AV_EVENT_RETURN_STATUS callback(int callbackId, void* event) = 0;
	virtual void init(IManager *) = 0;
	virtual std::string& getName() = 0;
};

#endif