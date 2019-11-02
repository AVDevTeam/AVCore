#include "PipeManager.h"



void PipeManager::listen()
{
	std::string message;

	while (!stopSignal)
	{
		pipe->waitForClient(stopSignal);

		// Пока сообщения валятся
		while (pipe->receiveMessage(message) != -1 && !stopSignal)
		{
			if (message != "")
			{
				// Если запрос на выполнение команды 
				if (message.substr(1, 6) == R"("id":0)")
				{
					commandsManager->manage(message.substr(0, message.size() - 2));
				}
				// Если запрос на изменение настроек
				else if (message.substr(1, 6) == R"("id":1)")
				{
					settingsManager->manage(message.substr(0, message.size() - 2));
				}
			}
			Sleep(500);
		}
	}
}

void PipeManager::stop()
{
	stopSignal = 1;
}

void PipeManager::join()
{
	thread->join();
}

PipeManager::PipeManager(ICoreImage * _AVCoreImage)
{
	logger = _AVCoreImage->getLogger();
	settingsManager = _AVCoreImage->getSettingsManager();
	commandsManager = _AVCoreImage->getCommandsManager();

	pipe = new PipeServer(serverName);
	pipe->createNamedPipe();
	
	thread = new std::thread(&PipeManager::listen, this);
}

PipeManager::~PipeManager()
{
	stop();

	delete pipe;
	pipe = nullptr;
}