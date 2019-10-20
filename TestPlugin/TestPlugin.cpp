#include "pch.h"
#include "TestPlugin.h"
#include "EventsKM.h"
#include "KMUMcomm.h"
#include "EventsParser.h"
#include <iostream>

AV_EVENT_RETURN_STATUS TestPlugin::callback(int callbackId, void* event)
{
	if (callbackId == 1)
	{
		PAV_MESSAGE message = (PAV_MESSAGE)event;
		if (message->EventType == AvFileCreate)
		{
			AvFSEventCreate eventFSCreate((PAV_EVENT_FILE_CREATE)message->EventBuffer);
			std::cout << "AvFileCreate: " << eventFSCreate.FilePath << "\n";

			if (eventFSCreate.FilePath.find(std::string("testfile.txt")) != std::string::npos)
			{
				return AvEventStatusBlock;
			}
		}
	}
	return AvEventStatusAllow;
}

void TestPlugin::init(IManager * manager)
{
	manager->registerCallback(this, 1);
}
