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
			manage(message);
			Sleep(500);
		}
	}
}

void PipeManager::manage(std::string _message)
{
	logger->log(_message);
	//std::list <std::string> *pluginNames;
	//pluginNames = manager->getPluginsNames();

	// manager.loadPlugin((char*)"TestPlugin");
	// auto pl = manager.getPluginByName("TestPlugin");
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
	AVCoreImage = _AVCoreImage;
	logger = AVCoreImage->getLogger();

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