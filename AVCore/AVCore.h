#pragma once
#include "CommPortServer.h"
#include "PluginManager.h"
#include "EventsParser.h"
#include "ConfigManager.h"
#include "FileLogger.h"
#include "PipeManager.h"
#include "ICoreImage.h"
#include <mutex>

//#define TESTBUILD



class AVCore : ICoreImage
{
public:
	AVCore(ILogger* logger) 
	{ 
		this->logger = logger;
		this->pipeManager = new PipeManager(this);
		this->portServer = new CommPortServer();
		this->manager = new PluginManager(logger);
		this->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (this->stopEvent == NULL)
		{
			throw "Error creating stop event";
		}
	}

	virtual ~AVCore() override
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
	PipeManager * pipeManager;
	PluginManager * manager;
	CommPortServer * portServer;
	ILogger* logger;
	HANDLE stopEvent = INVALID_HANDLE_VALUE;

	// Унаследовано через ICoreImage
	virtual ILogger * getLogger() override;
};