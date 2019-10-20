#pragma once
#include "EventsKM.h"
#include <Windows.h>
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

private:
	wchar_t* getVoluemLetter(wchar_t*);
};