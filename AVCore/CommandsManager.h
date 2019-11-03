#pragma once
#include <iostream>
#include <string>
#include "IPluginManagerImage.h"

// ������������ � ��������� ������� 
class CommandsManager
{

private:
	PluginManager * pluginManager;

public:


	std::string manage(std::string _command);	// ��������� ����������� ��������

	CommandsManager(IPluginManagerImage * _core);
	~CommandsManager();
};