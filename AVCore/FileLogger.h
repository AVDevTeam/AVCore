/*
Implemets simple file logger.
*/

#pragma once
#include "PluginInterface.h"
#include <mutex>

class FileLogger : public ILogger
{
public:
	virtual ~FileLogger() override;
	FileLogger(std::string filePath) 
	{ 
		this->filePath = filePath; 
		fopen_s(&this->logFile, this->filePath.c_str(), "a+");
	}

	// Inherited via ILogger
	virtual void log(std::string) override;
private:
	// syncronizes output to the file.
	std::mutex logMutex;
	std::string filePath;
	FILE* logFile;
};