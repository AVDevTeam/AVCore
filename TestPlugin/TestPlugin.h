#pragma once
#include "PluginInterface.h"
#include "EventsUMInterfaces.h"

typedef enum {
	CallbackFileCreate,
	CallbackPrHandleCreate,
	CallbackPrHandleDublicate,
	CallbackThHandleCreate,
	CallbackThHandleDublicate,
	CallbackProcessCreate,
	CallbackProcessExit,
	CallbackThreadCreate,
	CallbackThreadExit,
	CallbackImageLoad,
	CallbackRegCreateKey,
	CallbackRegOpenKey,
	CallbackWinApiCall,
} CALLBACK_ID;

class TestPlugin : public IPlugin
{
public:
	// Inherited via IPlugin
	virtual ~TestPlugin() override;
	AV_EVENT_RETURN_STATUS callback(int, void*, void**) override;
	void init(IManager* manager, HMODULE module, IConfig* configManager) override;
	void deinit() override;

	virtual std::string& getName() override;
	virtual HMODULE getModule() override;
	virtual std::string& getDescription() override;
	virtual IConfig* getConfig() override;
private:
	std::string name = std::string("TestPlugin");
	std::string description = std::string("Just a test plugin.");
	HMODULE module;
	IConfig* configManager;
	ILogger* logger;
};