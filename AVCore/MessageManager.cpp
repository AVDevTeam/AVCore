#include "MessageManager.h"
#include "../Dependencies/json.hpp"
using json = nlohmann::json;

MessageManager::MessageManager()
{

	pipe = new PipeServer(serverName);
	pipe->createNamedPipe();

	thread = new std::thread(&MessageManager::listen, this);
}

MessageManager::~MessageManager()
{
	stop();

	delete pipe;
	pipe = nullptr;
}

void MessageManager::stop()
{
	stopSignal = 1;
}

void MessageManager::join()
{
	thread->join();
}

void MessageManager::listen()
{
	std::string message;

	while (!stopSignal)
	{
		pipe->waitForClient(stopSignal);

		while (pipe->receiveMessage(message) != -1 && !stopSignal)
		{
			if (message == "ready\r\n")
			{
				std::cout << "GUI asking messages" << message << std::endl;

				json messages;
				std::string jMessage;

				for (const auto& t : messagesList)
				{
					// Кладется класс и сообщение пример: ("warning", "warning message")
					messages[std::get<0>(t)] += std::get<1>(t);
				}

				jMessage = messages.dump();
				if (jMessage != "null")
				{
					pipe->sendMessage(jMessage);
					messagesList.clear();
				}

			}
			Sleep(500);
		}



	}
}

void MessageManager::outLog(std::string _message)
{
	std::tuple<std::string, std::string> t("log", _message);
	messagesList.push_back(t);
}
void MessageManager::outAlert(std::string _message)
{
	std::tuple<std::string, std::string> t("alert", _message);
	messagesList.push_back(t);
}
void MessageManager::outWarning(std::string _message)
{
	std::tuple<std::string, std::string> t("warning", _message);
	messagesList.push_back(t);
}
void MessageManager::outDebug(std::string _message)
{
	std::tuple<std::string, std::string> t("debug", _message);
	messagesList.push_back(t);
}