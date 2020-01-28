#include "pch.h"
#include "TestPlugin.h"
#include "EventsUMInterfaces.h"
#include "KMUMcomm.h"
#include <iostream>
#include <fstream>
#include <mutex>

/*
This file contains the actual plugin logic.
*/

TestPlugin::~TestPlugin()
{
}

AV_EVENT_RETURN_STATUS TestPlugin::callback(int callbackId, void* event, void** umMessage)
{
	if (callbackId == CallbackFileCreate)
	{
		IEventFSCreate* eventFSCreate = reinterpret_cast<IEventFSCreate*>(event);
		this->logger->log("CallbackFileCreate");
		this->logger->log("\tFile path: " + eventFSCreate->getFilePath());
		this->logger->log("\tRequestor PID: " + std::to_string(eventFSCreate->getRequestorPID()));
	}
	else if (callbackId == CallbackPrHandleCreate)
	{
		IEventObProcessHandleCreate* eventPrHandleCreate = reinterpret_cast<IEventObProcessHandleCreate*>(event);
		this->logger->log("CallbackPrHandleCreate");
		this->logger->log("\tRequestor PID: " + std::to_string(eventPrHandleCreate->getRequestorPID()));
		this->logger->log("\tTarget PID: " + std::to_string(eventPrHandleCreate->getTargetPID()));
		this->logger->log("\tDesired access: " + std::to_string(eventPrHandleCreate->getDesiredAccess()));
	}
	else if (callbackId == CallbackPrHandleDublicate)
	{
		IEventObProcessHandleDublicate* eventPrHandleDublicate = reinterpret_cast<IEventObProcessHandleDublicate*>(event);
		this->logger->log("CallbackPrHandleDublicate");
		this->logger->log("\tRequestor PID: " + std::to_string(eventPrHandleDublicate->getRequestorPID()));
		this->logger->log("\tTarget PID: " + std::to_string(eventPrHandleDublicate->getTargetPID()));
		this->logger->log("\tDesired access: " + std::to_string(eventPrHandleDublicate->getDesiredAccess()));
		this->logger->log("\tSource dublicate PID: " + std::to_string(eventPrHandleDublicate->getDublicateSourcePID()));
		this->logger->log("\tTarget dublicate PID: " + std::to_string(eventPrHandleDublicate->getDublicateTargetPID()));
	}
	else if (callbackId == CallbackThHandleCreate)
	{
		IEventObThreadHandleCreate* eventThHandleCreate = reinterpret_cast<IEventObThreadHandleCreate*>(event);
		this->logger->log("CallbackThHandleCreate");
		this->logger->log("\tRequestor PID: " + std::to_string(eventThHandleCreate->getRequestorPID()));
		this->logger->log("\tRequestor TID: " + std::to_string(eventThHandleCreate->getRequestorTID()));
		this->logger->log("\tTarget PID: " + std::to_string(eventThHandleCreate->getTargetPID()));
		this->logger->log("\tTarget TID: " + std::to_string(eventThHandleCreate->getTargetTID()));
		this->logger->log("\tDesired access: " + std::to_string(eventThHandleCreate->getDesiredAccess()));
	}
	else if (callbackId == CallbackThHandleDublicate)
	{
		IEventObThreadHandleDublicate* eventThHandleDublicate = reinterpret_cast<IEventObThreadHandleDublicate*>(event);
		this->logger->log("CallbackThHandleDublicate");
		this->logger->log("\tRequestor PID: " + std::to_string(eventThHandleDublicate->getRequestorPID()));
		this->logger->log("\tRequestor TID: " + std::to_string(eventThHandleDublicate->getRequestorTID()));
		this->logger->log("\tTarget PID: " + std::to_string(eventThHandleDublicate->getTargetPID()));
		this->logger->log("\tTarget TID: " + std::to_string(eventThHandleDublicate->getTargetTID()));
		this->logger->log("\tDesired access: " + std::to_string(eventThHandleDublicate->getDesiredAccess()));
		this->logger->log("\tSource dublicate PID: " + std::to_string(eventThHandleDublicate->getDublicateSourcePID()));
		this->logger->log("\tTarget dublicate PID: " + std::to_string(eventThHandleDublicate->getDublicateTargetPID()));
	}
	else if (callbackId == CallbackProcessCreate)
	{
		IEventProcessCreate* eventProcessCreate = reinterpret_cast<IEventProcessCreate*>(event);
		this->logger->log("CallbackProcessCreate");
		this->logger->log("\tNew process PID: " + std::to_string(eventProcessCreate->getPID()));
		this->logger->log("\tParent PID: " + std::to_string(eventProcessCreate->getParentPID()));
		this->logger->log("\tCreating PID: " + std::to_string(eventProcessCreate->getCreatingPID()));
		this->logger->log("\tCreating TID: " + std::to_string(eventProcessCreate->getCreatingTID()));
		this->logger->log("\tImage file name: " + eventProcessCreate->getImageFileName());
		this->logger->log("\tCommand line: " + eventProcessCreate->getCommandLine());
	}
	else if (callbackId == CallbackProcessExit)
	{
		IEventProcessExit* eventProcessExit = reinterpret_cast<IEventProcessExit*>(event);
		this->logger->log("CallbackProcessExit");
		this->logger->log("\tProcess exits. PID: " + std::to_string(eventProcessExit->getPID()));
	}
	else if (callbackId == CallbackThreadCreate)
	{
		IEventThreadCreate* eventThreadCreate = reinterpret_cast<IEventThreadCreate*>(event);
		this->logger->log("CallbackThreadCreate");
		this->logger->log("\tNew thread in process with PID: " + std::to_string(eventThreadCreate->getPID()));
		this->logger->log("\tNew thread TID: " + eventThreadCreate->getTID());
	}
	else if (callbackId == CallbackThreadExit)
	{
		IEventThreadExit* eventThreadExit = reinterpret_cast<IEventThreadExit*>(event);
		this->logger->log("CallbackThreadExit");
		this->logger->log("\tThread exits in process with PID: " + std::to_string(eventThreadExit->getPID()));
		this->logger->log("\tExiting thread TID: " + std::to_string(eventThreadExit->getTID()));
	}
	else if (callbackId == CallbackImageLoad)
	{
		IEventImageLoad* eventImageLoad = reinterpret_cast<IEventImageLoad*>(event);
		this->logger->log("CallbackImageLoad");
		this->logger->log("\tModule loads into process with PID: " + std::to_string(eventImageLoad->getPID()));
		this->logger->log("\tisSystemModule: " + std::to_string(eventImageLoad->getIsSystemModule()));
		this->logger->log("\tImage name: " + eventImageLoad->getImageName());
	}
	else if (callbackId == CallbackRegCreateKey)
	{
		IEventRegCreateKey* eventRegCreateKey = reinterpret_cast<IEventRegCreateKey*>(event);
		this->logger->log("CallbackRegCreateKey");
		this->logger->log("\tPID: " + std::to_string(eventRegCreateKey->getRequestorPID()));
		this->logger->log("\tProcess PID: " + std::to_string(eventRegCreateKey->getRequestorPID()));
		this->logger->log("\tKey path: " + eventRegCreateKey->getKeyPath());
	}
	else if (callbackId == CallbackRegOpenKey)
	{
		IEventRegOpenKey* eventRegOpenKey = reinterpret_cast<IEventRegOpenKey*>(event);
		this->logger->log("CallbackRegOpenKey");
		this->logger->log("\tPID: " + std::to_string(eventRegOpenKey->getRequestorPID()));
		this->logger->log("\tProcess PID: " + std::to_string(eventRegOpenKey->getRequestorPID()));
		this->logger->log("\tKey path: " + eventRegOpenKey->getKeyPath());
	}
	else if (callbackId == CallbackWinApiCall)
	{
		IEventWinApiCall* eventWinApiCall = reinterpret_cast<IEventWinApiCall*>(event);
		this->logger->log("CallbackWinApiCall");
		this->logger->log("\tPID: " + std::to_string(eventWinApiCall->getPID()));
		this->logger->log("\tFunction: " + eventWinApiCall->getFunctionName());
		auto args = eventWinApiCall->getFunctionArgs();
		this->logger->log("\tArguments:");
		for (auto it = args.begin(); it != args.end(); it++)
			this->logger->log("\t\t- " + (*it));
	}
	return AvEventStatusAllow;
}

void TestPlugin::init(IManager * manager, HMODULE module, IConfig * config)
{
	this->module = module;
	this->logger = manager->getLogger();

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

	manager->registerCallback(this, CallbackFileCreate, AvFileCreate, 100);

	manager->registerCallback(this, CallbackPrHandleCreate, AvProcessHandleCreate, 100);
	manager->registerCallback(this, CallbackPrHandleDublicate, AvProcessHandleDublicate , 100);
	
	manager->registerCallback(this, CallbackThHandleCreate, AvThreadHandleCreate , 100);
	manager->registerCallback(this, CallbackThHandleDublicate, AvThreadHandleDublicate , 100);

	manager->registerCallback(this, CallbackProcessCreate, AvProcessCreate, 100);
	manager->registerCallback(this, CallbackProcessExit, AvProcessExit, 100);

	manager->registerCallback(this, CallbackThreadCreate, AvThreadCreate, 100);
	manager->registerCallback(this, CallbackThreadExit, AvThreadExit, 100);

	manager->registerCallback(this, CallbackImageLoad, AvImageLoad, 100);

	manager->registerCallback(this, CallbackRegCreateKey, AvRegCreateKey, 100);
	manager->registerCallback(this, CallbackRegOpenKey, AvRegOpenKey, 100);

	manager->registerCallback(this, CallbackWinApiCall, AvWinApiCall, 100);
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
