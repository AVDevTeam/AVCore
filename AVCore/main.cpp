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
	//settingManager->join(); 
	delete settingManager;
}
*/

void testEventsParsers(PluginManager* manager)
{
	AV_EVENT_FILE_CREATE testFileCreate = { 0 };
	testFileCreate.FileName = (wchar_t*)L"TEST_FILE_NAME";
	testFileCreate.FileNameSize = sizeof(L"TEST_FILE_NAME");
	testFileCreate.VolumeName = (wchar_t*)L"TEST_VOLUME_NAME";
	testFileCreate.VolumeNameSize = sizeof(L"TEST_VOLUME_NAME");
	testFileCreate.RequestorMode = 0;
	testFileCreate.RequestorPID = 111;
	AV_EVENT_PROCESS_HANDLE_CREATE testProcessHandleCreate = { 0 };
	testProcessHandleCreate.KernelHandle = 0;
	testProcessHandleCreate.RequestorPID = 111;
	testProcessHandleCreate.TargetPID = 222;
	testProcessHandleCreate.DesiredAccess = 0xFFF;
	AV_EVENT_PROCESS_HANDLE_DUBLICATE testProcessHandleDublicate = { 0 };
	testProcessHandleDublicate.KernelHandle = 0;
	testProcessHandleDublicate.RequestorPID = 111;
	testProcessHandleDublicate.TargetPID = 222;
	testProcessHandleDublicate.DesiredAccess = 0xFFF;
	testProcessHandleDublicate.DublicateSourcePID = 333;
	testProcessHandleDublicate.DublicateTargetPID = 444;
	AV_EVENT_THREAD_HANDLE_CREATE testThreadHandleCreate = { 0 };
	testThreadHandleCreate.RequestorPID = 111;
	testThreadHandleCreate.DesiredAccess = 222;
	testThreadHandleCreate.RequestorTID = 333;
	testThreadHandleCreate.TargetPID = 444;
	testThreadHandleCreate.TargetTID = 555;
	AV_EVENT_THREAD_HANDLE_DUBLICATE testThreadHandleDublicate = { 0 };
	testThreadHandleDublicate.RequestorPID = 666;
	testThreadHandleDublicate.DesiredAccess = 777;
	testThreadHandleDublicate.RequestorTID = 888;
	testThreadHandleDublicate.TargetPID = 999;
	testThreadHandleDublicate.TargetTID = 1111;
	testThreadHandleDublicate.DublicateSourcePID = 2222;
	testThreadHandleDublicate.DublicateTargetPID = 3333;
	AV_EVENT_PROCESS_CREATE testProcessCreate = { 0 };
	testProcessCreate.PID = 111;
	testProcessCreate.parentPID = 222;
	testProcessCreate.creatingPID = 333;
	testProcessCreate.creatingTID = 444;
	testProcessCreate.imageFileName = (wchar_t*)L"TEST_IMAGE_PATH";
	testProcessCreate.imageFileNameSize = sizeof(L"TEST_IMAGE_PATH");
	testProcessCreate.commandLine = (wchar_t*)L"TEST_COMMAND_LINE";
	testProcessCreate.commandLineSize = sizeof(L"TEST_COMMAND_LINE");
	AV_EVENT_PROCESS_EXIT testProcessExit = { 0 };
	testProcessExit.PID = 111;
	AV_EVENT_THREAD_CREATE testThreadCreateExit = { 0 };
	testThreadCreateExit.PID = 111;
	testThreadCreateExit.TID = 222;
	AV_EVENT_IMAGE_LOAD testImageLoad = { 0 };
	testImageLoad.PID = 111;
	testImageLoad.systemModeImage = 0;
	testImageLoad.imageName = (wchar_t*)L"TEST_IMGAE_NAME";
	testImageLoad.imageNameSize = sizeof(L"TEST_IMGAE_NAME");

	manager->processEvent(AvFileCreate, &testFileCreate);
	manager->processEvent(AvProcessHandleCreate, &testProcessHandleCreate);
	manager->processEvent(AvProcessHandleDublicate, &testProcessHandleDublicate);
	manager->processEvent(AvThreadHandleCreate, &testThreadHandleCreate);
	manager->processEvent(AvThreadHandleDublicate, &testThreadHandleDublicate);
	manager->processEvent(AvProcessCreate, &testProcessCreate);
	manager->processEvent(AvProcessExit, &testProcessExit);
	manager->processEvent(AvThreadCreate, &testThreadCreateExit);
	manager->processEvent(AvThreadExit, &testThreadCreateExit);
	manager->processEvent(AvImageLoad, &testImageLoad);
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
	manager.addEventParser(AvProcessExit, reinterpret_cast<EventParser*>(new AvEventProcessEixtParser()));
	manager.addEventParser(AvThreadCreate, reinterpret_cast<EventParser*>(new AvEventThreadCreateParser()));
	manager.addEventParser(AvThreadExit, reinterpret_cast<EventParser*>(new AvEventThreadExitParser()));
	manager.addEventParser(AvImageLoad, reinterpret_cast<EventParser*>(new AvEventImageLoadParser()));
		
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