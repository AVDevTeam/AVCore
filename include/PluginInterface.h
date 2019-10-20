#ifndef __EVENTS_H__
#define __EVENTS_H__

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

#include "EventsUMInterfaces.h"

class IPlugin;

// Plugin manager interface
class IManager
{
public:
	virtual int registerCallback(IPlugin*, int, int, int) = 0;
	virtual AV_EVENT_RETURN_STATUS processEvent(int, void*) = 0;
};

// Base class for plugins
class IPlugin
{
public:
	virtual AV_EVENT_RETURN_STATUS callback(int, void*) = 0;
	virtual void init(IManager *) = 0;
};

#endif