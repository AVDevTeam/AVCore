
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
	if (command == "EnumeratePlugins")
	{
		std::list<std::string>* pluginsNamesList = pluginManager->getPluginsNames();
		
		json jEnumeratedPlugins;
		json jPluginsArray;

		for (const auto& pluginName : *pluginsNamesList)
		{
			jPluginsArray.push_back(pluginName);
		}

		jEnumeratedPlugins["Plugins"] = jPluginsArray;

		ret = jEnumeratedPlugins.dump();
	}
	// Команда получить информацию конкретному по модулю
	else if(command == "GetPluginInfo")
	{
		json jPluginInfo;

		// Получили плагин
		std::string pluginName = jCommand.find("PluginName").value();
		IPlugin* plugin = pluginManager->getPluginByName(pluginName);
		IConfig* pluginConfig = plugin->getConfig();
		
		// Получили список его параметров
		std::map<std::string, ConfigParamType>* paramMap = pluginConfig->getParamMap();

		// Получили его параметры
		for (auto& param : *paramMap)
		{
			if (param.second == DwordParam)
			{
				// Записали параметр DWORD
				jPluginInfo[param.first] = pluginConfig->getDwordParam(param.first);
			}
			else if (param.second == StringParam)
			{
				// Записали параметр String
				jPluginInfo[param.first] = pluginConfig->getStringParam(param.first);
			}
			else if (param.second == ListParam)
			{
				// Записали параметр List
				json jListParam;
				std::list<std::string> *listParam = pluginConfig->getListParam(param.first);
				// List записали в отдельный json 
				for (const auto& strParam : *listParam)
				{
					jListParam.push_back(strParam);
				}
				// И добавляем его в jPluginInfo
				jPluginInfo[param.first] = jListParam;
			}
		}

		ret = jPluginInfo.dump();
	}

	return ret;
}

CommandsManager::CommandsManager(IPluginManagerImage * _core)
{
	pluginManager = _core->getPluginManager();
	messageManager = _core->getMessageManager();
}