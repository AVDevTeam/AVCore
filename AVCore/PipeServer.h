#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

class PipeServer
{
private:
	HANDLE hPipe;
	std::string pipeName;
	std::thread * thread;
	int stopSignal = 0;

public:
	PipeServer(const std::string &_pipeName);
	~PipeServer();

public:
	int createNamedPipe();
	int waitForClient();
	int sendMessage(const std::string &_message);
	int receiveMessage(std::string &_message);
	void start();
	void stop();

private:
	int createSecurityAttributes(SECURITY_ATTRIBUTES *);
	void listen();
};