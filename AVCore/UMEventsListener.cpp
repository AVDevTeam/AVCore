#include "UMEventsListener.h"
#include "EventsKMStructures.h"

#include "json.hpp"
using json = nlohmann::json;

void UMEventsManager::listen()
{
	std::string message;

	while (!stopSignal)
	{
		pipe->waitForClient(stopSignal);

		// ѕока сообщени€ вал€тс€
		while (pipe->receiveMessage(message) != -1 && !stopSignal)
		{
			//TODO обработка сообщений
			if (message != "")
			{
				std::cout << "Message: " << message << std::endl;
				auto j = json::parse(message);
				AvEventWinApiCall* eventWinApiCall = new AvEventWinApiCall(pipe->getClientPID(), j["func"], j["args"]);
				pipe->sendMessage(std::to_string(
						(int)this->manager->processEvent(AvWinApiCall, eventWinApiCall, NULL)
					)
				);
				message = "";
			}
			Sleep(500);
		}
	}
}

void UMEventsManager::manage(std::string _command)
{

}

void UMEventsManager::stop()
{
	stopSignal = 1;
}

void UMEventsManager::join()
{
	thread->join();
}

UMEventsManager::UMEventsManager(IManager* manager)
{
	this->manager = manager;
	pipe = new PipeServer(serverName);
	pipe->createNamedPipe();

	thread = new std::thread(&UMEventsManager::listen, this);
}

UMEventsManager::~UMEventsManager()
{
	stop();

	delete pipe;
	pipe = nullptr;
}