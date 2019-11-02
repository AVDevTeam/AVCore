#pragma once
#include <iostream>
#include <string>

//Обрабатывает запросы связанные с настройками
class SettingsManager
{

public:


	void manage(std::string _command);	// Обработка поступающих запросов

	SettingsManager();
	~SettingsManager();
};