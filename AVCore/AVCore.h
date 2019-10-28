#pragma once
#include "CommPortServer.h"
#include "PluginManager.h"
#include "EventsParser.h"
#include "ConfigManager.h"
#include "FileLogger.h"
#include <mutex>

//#define TESTBUILD

class AVCore
{
public:
	AVCore(ILogger* logger) 
	{ 
		this->logger = logger;
		this->portServer = new CommPortServer();
		this->manager = new PluginManager(logger);
		this->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (this->stopEvent == NULL)
		{
			throw "Error creating stop event";
		}
	}
	~AVCore() 
	{ 
		delete this->manager;
		delete this->portServer;
		CloseHandle(this->stopEvent);
	}
	void start(void);
	void stop(void) 
	{
		portServer->stop();
		SetEvent(this->stopEvent);
	}
	void wait(void) { WaitForSingleObject(this->stopEvent, INFINITE); }

private:
	PluginManager * manager;
	CommPortServer * portServer;
	ILogger* logger;
	HANDLE stopEvent = INVALID_HANDLE_VALUE;
};