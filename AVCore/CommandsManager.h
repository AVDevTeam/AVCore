#pragma once
#include <iostream>
#include <string>

// ������������ � ��������� ������� 
class CommandsManager
{

public:


	void manage(std::string _command);	// ��������� ����������� ��������

	CommandsManager();
	~CommandsManager();
};