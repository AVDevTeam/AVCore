#include "pch.h"
#include "TestPlugin.h"
#include "EventsUMInterfaces.h"
#include "KMUMcomm.h"
#include <iostream>

/*
This file contains the actual plugin logic.
*/

AV_EVENT_RETURN_STATUS TestPlugin::callback(int callbackId, void* event)
{
	if (callbackId == 1)
	{
		IEventFSCreate* eventFSCreate = reinterpret_cast<IEventFSCreate*>(event);
		std::string fName = eventFSCreate->getFilePath();
		
		if (fName.find(std::string("testfile.txt")) != std::string::npos)
		{
			std::cout << "BLOCKED: " << fName << "\n";
			return AvEventStatusBlock;
		}
	}
	else if (callbackId == 2)
	{
		IEventFSCreate* eventFSCreate = reinterpret_cast<IEventFSCreate*>(event);
		std::string fName = eventFSCreate->getFilePath();
		std::cout << "AvFileCreate: " << fName << "\n";
	}
	return AvEventStatusAllow;
}


void TestPlugin::init(IManager * manager, HMODULE module, IConfig * config)
{
	this->module = module;

	this->configManager = config;
	paramMap* paramMap = new std::map<std::string, ConfigParamType>();
	paramMap->insert(paramPair("BlockList", ListParam));
	paramMap->insert(paramPair("SomeInt", DwordParam));
	paramMap->insert(paramPair("SomeStr", StringParam));

	this->configManager->setParamMap(paramMap);

	std::list<std::string> blockList;
	blockList.push_back("testfile.txt");
	blockList.push_back("eicar.txt");
	blockList.push_back("eicar.com");

	// set default values. TODO check if values were not set.
	std::string param("BlockList");
	this->configManager->setListParam(param, blockList);
	param = "SomeInt";
	this->configManager->setDwordParam(param, 777);
	param = "SomeStr";
	std::string value("A STRING");
	this->configManager->setStringParam(param, value);

	std::list<std::string>* blockListTest = this->getConfig()->getListParam("BlockList");
	delete blockListTest;

	manager->registerCallback(this, 1, AvFileCreate, 1);
	manager->registerCallback(this, 2, AvFileCreate, 2);
}

void TestPlugin::deinit()
{
	delete this->configManager->getParamMap();
	delete this;
}

std::string& TestPlugin::getName()
{
	return this->name;
}

HMODULE TestPlugin::getModule()
{
	return this->module;
}

std::string& TestPlugin::getDescription()
{
	return this->description;
}

IConfig* TestPlugin::getConfig()
{
	return this->configManager;
}
