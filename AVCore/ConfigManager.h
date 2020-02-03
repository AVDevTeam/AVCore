#pragma once
#include "PluginInterface.h"
#include <list>

class UMModuleConfig : public IConfig
{
public:
	UMModuleConfig(ILogger* logger) { this->logger = logger; }
	// Inherited via IConfig
	virtual void init(std::string moduleId) override;
	virtual void deinit() override;

	virtual void setParamMap(paramMap*) override;
	virtual paramMap* getParamMap() override;

	// checks weather parameter exists
	bool checkParamSet(std::string paramName);

	// parameter getters
	DWORD getDwordParam(std::string paramName);
	std::string getStringParam(std::string paramName);
	std::list<std::string> * getListParam(std::string paramName);

	// parameter setters
	void setDwordParam(std::string& paramName, DWORD value);
	void setStringParam(std::string& paramName, std::string& value);
	void setListParam(std::string& paramName, std::list<std::string>& value);
private:
	// holds the key assosiated with current IConfig instance.
	HKEY configKey;
	// path to configKey
	std::string regStorePath;
	// list of parameters. This map is set in plugins in
	// order to expose parameters available for the plugin.
	paramMap* configParamMap = nullptr;

	ILogger* logger;
};