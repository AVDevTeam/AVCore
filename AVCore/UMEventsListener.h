#pragma once

#include "PipeServer.h"
#include "EventsParser.h"
#include "EventsUMInterfaces.h"
#include "PluginInterface.h"

// Do some stuff with settings 
class UMEventsManager
{
private:
	IManager* manager;
	std::thread* thread;
	int stopSignal = 0;

	void listen();
	void stop();						// Прервать listen()
	void manage(std::string _command);	// Обработка поступающих команд

public:
	const std::string serverName = AV_UM_EVENTS_PIPE_NAME;
	PipeServer* pipe;

	void join();

	UMEventsManager(IManager* manager);
	~UMEventsManager();
};