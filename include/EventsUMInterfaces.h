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
class IEventObProcessHandleDublicate
{
public:
	virtual int getRequestorPID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
	virtual int getDublicateSourcePID() = 0;
	virtual int getDublicateTargetPID() = 0;
};

// Interface for AvObEventThreadHandleCreate that will be used
// in plugins to access event parameters.
class IEventObThreadHandleCreate
{
public:
	virtual int getRequestorPID() = 0;
	virtual int getRequestorTID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual int getTargetTID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
};

// Interface for AvObEventThreadHandleDublicate that will be used
// in plugins to access event parameters.
class IEventObThreadHandleDublicate
{
public:
	virtual int getRequestorPID() = 0;
	virtual int getRequestorTID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual int getTargetTID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
	virtual int getDublicateSourcePID() = 0;
	virtual int getDublicateTargetPID() = 0;
};

// Interface for AvEventProcessCreate that will be used
// in plugins to access event parameters.
class IEventProcessCreate
{
public:
	virtual int getPID() = 0;
	virtual int getParentPID() = 0;
	virtual int getCreatingPID() = 0;
	virtual int getCreatingTID() = 0;
	virtual std::string& getImageFileName() = 0;
	virtual std::string& getCommandLine() = 0;
};

// Interface for AvEventProcessExit that will be used
// in plugins to access event parameters.
class IEventProcessExit
{
public:
	virtual int getPID() = 0;
};

// Interface for AvEventThreadCreate that will be used
// in plugins to access event parameters.
class IEventThreadCreate
{
public:
	virtual int getPID() = 0;
	virtual int getTID() = 0;
};

// Interface for AvEventThreadExit that will be used
// in plugins to access event parameters.
class IEventThreadExit
{
public:
	virtual int getPID() = 0;
	virtual int getTID() = 0;
};

// Interface for AvEventImageLoad that will be used
// in plugins to access event parameters.
class IEventImageLoad
{
public:
	virtual int getPID() = 0;
	virtual std::string& getImageName() = 0;
	virtual unsigned char getIsSystemModule() = 0;
};

