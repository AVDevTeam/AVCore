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

public:
	PipeServer(const std::string &_pipeName);
	~PipeServer();

	ULONG getClientPID()
	{
		ULONG PID = 0;
		if (GetNamedPipeClientProcessId(this->hPipe, &PID))
			return PID;
		throw "GetNamedPipeClientProcessId error";
	}

public:
	int createNamedPipe();
	int waitForClient(int &_stopSignal);
	int sendMessage(const std::string &_message);
	int receiveMessage(std::string &_message);

private:
	int createSecurityAttributes(SECURITY_ATTRIBUTES *);
	
};