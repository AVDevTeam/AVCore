#pragma once

#include <windows.h>
#include <iostream>
#include <string>

class PipeServer
{
private:
	HANDLE hPipe;
	std::string pipeName;

public:
	PipeServer(const std::string &_pipeName);
	~PipeServer();

public:
	int createNamedPipe();
	int waitForClient();
	int sendMessage(const std::string &_message);
	int receiveMessage(std::string &_message);

private:
	int createSecurityAttributes(SECURITY_ATTRIBUTES *);
};