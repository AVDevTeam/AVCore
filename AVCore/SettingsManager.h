#pragma once

#include "PipeServer.h"

// Do some stuff with settings 
class SettingsManager
{
private:
	std::thread * thread;
	int stopSignal = 0;

	void listen();
	void stop();						// Прервать listen()
	void manage(std::string _command);	// Обработка поступающих команд

public:
	const std::string serverName = "\\\\.\\pipe\\AVCoreSettings";
	PipeServer *pipe;
	
	SettingsManager();
	~SettingsManager();
};