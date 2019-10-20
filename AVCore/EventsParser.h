#pragma once
#include <Windows.h>
#include "EventsUMInterfaces.h"
#include <string>

// Base class for all UM events.
// contains helper functions.
class AvEvent
{
protected:
	wchar_t* wcscpyZeroTerminate(wchar_t*, int);
};

// Class for File System create event (IRP_MJ_CREATE).
class AvFSEventCreate : public IEventFSCreate, AvEvent
{
public:
	AvFSEventCreate(PAV_EVENT_FILE_CREATE);
	~AvFSEventCreate();

	// Inherited via IEventFSCreate
	virtual int getRequestorPID() override;
	virtual char getRequestorMode() override;
	virtual std::string& getFilePath() override;

private:
	wchar_t* getVoluemLetter(wchar_t*);
	std::string FilePath;
	int RequestorPID;
	char RequestorMode;
};