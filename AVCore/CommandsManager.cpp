
#include "CommandsManager.h"
#include "IPluginManagerImage.h"
#include "../Dependencies/json.hpp"
using json = nlohmann::json;

//https://habr.com/ru/company/infopulse/blog/254075/	- guide
//https://github.com/nlohmann/json/releases/tag/v3.7.0	- last release

// Парсит команды от GUI, возвращает ответ на них чтобы PipeManager отправил их обратно
std::string CommandsManager::manage(std::string _command)
{
	std::string ret = "";
	json jCommand = json::parse(_command);
	std::string command = jCommand.find("CommandName").value();

	// Команда перечислить модули
	if (command == "EnumerateModules")
	{
		std::list<std::string>* pluginsNamesList = pluginManager->getPluginsNames();
		
		json jEnumeratedModules;
		json jModulesArray;

		for (const auto& pluginName : *pluginsNamesList)
		{
			jModulesArray.push_back(pluginName);
		}

		jEnumeratedModules["Modules"] = jModulesArray;

		ret = jEnumeratedModules.dump();
	}

	return ret;
}

CommandsManager::CommandsManager(IPluginManagerImage * _core)
{
	pluginManager = _core->getPluginManager();
}