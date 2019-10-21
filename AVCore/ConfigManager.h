#pragma once
#include "PluginInterface.h"
#include <list>

class UMModuleConfig : public IConfig
{
public:
	// Inherited via IConfig
	virtual void init(std::string moduleId) override;
	virtual std::map<std::string, ConfigParamType>* getParamList() override;

	DWORD getDwordParam(std::string paramName);
	std::string getStringParam(std::string paramName);
	std::list<std::string> * getListParam(std::string paramName);

	void setDwordParam(std::string paramName, DWORD value);
	void setStringParam(std::string paramName, std::string value);
	void setListParam(std::string paramName, std::list<std::string>& value);
private:
	HKEY configKey;
	std::string regStorePath;
	
};