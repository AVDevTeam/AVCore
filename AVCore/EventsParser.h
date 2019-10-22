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
	AvEvent(PVOID) {};
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