#include "ConfigManager.h"

void UMModuleConfig::init(std::string moduleId)
{
	this->regStorePath = std::string("Software\\AVCore\\") + moduleId;
	LSTATUS status = RegCreateKeyExA(HKEY_LOCAL_MACHINE, this->regStorePath.c_str(), NULL, NULL, NULL, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &this->configKey, NULL);
	if (status != ERROR_SUCCESS)
	{
		this->logger->log("UMModuleConfig::init. Open key error.");
		throw "Open key error";
	}
}

void UMModuleConfig::deinit()
{
	LSTATUS status = RegCloseKey(this->configKey);
	if (status != ERROR_SUCCESS)
	{
		this->logger->log("UMModuleConfig::deinit. Close key error.");
		throw "Close key error";
	}
	if (this->configParamMap != nullptr)
		delete this->configParamMap;
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
	{
		this->logger->log("UMModuleConfig::getDwordParam. Error reading dword from registry. " + paramName);
		throw "Error reading dword from registry";
	}
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
			this->logger->log("UMModuleConfig::getStringParam. Error reading registry string value. " + paramName);
			throw "Error reading registry string value";
		}
		std::string result(data);
		free(data);
		return result;
	}
	else if (status != ERROR_SUCCESS)
	{
		this->logger->log("UMModuleConfig::getStringParam. Error reading registry string value. " + paramName);
		throw "Error reading registry string value";
	}
	
	
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
	{
		this->logger->log("UMModuleConfig::setDwordParam. Error setting dword in registry. " + paramName);
		throw "Error setting dword in registry";
	}
}

void UMModuleConfig::setStringParam(std::string& paramName, std::string& value)
{
	std::string local(value);
	LSTATUS status = RegSetValueExA(this->configKey, paramName.c_str(), NULL, REG_SZ, (LPBYTE)local.c_str(), (DWORD)value.size() + 1);
	if (status != ERROR_SUCCESS)
	{
		this->logger->log("UMModuleConfig::setStringParam. Error setting string in registry. " + paramName);
		throw "Error setting string in registry";
	}
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

/**
\brief checks if parameter exists in registry.

TODO! Check param type.
*/
bool UMModuleConfig::checkParamSet(std::string paramName)
{
	DWORD size = 0;
	DWORD type = 0;
	LSTATUS status = RegQueryValueExA(this->configKey, paramName.c_str(), NULL, &type, NULL, &size);
	return (status != ERROR_FILE_NOT_FOUND);
}
