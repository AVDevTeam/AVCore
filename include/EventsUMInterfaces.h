#pragma once
#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

#include "EventsKMStructures.h"
#include <string>

// Interface for AvFSEventCreate that will be used
// in plugins to access event parameters.
class IEventFSCreate
{
public:
	virtual int getRequestorPID() = 0;
	virtual char getRequestorMode() = 0;
	virtual std::string& getFilePath() = 0;
};

// Interface for AvObEventProcessHandleCreate that will be used
// in plugins to access event parameters.
class IEventObProcessHandleCreate
{
public:
	virtual int getRequestorPID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
};

// Interface for AvObEventProcessHandleDublicate that will be used
// in plugins to access event parameters.
class IEventObProcessHandleDublicate : IEventObProcessHandleCreate
{
public:
	virtual int getDublicateSourcePID() = 0;
	virtual int getDublicateTargetPID() = 0;
};

