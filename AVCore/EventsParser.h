#pragma once
#include <Windows.h>
#include "EventsUMInterfaces.h"
#include <string>

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
	~AvFSEventCreate();

	// Inherited via IEventFSCreate
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
class AvEventProcessCreate : public IEventProcessCreate
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