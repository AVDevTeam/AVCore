#pragma once
#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

#include "EventsKMStructures.h"
#include <string>

class IEventFSCreate
{
public:
	virtual int getRequestorPID() = 0;
	virtual char getRequestorMode() = 0;
	virtual std::string& getFilePath() = 0;
};
