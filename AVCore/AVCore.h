#pragma once
#include "CommPortServer.h"
#include "EventsParser.h"
#include "MessageManager.h"
#include "ConfigManager.h"
#include "FileLogger.h"
#include "PipeManager.h"
#include "IPluginManagerImage.h"
#include "UMEventsListener.h"
#include <mutex>

//#define TESTBUILD

class AVCore : public ICoreImage, public IPluginManagerImage
{
public:
	AVCore(ILogger* logger) 
	{ 
		this->messageManager = new MessageManager();
		this->manager = new PluginManager(logger, (IMessageManager*)this->messageManager);
		this->logger = logger;
		this->commandsManager = new CommandsManager(this);
		this->settingsManager = new SettingsManager();
		this->pipeManager = new PipeManager(this);
		this->logger->log("AVCore. Created instance.");
		this->portServer = new CommPortServer();
		this->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (this->stopEvent == NULL)
		{
			this->logger->log("AVCore(). Error creating stop event");
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
	SettingsManager * settingsManager;
	CommandsManager * commandsManager;

	PluginManager * manager;
	CommPortServer * portServer;
	ILogger* logger;
	MessageManager* messageManager;
	HANDLE stopEvent = INVALID_HANDLE_VALUE;

	// Унаследовано через ICoreImage
	virtual ILogger * getLogger() override;
	virtual SettingsManager * getSettingsManager() override;
	virtual CommandsManager * getCommandsManager() override;

	// Унаследовано через IPluginManagerImage
	virtual PluginManager * getPluginManager() override;
	virtual MessageManager * getMessageManager() override;
	UMEventsManager* umEventsManager;
};