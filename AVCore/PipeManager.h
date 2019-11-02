#pragma once

#include "PipeServer.h"
#include "ICoreImage.h"

// Do some stuff with settings 
class PipeManager
{
private:
	ICoreImage * AVCoreImage;
	ILogger * logger;

	std::thread * thread;
	int stopSignal = 0;

	void listen();
	void stop();						// Прервать listen()
	void manage(std::string _command);	// Обработка поступающих команд

public:
	const std::string serverName = "\\\\.\\pipe\\AVCoreConnection";
	PipeServer *pipe;
	
	void join();

	PipeManager(ICoreImage * _AVCoreImage);
	~PipeManager();
};