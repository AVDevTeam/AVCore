#include "pch.h"
#include "TestPlugin.h"
#include "EventsKM.h"
#include "KMUMcomm.h"
#include "EventsUMInterfaces.h"
#include <iostream>

AV_EVENT_RETURN_STATUS TestPlugin::callback(int callbackId, void* event)
{
	if (callbackId == 1)
	{
		IEventFSCreate* eventFSCreate = reinterpret_cast<IEventFSCreate*>(event);
		std::string fName = eventFSCreate->getFilePath();
		
		std::cout << "AvFileCreate: " << fName << "\n";

		if (fName.find(std::string("testfile.txt")) != std::string::npos)
		{
			return AvEventStatusBlock;
		}
	}
	return AvEventStatusAllow;
}

void TestPlugin::init(IManager * manager)
{
	manager->registerCallback(this, 1);
}
