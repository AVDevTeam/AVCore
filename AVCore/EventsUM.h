/*++
Module Name:
	EventsUM.h
Abstract:
	This header defines classes that represents events in UM (AVCore.exe).
	Constructors of the classes should parse the event buffer received from KM driver
	or the input from other event source (TODO! pass events from hooking module, for
	example through pipes).
Environment:
	User mode
--*/

#pragma once
#include "EventsKM.h"
#include <windows.h>
#include <string>

// Base class for all UM events.
// contains helper functions.
class AvEvent
{
protected:
	wchar_t* wcscpyZeroTerminate(wchar_t*, int);
};

// Class for File System create event (IRP_MJ_CREATE).
class AvFSEventCreate : public AvEvent
{
public:
	AvFSEventCreate(PAV_EVENT_FILE_CREATE);
	~AvFSEventCreate();

	std::string FilePath;
	int RequestorPID;
	char RequestorMode;
};