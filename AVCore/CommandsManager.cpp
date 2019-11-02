
#include "CommandsManager.h"

#include "../Dependencies/json.hpp"
using json = nlohmann::json;

//https://habr.com/ru/company/infopulse/blog/254075/	- guide
//https://github.com/nlohmann/json/releases/tag/v3.7.0	- last release

void CommandsManager::manage(std::string _command)
{
	//TODO Parse command

	json jCommand = json::parse(_command);
	std::string command = jCommand.find("CommandName").value();

	if (command == "EnumeraModules")
	{
		//TODO enumerate
	}
}

CommandsManager::CommandsManager()
{

}