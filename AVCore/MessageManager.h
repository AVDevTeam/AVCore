#pragma once
#include <iostream>
#include <string>
#include <list>
#include "PipeServer.h"

// Send messages to GUI
class MessageManager
{
private:
	std::list<std::tuple<std::string, std::string>> messagesList;	//id и сообщение. 1 - Log, 2 - Alert, 3 - Warning, 4 - Debug

	std::thread * thread;
	int stopSignal = 0;

	void listen();
	void stop();						// Прервать listen()

public:
	const std::string serverName = "\\\\.\\pipe\\AVCoreConnection2";
	PipeServer *pipe;

	void join();

	void outLog(std::string _message);
	void outAlert(std::string _message);
	void outWarning(std::string _message);
	void outDebug(std::string _message);

	MessageManager();
	~MessageManager();
};