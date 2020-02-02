#pragma once
#include <iostream>
#include <string>
#include <list>
#include "PipeServer.h"
#include "PluginInterface.h"

// Send messages to GUI
class MessageManager : IMessageManager
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

	virtual void outLog(std::string _message) override;
	virtual void outAlert(std::string _message) override;
	virtual void outWarning(std::string _message) override;
	virtual void outDebug(std::string _message) override;

	MessageManager();
	~MessageManager();
};