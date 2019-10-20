#include "pch.h"
#include "TestPlugin.h"
#include "EventsUMInterfaces.h"
#include "KMUMcomm.h"
#include <iostream>

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

void TestPlugin::init(IManager * manager)
{
	manager->registerCallback(this, 1, AvFileCreate, 1);
	manager->registerCallback(this, 2, AvFileCreate, 2);
}
