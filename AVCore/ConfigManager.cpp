#include "ConfigManager.h"

void UMModuleConfig::init(std::string moduleId)
{
	this->regStorePath = std::string("SOFTWARE\\AVCore\\") + moduleId;
	LSTATUS status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, this->regStorePath.c_str(), NULL, KEY_QUERY_VALUE | KEY_SET_VALUE, &this->configKey);
	if (status != ERROR_SUCCESS)
		throw "Open key error";
}

void UMModuleConfig::setParamMap(paramMap* configParamMap)
{
	this->configParamMap = configParamMap;
}

DWORD UMModuleConfig::getDwordParam(std::string paramName)
{
	DWORD type = NULL;
	DWORD data = NULL;
	DWORD dataSize = sizeof(DWORD);
	LSTATUS status = RegQueryValueExA(this->configKey, paramName.c_str(), NULL, &type, (LPBYTE)&data, &dataSize);
	if (status != ERROR_SUCCESS)
		throw "Error reading dword from registry";
	return data;
}

std::string UMModuleConfig::getStringParam(std::string paramName)
{
	DWORD type = NULL;
	char * data = (char*)1;
	DWORD dataSize = NULL;
	LSTATUS status = RegQueryValueExA(this->configKey, paramName.c_str(), NULL, &type, (LPBYTE)data, &dataSize);
	if (status == ERROR_MORE_DATA)
	{
		data = (char*)malloc(sizeof(char) * dataSize);
		if (data == NULL)
			throw std::bad_alloc();
		status = RegQueryValueExA(this->configKey, paramName.c_str(), NULL, &type, (LPBYTE)data, &dataSize);
		if (status != ERROR_SUCCESS)
		{
			free(data);
			throw "Error reading registry string value";
		}
		std::string result(data);
		free(data);
		return result;
	}
	else if (status != ERROR_SUCCESS)
		throw "Error reading registry string value";
	
	
	return std::string(data);
}

std::list<std::string>* UMModuleConfig::getListParam(std::string paramName)
{
	std::string rawStringList = this->getStringParam(paramName);
	std::list<std::string>* result = new std::list<std::string>();
	std::string delimiter(";");
	std::string token;
	int pos;
	// split string by delimiter
	while ((pos = rawStringList.find(delimiter)) != std::string::npos)
	{
		token = rawStringList.substr(0, pos);
		result->push_back(token);
		rawStringList.erase(0, pos + delimiter.length());
	}
	return result;
}

void UMModuleConfig::setDwordParam(std::string& paramName, DWORD value)
{
	LSTATUS status = RegSetValueExA(this->configKey, paramName.c_str(), NULL, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
	if (status != ERROR_SUCCESS)
		throw "Error reading dword from registry";
}

void UMModuleConfig::setStringParam(std::string& paramName, std::string& value)
{
	std::string local(value);
	LSTATUS status = RegSetValueExA(this->configKey, paramName.c_str(), NULL, REG_SZ, (LPBYTE)local.c_str(), value.size() + 1);
	if (status != ERROR_SUCCESS)
		throw "Error reading dword from registry";
}

void UMModuleConfig::setListParam(std::string& paramName, std::list<std::string>& value)
{
	std::string rawListString("");
	for (std::list<std::string>::iterator it = value.begin(); it != value.end(); it++)
	{
		rawListString += (*it) + ";";
	}
	this->setStringParam(paramName, rawListString);
}

std::map<std::string, ConfigParamType>* UMModuleConfig::getParamMap()
{
	return this->configParamMap;
}
