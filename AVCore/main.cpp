#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "SettingsManager.h"
#include "CommPortServer.h"
#include "PluginManager.h"
#include "EventsParser.h"
#include "ConfigManager.h"

/*
int _cdecl
main(
	_Unreferenced_parameter_ int argc,
	_Unreferenced_parameter_ char* argv[]
)
{
	SettingsManager *settingManager = new SettingsManager();



	getchar();
	delete settingManager;
}
*/

void testEventsParsers(PluginManager* manager)
{
	AV_EVENT_THREAD_HANDLE_CREATE testThreadCreate = { 0 };
	testThreadCreate.RequestorPID = 111;
	testThreadCreate.DesiredAccess = 222;
	testThreadCreate.RequestorTID = 333;
	testThreadCreate.TargetPID = 444;
	testThreadCreate.TargetTID = 555;
	AV_EVENT_THREAD_HANDLE_DUBLICATE testThreadDublicate = { 0 };
	testThreadDublicate.RequestorPID = 666;
	testThreadDublicate.DesiredAccess = 777;
	testThreadDublicate.RequestorTID = 888;
	testThreadDublicate.TargetPID = 999;
	testThreadDublicate.TargetTID = 1111;
	testThreadDublicate.DublicateSourcePID = 2222;
	testThreadDublicate.DublicateTargetPID = 3333;
	AV_EVENT_PROCESS_CREATE testProcessCreate = { 0 };
	testProcessCreate.PID = 111;
	testProcessCreate.parentPID = 222;
	testProcessCreate.creatingPID = 333;
	testProcessCreate.creatingTID = 444;
	testProcessCreate.imageFileName = (wchar_t*)L"TEST_IMAGE_PATH";
	testProcessCreate.imageFileNameSize = sizeof(L"TEST_IMAGE_PATH");
	testProcessCreate.commandLine = (wchar_t*)L"TEST_COMMAND_LINE";
	testProcessCreate.commandLineSize = sizeof(L"TEST_COMMAND_LINE");

	manager->processEvent(AvThreadHandleCreate, &testThreadCreate);
	manager->processEvent(AvThreadHandleDublicate, &testThreadDublicate);
	manager->processEvent(AvProcessCreate, &testProcessCreate);
}

int _cdecl
main(
	_Unreferenced_parameter_ int argc,
	_Unreferenced_parameter_ char* argv[]
)
{

	printf("press any key to start\n");
	getchar();

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	PluginManager manager;
	manager.addEventParser(AvFileCreate, reinterpret_cast<EventParser*>(new AvFSEventCreateParser()));
	manager.addEventParser(AvProcessHandleCreate, reinterpret_cast<EventParser*>(new AvObEventProcessHandleCreateParser()));
	manager.addEventParser(AvProcessHandleDublicate, reinterpret_cast<EventParser*>(new AvObEventProcessHandleDublicateParser()));
	manager.addEventParser(AvThreadHandleCreate, reinterpret_cast<EventParser*>(new AvObEventThreadHandleCreateParser()));
	manager.addEventParser(AvThreadHandleDublicate, reinterpret_cast<EventParser*>(new AvObEventThreadHandleDublicateParser()));
	manager.addEventParser(AvProcessCreate, reinterpret_cast<EventParser*>(new AvEventProcessCreateParser()));

	//manager.loadPlugin((char*)"TestPlugin.dll");
	//testEventsParsers(&manager);

	CommPortServer portServer;
	try
	{
		portServer.start(&manager);
	}
	catch (char* ex)
	{
		std::cout << "Exception: " << std::string(ex) << "\n";
		return 1;
	}

	std::cout << "$ ";
	for (std::string cmd; std::getline(std::cin, cmd);)
	{

		if (cmd == std::string("load"))
		{
			manager.loadPlugin((char*)"TestPlugin.dll");
		}
		else if (cmd == std::string("unload"))
		{
			if (manager.getPluginByName("TestPlugin") != nullptr)
				manager.unloadPlugin("TestPlugin");
		}
		else if (cmd == std::string("exit"))
		{
			break;
		}
		else
		{
			std::cout << "Invalid command.\n";
		}
		std::cout << "$ ";
	}
	
	
	portServer.stop();
	return 0;
}