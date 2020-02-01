#pragma once
#include <iostream>
#include <string>
#include "IPluginManagerImage.h"

// Обрабатывает и исполняет команды 
class CommandsManager
{

private:
	PluginManager *  pluginManager;
	MessageManager * messageManager;
public:
	std::string manage(std::string _command);	// Обработка поступающих запросов

	CommandsManager(IPluginManagerImage * _core);
	~CommandsManager();
};