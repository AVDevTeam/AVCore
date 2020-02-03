
#include "CommandsManager.h"
#include "IPluginManagerImage.h"
#include "../Dependencies/json.hpp"
#include <map>

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

		json jEnumeratedPlugins;	// ������� � �� ��������
		json jPropList;				// �������� - ������� ��, ��������, ������

		// �������� �� ���� ��������
		for (std::string pluginName : *pluginsNamesList)
		{
			// ������ .dll � �������� �� � ������ �� ��������
			pluginName.erase(pluginName.find(".dll"), 4);
			jPropList["IsRun"] = 0;
			jEnumeratedPlugins[pluginName] = jPropList;

			// ���� �����-�� �� �������� �����������, �� ��������� �� 1
			for (const auto& loadedPluginName : *loadedPluginsNamesList)
			{
				if (pluginName == loadedPluginName)
				{
					IPlugin* plugin = pluginManager->getPluginByName(pluginName);
					
					jPropList["IsRun"] = 1;
					jPropList["Version"] = plugin->getVersion();
					jPropList["Description"] = plugin->getDescription();

					jEnumeratedPlugins[pluginName] = jPropList;
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
	// Change plugins settings
	else if (command == "ChangePluginSettings")
	{
		std::string pluginName = jCommand.find("PluginName").value();
		IPlugin* plugin = this->pluginManager->getPluginByName(pluginName);
		if (plugin != nullptr)
		{
			// ���� � �����������
			// ������ ����������� - Param1 : [p1, p2], Param2 : [p1]
			std::map<std::string, std::list<std::string>> changedParams;

			// ���������� ���� � �����������
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

			for (std::map<std::string, std::list<std::string>>::iterator it = changedParams.begin(); it != changedParams.end(); ++it)
			{
				std::string param = (*it).first;
				std::list<std::string> values = (*it).second;
				paramMap* pluginParams = plugin->getConfig()->getParamMap();
				if (pluginParams->find(param) != pluginParams->end())
				{ // valid param name
					switch (pluginParams->at(param))
					{
					case DwordParam:
						if (values.size() > 0)
						{
							DWORD value = 0;
							try
							{
								value = std::stoi(values.front());
								plugin->getConfig()->setDwordParam(param, value);
							}
							catch (std::invalid_argument const& e)
							{
								continue;
							}
							catch (std::out_of_range const& e)
							{
								continue;
							}
						}
						break;
					case StringParam:
						if (values.size() > 0)
						{
							plugin->getConfig()->setStringParam(param, values.front());
						}
						break;
					case ListParam:
						plugin->getConfig()->setListParam(param, values);
						break;
					default:
						break;
					}
				}
			}
			ret = "Configs set";
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