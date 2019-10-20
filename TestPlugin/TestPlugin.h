#pragma once
#include "PluginInterface.h"
#include "EventsUMInterfaces.h"

class TestPlugin : public IPlugin
{
public:
	// Inherited via IPlugin
	AV_EVENT_RETURN_STATUS callback(int, void*) override;
	void init(IManager* manager) override;
	virtual std::string& getName() override;

private:
	std::string name = std::string("TEST PLUGIN");


};