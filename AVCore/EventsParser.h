#pragma once
#include <Windows.h>
#include "EventsUMInterfaces.h"
#include <string>
#include <list>
#include <iostream>

// Base class for all UM events.
// contains helper functions.
// Classes inherited from AvEvent are passed
// to the plugins that registered callback
// on a particula event.
// Each implementation of AvEvent class
// should provide interface that will be available
// for plugins (those interfaces are declared 
// in EventsUMInterfaces.h).
class AvEvent
{
public:
	AvEvent() {};
	virtual ~AvEvent() {}
};

// Base class for event parsers.
// Event parsers should process KM event structures and
// build AvEventClasses (inherited from AvEvent).
class EventParser
{
public:
	virtual AvEvent* parse(PVOID) = 0;

protected:
	wchar_t* wcscpyZeroTerminate(wchar_t*, int);
};

// Class for File System create event (IRP_MJ_CREATE).
class AvFSEventCreate : public IEventFSCreate, AvEvent
{
public:
	// constructor that is used by AvFSEventCreateParser.
	AvFSEventCreate(char, int, std::string);

	// Inherited via IEventFSCreate
	virtual ~AvFSEventCreate() override;
	virtual int getRequestorPID() override;
	virtual char getRequestorMode() override;
	virtual std::string& getFilePath() override;

private:
	std::string FilePath;
	int RequestorPID;
	char RequestorMode;
};

// AvFSEvent parser
class AvFSEventCreateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;

private:
	wchar_t* getVoluemLetter(wchar_t*);
};

// Class for Object Filter (handles operations)
// process handle create event.
class AvObEventProcessHandleCreate : public IEventObProcessHandleCreate, AvEvent
{
public:
	AvObEventProcessHandleCreate(int requestorPID, boolean isKernelHandle, int targetPID, ACCESS_MASK desiredAccess)
	{
		this->requestorPID = requestorPID;
		this->isKernelHandle = isKernelHandle;
		this->targetPID = targetPID;
		this->desiredAccess = desiredAccess;
	}

	// Inherited via IEventObProcessHandleCreate
	virtual ~AvObEventProcessHandleCreate() override;
	virtual int getRequestorPID() override;
	virtual unsigned char  getIsKernelHandle() override;
	virtual int getTargetPID() override;
	virtual unsigned long  getDesiredAccess() override;
protected:
	int requestorPID;
	unsigned char isKernelHandle;
	int targetPID;
	unsigned long desiredAccess;
};

// AvProcessHandleCreate parser
class AvObEventProcessHandleCreateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for Object Filter (handles operations)
// process handle dublicate event.
class AvObEventProcessHandleDublicate : public IEventObProcessHandleDublicate, AvObEventProcessHandleCreate
{
public:
	AvObEventProcessHandleDublicate(int requestorPID, boolean isKernelHandle, int targetPID, ACCESS_MASK desiredAccess, int dublicateSourcePID, int dublicateTargetPID)
		: AvObEventProcessHandleCreate(requestorPID, isKernelHandle, targetPID, desiredAccess)
	{
		this->dublicateSourcePID = dublicateSourcePID;
		this->dublicateTargetPID = dublicateTargetPID;
	}

	// Inherited via IEventObProcessHandleDublicate
	virtual ~AvObEventProcessHandleDublicate() override;
	virtual int getDublicateSourcePID() override;
	virtual int getDublicateTargetPID() override;
	virtual int getRequestorPID() override;
	virtual unsigned char getIsKernelHandle() override;
	virtual int getTargetPID() override;
	virtual unsigned long getDesiredAccess() override;

private:
	int dublicateSourcePID;
	int dublicateTargetPID;
};

// AvProcessHandleDublicate parser
class AvObEventProcessHandleDublicateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for Object Filter (handles operations)
// thread handle create event.
class AvObEventThreadHandleCreate : public IEventObThreadHandleCreate, AvObEventProcessHandleCreate
{
public:
	AvObEventThreadHandleCreate(int requestorPID, int requestorTID, boolean isKernelHandle, int targetPID, int targetTID, ACCESS_MASK desiredAccess)
		: AvObEventProcessHandleCreate(requestorPID, isKernelHandle,targetPID, desiredAccess)
	{
		this->requestorTID = requestorTID;
		this->targetTID = targetTID;
	}
	// Inherited via IEventObThreadHandleCreate
	virtual ~AvObEventThreadHandleCreate() override;
	virtual int getRequestorTID() override;
	virtual int getTargetTID() override;
	virtual int getRequestorPID() override;
	virtual unsigned char  getIsKernelHandle() override;
	virtual int getTargetPID() override;
	virtual unsigned long  getDesiredAccess() override;
protected:
	int requestorTID;
	int targetTID;
	using AvObEventProcessHandleCreate::requestorPID;
	using AvObEventProcessHandleCreate::isKernelHandle;
	using AvObEventProcessHandleCreate::targetPID;
	using AvObEventProcessHandleCreate::desiredAccess;
};

// AvThreadHandleCreate parser
class AvObEventThreadHandleCreateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for Object Filter (handles operations)
// thread handle dublicate event.
class AvObEventThreadHandleDublicate : public IEventObThreadHandleDublicate, private AvObEventThreadHandleCreate
{
public:
	AvObEventThreadHandleDublicate(int requestorPID, int requestorTID, boolean isKernelHandle, int targetPID, int targetTID, ACCESS_MASK desiredAccess, int dublicateSourcePID, int dublicateTargetPID)
		: AvObEventThreadHandleCreate(requestorPID, requestorTID, isKernelHandle, targetPID, targetTID, desiredAccess)
	{
		this->dublicateSourcePID = dublicateSourcePID;
		this->dublicateTargetPID = dublicateTargetPID;
	}

	// Inherited via IEventObThreadHandleDublicate
	virtual ~AvObEventThreadHandleDublicate() override;
	virtual int getRequestorTID() override;
	virtual int getTargetTID() override;
	virtual int getDublicateSourcePID() override;
	virtual int getDublicateTargetPID() override;
	virtual int getRequestorPID() override;
	virtual unsigned char getIsKernelHandle() override;
	virtual int getTargetPID() override;
	virtual unsigned long getDesiredAccess() override;

private:
	int dublicateSourcePID;
	int dublicateTargetPID;
};

// AvThreadHandleDublicate parser
class AvObEventThreadHandleDublicateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for process create event (PsSetCreateProcessNotifyRoutineEx[2])
class AvEventProcessCreate : public IEventProcessCreate, AvEvent
{
public:
	AvEventProcessCreate(int PID, int parentPID, int creatingPID, int creatingTID, std::string imageFileName, std::string commandLine)
	{
		this->PID = PID;
		this->parentPID = parentPID;
		this->creatingPID = creatingPID;
		this->creatingTID = creatingTID;
		this->imageFileName = imageFileName;
		this->commandLine = commandLine;
	}
	
	// Inherited via IEventProcessCreate
	virtual ~AvEventProcessCreate() override;
	virtual int getPID() override;
	virtual int getParentPID() override;
	virtual int getCreatingPID() override;
	virtual int getCreatingTID() override;
	virtual std::string& getImageFileName() override;
	virtual std::string& getCommandLine() override;
private:
	int PID;
	int parentPID;
	int creatingPID;
	int creatingTID;
	std::string imageFileName;
	std::string commandLine;
};

// AvEventProcessCreate parser
class AvEventProcessCreateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for process exit event (PsSetCreateProcessNotifyRoutineEx[2])
class AvEventProcessEixt : public IEventProcessExit, AvEvent
{
public:
	AvEventProcessEixt(int PID) { this->PID = PID; }

	// Inherited via IEventProcessExit
	virtual ~AvEventProcessEixt() override;
	virtual int getPID() override;
private:
	int PID;
};

// AvEventProcessEixt parser
class AvEventProcessEixtParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for thread create event (PsSetCreateThreadNotifyRoutine)
class AvEventThreadCreate : public IEventThreadCreate, AvEvent
{
public:
	AvEventThreadCreate(int PID, int TID) { this->PID = PID; this->TID = TID; }

	// Inherited via IEventThreadCreate
	virtual ~AvEventThreadCreate() override;
	virtual int getPID() override;
	virtual int getTID() override;
private:
	int PID;
	int TID;
};

// AvEventThreadCreate parser
class AvEventThreadCreateParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for thread exit event (PsSetCreateThreadNotifyRoutine)
class AvEventThreadExit : public IEventThreadExit, AvEvent
{
public:
	AvEventThreadExit(int PID, int TID) { this->PID = PID; this->TID = TID; }

	// Inherited via IEventThreadCreate
	virtual ~AvEventThreadExit() override;
	virtual int getPID() override;
	virtual int getTID() override;
private:
	int PID;
	int TID;
};

// AvEventThreadExit parser
class AvEventThreadExitParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for image load event (PsSetImageNotifyRoutine)
class AvEventImageLoad : public IEventImageLoad, AvEvent
{
public:
	AvEventImageLoad(int PID, std::string imageName, unsigned char isSystemModule) 
	{ 
		this->PID = PID;
		this->imageName = imageName;
		this->isSystemModule = isSystemModule;
	}

	// Inherited via IEventImageLoad
	virtual ~AvEventImageLoad() override;
	virtual int getPID() override;
	virtual std::string& getImageName() override;
	virtual unsigned char getIsSystemModule() override;
private:
	int PID;
	std::string imageName;
	unsigned char isSystemModule;
};

// AvEventImageLoad parser
class AvEventImageLoadParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for reg key create event (CmRegisterCallback)
class AvEventRegCreateKey : public IEventRegCreateKey, AvEvent
{
public:
	AvEventRegCreateKey(int requestorPID, std::string keyPath)
	{
		this->requestorPID = requestorPID;
		this->keyPath = keyPath;
	}

	// Inherited via IEventRegCreateKey
	virtual ~AvEventRegCreateKey() override;
	virtual int getRequestorPID() override;
	virtual std::string& getKeyPath() override;
private:
	int requestorPID;
	std::string keyPath;
};

// AvEventRegCreateKey parser
class AvEventRegCreateKeyParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

// Class for reg key open event (CmRegisterCallback)
class AvEventRegOpenKey : public IEventRegOpenKey, AvEvent
{
public:
	AvEventRegOpenKey(int requestorPID, std::string keyPath)
	{
		this->requestorPID = requestorPID;
		this->keyPath = keyPath;
	}

	// Inherited via IEventRegOpenKey
	virtual ~AvEventRegOpenKey() override;
	virtual int getRequestorPID() override;
	virtual std::string& getKeyPath() override;
private:
	int requestorPID;
	std::string keyPath;
};

// AvEventRegOpenKey parser
class AvEventRegOpenKeyParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

class AvEventWinApiCall : IEventWinApiCall, AvEvent
{
public:
	AvEventWinApiCall(int PID, std::string functionName, std::list<std::string> arguments)
	{
		this->PID = PID;
		this->functionName = functionName;
		this->argumetns = arguments;
	}

	// Inherited via IEventWinApiCall
	virtual int getPID() override;
	virtual std::string getFunctionName() override;
	virtual std::list<std::string> getFunctionArgs() override;
private:
	int PID;
	std::string functionName;
	std::list<std::string> argumetns;
};

// AvEventRegOpenKey parser
class AvEventNetworkParser : EventParser
{
public:
	// Inherited via EventParser
	virtual AvEvent* parse(PVOID) override;
};

class AvEventNetwork : IEventNetwork, AvEvent
{
public:
	AvEventNetwork(
		uint8_t *localAddress, 
		uint8_t *remoteAddress, 
		int localPort, 
		int remotePort, 
		int family, 
		char* data, 
		unsigned long long dataLength)
	{
		memcpy(this->localAddress, localAddress, 16);
		memcpy(this->remoteAddress, remoteAddress, 16);
		this->localPort = localPort;
		this->remotePort = remotePort;
		this->family = family;
		this->data = data;
		this->dataLength = dataLength;
	}

	~AvEventNetwork();

	// Inherited via IEventNetwork
	virtual void *getLocalAddress() override;
	virtual void *getRemoteAddress() override;
	virtual char *getLocalAddressStr() override;
	virtual char* getRemoteAddressStr() override;
	virtual int getLocalPort() override;
	virtual int getRemotePort() override;
	virtual int getFamily() override;
	virtual char *getData() override;
	virtual unsigned long long getDataLength() override;

private:
	uint8_t localAddress[16];
	uint8_t remoteAddress[16];
	int localPort;
	int remotePort;
	int family;
	char* data;
	unsigned long long dataLength;
};