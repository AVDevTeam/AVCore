#include "FileLogger.h"

FileLogger::~FileLogger()
{
	fclose(this->logFile);
}

void FileLogger::log(std::string input)
{
	this->logMutex.lock();
	fprintf(this->logFile, "%s\n", input.c_str());
	fflush(this->logFile);
	this->logMutex.unlock();
}
