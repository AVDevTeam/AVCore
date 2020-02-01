
#include "CommandsManager.h"
#include "IPluginManagerImage.h"
#include "../Dependencies/json.hpp"
#include <map>

using json = nlohmann::json;

//https://habr.com/ru/company/infopulse/blog/254075/	- guide
//https://github.com/nlohmann/json/releases/tag/v3.7.0	- last release

// Парсит команды от GUI, возвращает ответ на них чтобы PipeManager отправил их обратно
std::string CommandsManager::manage(std::string _command)
{
	messageManager->outWarning("Manage function were called");
	std::string ret = "";
	json jCommand = json::parse(_command);
	std::string command = jCommand.find("CommandName").value();

	// Команда перечислить модули
	if (command == "EnumeratePlugins")
	{
		std::list<std::string>* pluginsNamesList = pluginManager->getConfig()->getListParam("Plugins");
		std::list<std::string>* loadedPluginsNamesList = pluginManager->getPluginsNames();

		json jEnumeratedPlugins;

		// Пройтись по всем плагинам
		for (std::string pluginName : *pluginsNamesList)
		{
			// Убрать .dll и записать их в строку на отправку
			pluginName.erase(pluginName.find(".dll"), 4);
			jEnumeratedPlugins[pluginName] = 0;

			// Если какие-то из плагинов прогруженны, то выставить им 1
			for (const auto& loadedPluginName : *loadedPluginsNamesList)
			{
				if (pluginName == loadedPluginName)
				{
					jEnumeratedPlugins[pluginName] = 1;
					break;
				}
			}
		}
		ret = jEnumeratedPlugins.dump();
	}
	// Команда получить информацию конкретному по модулю
	else if (command == "GetPluginInfo")
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
	// Load pluging command
	else if (command == "LoadPlugin")
	{
		std::string pluginName = jCommand.find("PluginName").value();
		if (this->pluginManager->getPluginByName(pluginName) != nullptr)
		{
			// plugin already loaded. Return error status.
			ret = "Error. Plugin aleready loaded.";
		}
		else
		{
			std::string pluginsFolder = this->pluginManager->getConfig()->getStringParam("PluginsPath");
			if (this->pluginManager->loadPlugin(pluginsFolder + pluginName + ".dll") != nullptr)
				ret = "Plugin loaded.";
			else
				ret = "Plugin not found.";
		}
	}
	// Unload plugin command
	else if (command == "UnloadPlugin")
	{
		std::string pluginName = jCommand.find("PluginName").value();
		IPlugin* pluginToUnload = this->pluginManager->getPluginByName(pluginName);
		if (pluginToUnload != nullptr)
		{
			this->pluginManager->unloadPlugin(pluginToUnload->getName());
			ret = "Plugin unloaded.";
		}
		else
		{
			// Error plugin not found.
			ret = "Plugin not found.";
		}
	}
	// Plugin command
	else if (command == "PluginCommand")
	{
		std::string pluginName = jCommand.find("PluginName").value();
		std::string command = jCommand.find("Command").value();
		std::string args = jCommand.find("Args").value();

		IPlugin* pluginForCommand = this->pluginManager->getPluginByName(pluginName);
		if (pluginForCommand != nullptr)
		{ // if target plugin is loaded
			int status = pluginForCommand->processCommand(command, args);
			ret = std::to_string(status);
		}
		else
		{
			ret = "Plugin not found.";
		}
	}
	// Change plugins settings
	else if (command == "ChangePluginSettings")
	{
		std::string pluginName = jCommand.find("PluginName").value();
		
		// Мапа с параметрами
		// Пример содержимого - Param1 : [p1, p2], Param2 : [p1]
		std::map<std::string, std::list<std::string>>changedParams;
		
		// Заполнение мапы с параметрами
		json jsonChangedParams = jCommand.find("ChangedParams").value();
		for (json::iterator it = jsonChangedParams.begin(); it != jsonChangedParams.end(); ++it) 
		{
			std::list<std::string> list;
			for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2)
			{
				list.push_back(it2.value());
			}
			changedParams[it.key()] = list;
		}


	}



	return ret;
}

CommandsManager::CommandsManager(IPluginManagerImage * _core)
{
	pluginManager = _core->getPluginManager();
	messageManager = _core->getMessageManager();
}