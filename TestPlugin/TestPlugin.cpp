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


void TestPlugin::init(IManager * manager, HMODULE module)
{
	this->module = module;
	manager->registerCallback(this, 1, AvFileCreate, 1);
	manager->registerCallback(this, 2, AvFileCreate, 2);
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
