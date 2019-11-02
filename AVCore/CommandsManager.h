#pragma once
#include <iostream>
#include <string>

// Обрабатывает и исполняет команды 
class CommandsManager
{

public:


	void manage(std::string _command);	// Обработка поступающих запросов

	CommandsManager();
	~CommandsManager();
};