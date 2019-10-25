#include "SettingsManager.h"



void SettingsManager::listen()
{
	std::string message;

	while (!stopSignal)
	{
		pipe->waitForClient(stopSignal);

		// ���� ��������� �������
		while (pipe->receiveMessage(message) != -1 && !stopSignal)
		{
			//TODO ��������� ���������
			if (message != "")
			{
				std::cout << "Message: " << message << std::endl;
				message = "";
			}
			Sleep(500);
		}
	}
}

void SettingsManager::manage(std::string _command)
{

}

void SettingsManager::stop()
{
	stopSignal = 1;
}

SettingsManager::SettingsManager()
{
	pipe = new PipeServer(serverName);
	pipe->createNamedPipe();

	thread = new std::thread(&SettingsManager::listen, this);
}

SettingsManager::~SettingsManager()
{
	stop();

	delete pipe;
	pipe = nullptr;
}