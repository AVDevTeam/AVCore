
#include "CommandsManager.h"
#include "IPluginManagerImage.h"
#include "json.hpp"
using json = nlohmann::json;

//https://habr.com/ru/company/infopulse/blog/254075/	- guide
//https://github.com/nlohmann/json/releases/tag/v3.7.0	- last release

// ������ ������� �� GUI, ���������� ����� �� ��� ����� PipeManager �������� �� �������
std::string CommandsManager::manage(std::string _command)
{
	messageManager->outWarning("Manage function were called");
	std::string ret = "";
	json jCommand = json::parse(_command);
	std::string command = jCommand.find("CommandName").value();

	// ������� ����������� ������
	if (command == "EnumeratePlugins")
	{
		std::list<std::string>* pluginsNamesList = pluginManager->getConfig()->getListParam("Plugins");
		std::list<std::string>* loadedPluginsNamesList = pluginManager->getPluginsNames();

		json jEnumeratedPlugins;

		// �������� �� ���� ��������
		for (std::string pluginName : *pluginsNamesList)
		{
			// ������ .dll � �������� �� � ������ �� ��������
			pluginName.erase(pluginName.find(".dll"), 4);
			jEnumeratedPlugins[pluginName] = 0;

			// ���� �����-�� �� �������� �����������, �� ��������� �� 1
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
	// ������� �������� ���������� ����������� �� ������
	else if (command == "GetPluginInfo")
	{
		json jPluginInfo;

		// �������� ������
		std::string pluginName = jCommand.find("PluginName").value();
		IPlugin* plugin = pluginManager->getPluginByName(pluginName);
		IConfig* pluginConfig = plugin->getConfig();

		// �������� ������ ��� ����������
		std::map<std::string, ConfigParamType>* paramMap = pluginConfig->getParamMap();

		// �������� ��� ���������
		for (auto& param : *paramMap)
		{
			if (param.second == DwordParam)
			{
				// �������� �������� DWORD
				jPluginInfo[param.first] = pluginConfig->getDwordParam(param.first);
			}
			else if (param.second == StringParam)
			{
				// �������� �������� String
				jPluginInfo[param.first] = pluginConfig->getStringParam(param.first);
			}
			else if (param.second == ListParam)
			{
				// �������� �������� List
				json jListParam;
				std::list<std::string> *listParam = pluginConfig->getListParam(param.first);
				// List �������� � ��������� json 
				for (const auto& strParam : *listParam)
				{
					jListParam.push_back(strParam);
				}
				// � ��������� ��� � jPluginInfo
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
	return ret;
}

CommandsManager::CommandsManager(IPluginManagerImage * _core)
{
	pluginManager = _core->getPluginManager();
	messageManager = _core->getMessageManager();
}