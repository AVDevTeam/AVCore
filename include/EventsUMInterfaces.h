/**
\file
\brief Declares events interfaces that are used from plugins to access event data.
*/

#pragma once
#include "EventsKMStructures.h"
#include <list>
#include <string>

/**
\brief AvFSEventCreate interface
Interface for AvFSEventCreate that will be used
in plugins to access event parameters.
*/
class IEventFSCreate
{
public:
	virtual ~IEventFSCreate() {}
	virtual int getRequestorPID() = 0;
	virtual char getRequestorMode() = 0;
	virtual std::string& getFilePath() = 0;
};

/**
\brief AvObEventProcessHandleCreate interface
Interface for AvObEventProcessHandleCreate that will be used
in plugins to access event parameters.
*/
class IEventObProcessHandleCreate
{
public:
	virtual ~IEventObProcessHandleCreate() {}
	virtual int getRequestorPID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
};

/**
\brief AvObEventProcessHandleDublicate interface
Interface for AvObEventProcessHandleDublicate that will be used
in plugins to access event parameters.
*/
class IEventObProcessHandleDublicate
{
public:
	virtual ~IEventObProcessHandleDublicate() {}
	virtual int getRequestorPID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
	virtual int getDublicateSourcePID() = 0;
	virtual int getDublicateTargetPID() = 0;
};

/**
\brief AvObEventThreadHandleCreate interface
Interface for AvObEventThreadHandleCreate that will be used
in plugins to access event parameters.
*/
class IEventObThreadHandleCreate
{
public:
	virtual ~IEventObThreadHandleCreate() {}
	virtual int getRequestorPID() = 0;
	virtual int getRequestorTID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual int getTargetTID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
};

/**
\brief AvObEventThreadHandleDublicate interface
Interface for AvObEventThreadHandleDublicate that will be used
in plugins to access event parameters.
*/
class IEventObThreadHandleDublicate
{
public:
	virtual ~IEventObThreadHandleDublicate() {}
	virtual int getRequestorPID() = 0;
	virtual int getRequestorTID() = 0;
	virtual unsigned char getIsKernelHandle() = 0;
	virtual int getTargetPID() = 0;
	virtual int getTargetTID() = 0;
	virtual unsigned long getDesiredAccess() = 0;
	virtual int getDublicateSourcePID() = 0;
	virtual int getDublicateTargetPID() = 0;
};

/**
\brief AvEventProcessCreate interface
Interface for AvEventProcessCreate that will be used
in plugins to access event parameters.
*/
class IEventProcessCreate
{
public:
	virtual ~IEventProcessCreate() {}
	virtual int getPID() = 0;
	virtual int getParentPID() = 0;
	virtual int getCreatingPID() = 0;
	virtual int getCreatingTID() = 0;
	virtual std::string& getImageFileName() = 0;
	virtual std::string& getCommandLine() = 0;
};

/**
\brief AvEventProcessExit interface
Interface for AvEventProcessExit that will be used
in plugins to access event parameters.
*/
class IEventProcessExit
{
public:
	virtual ~IEventProcessExit() {}
	virtual int getPID() = 0;
};

/**
\brief AvEventThreadCreate interface
Interface for AvEventThreadCreate that will be used
in plugins to access event parameters.
*/
class IEventThreadCreate
{
public:
	virtual ~IEventThreadCreate() {}
	virtual int getPID() = 0;
	virtual int getTID() = 0;
};

/**
\brief AvEventThreadExit interface
Interface for AvEventThreadExit that will be used
in plugins to access event parameters.
*/
class IEventThreadExit
{
public:
	virtual ~IEventThreadExit() {}
	virtual int getPID() = 0;
	virtual int getTID() = 0;
};

/**
\brief AvEventImageLoad interface
Interface for AvEventImageLoad that will be used
in plugins to access event parameters.
*/
class IEventImageLoad
{
public:
	virtual ~IEventImageLoad() {}
	virtual int getPID() = 0;
	virtual std::string& getImageName() = 0;
	virtual unsigned char getIsSystemModule() = 0;
};

/**
\brief AvEventRegCreateKey interface
Interface for AvEventRegCreateKey that will be used
in plugins to access event parameters.
*/
class IEventRegCreateKey
{
public:
	virtual ~IEventRegCreateKey() {}
	virtual int getRequestorPID() = 0;
	virtual std::string& getKeyPath() = 0;
};

/**
\brief AvEventRegOpenKey interface
Interface for AvEventRegOpenKey that will be used
in plugins to access event parameters.
*/
class IEventRegOpenKey
{
public:
	virtual ~IEventRegOpenKey() {}
	virtual int getRequestorPID() = 0;
	virtual std::string& getKeyPath() = 0;
};

/**
\brief IEventNetwork interface
Interface for AvNetwork that will be used
in plugins to access event parameters.
*/
class IEventNetwork
{
public:
	virtual ~IEventNetwork() {}
	virtual void *getLocalAddress() = 0;
	virtual void *getRemoteAddress() = 0;
	virtual char *getLocalAddressStr() = 0;
	virtual char *getRemoteAddressStr() = 0;
	virtual int getLocalPort() = 0;
	virtual int getRemotePort() = 0;
	virtual int getFamily() = 0;
	virtual char *getData() = 0;
	virtual unsigned long long getDataLength() = 0;
};

/**
\brief AvWinApiCall interface
Interface for AvWinApiCall that will be used
in plugins to access event parameters.
*/
class IEventWinApiCall
{
public:
	virtual ~IEventWinApiCall() {}
	virtual int getPID() = 0;
	virtual std::string getFunctionName() = 0;
	virtual std::list<std::string> getFunctionArgs() = 0;
};