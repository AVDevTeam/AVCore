#pragma once
#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

#include "EventsKM.h"
#include <string>

class IEventHasRequestor
{
public:
	virtual int getRequestorPID() = 0;
	virtual char getRequestorMode() = 0;
};

class IEventFSCreate
{
public:
	virtual std::string getFilePath() = 0;
};
