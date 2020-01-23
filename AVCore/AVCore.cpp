#include "AVCore.h"
#include "UMEventsListener.h"

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
	AV_EVENT_REG_OPEN_KEY testOpenKey = { 0 };
	testOpenKey.keyPath = (wchar_t*)L"TEST_KEY_PATH";
	testOpenKey.keyPathSize = sizeof(L"TEST_KEY_PATH");
	testOpenKey.requestorPID = 111;


	PVOID umMessage;


	manager->processEvent(AvFileCreate, manager->parseKMEvent(AvFileCreate, &testFileCreate), &umMessage);
	manager->processEvent(AvProcessHandleCreate, manager->parseKMEvent(AvProcessHandleCreate, &testProcessHandleCreate), &umMessage);
	manager->processEvent(AvProcessHandleDublicate, manager->parseKMEvent(AvProcessHandleDublicate, &testProcessHandleDublicate), &umMessage);
	manager->processEvent(AvThreadHandleCreate, manager->parseKMEvent(AvThreadHandleCreate, &testThreadHandleCreate), &umMessage);
	manager->processEvent(AvThreadHandleDublicate, manager->parseKMEvent(AvThreadHandleDublicate, &testThreadHandleDublicate), &umMessage);
	manager->processEvent(AvProcessCreate, manager->parseKMEvent(AvProcessCreate, &testProcessCreate), &umMessage);
	manager->processEvent(AvProcessExit, manager->parseKMEvent(AvProcessExit, &testProcessExit), &umMessage);
	manager->processEvent(AvThreadCreate, manager->parseKMEvent(AvThreadCreate, &testThreadCreateExit), &umMessage);
	manager->processEvent(AvThreadExit, manager->parseKMEvent(AvThreadExit, &testThreadCreateExit), &umMessage);
	manager->processEvent(AvImageLoad, manager->parseKMEvent(AvImageLoad, &testImageLoad), &umMessage);
	manager->processEvent(AvRegCreateKey, manager->parseKMEvent(AvRegCreateKey, &testOpenKey), &umMessage);
	manager->processEvent(AvRegOpenKey, manager->parseKMEvent(AvRegOpenKey, &testOpenKey), &umMessage);
}

void AVCore::start(void)
{
	messageManager->outAlert("test alert");
	messageManager->outLog("test log");
	messageManager->outDebug("test debug");
	messageManager->outWarning("test1 warning");

	this->logger->log("AVCore. Starting.");
	manager->addEventParser(AvFileCreate, reinterpret_cast<EventParser*>(new AvFSEventCreateParser()));
	manager->addEventParser(AvProcessHandleCreate, reinterpret_cast<EventParser*>(new AvObEventProcessHandleCreateParser()));
	manager->addEventParser(AvProcessHandleDublicate, reinterpret_cast<EventParser*>(new AvObEventProcessHandleDublicateParser()));
	manager->addEventParser(AvThreadHandleCreate, reinterpret_cast<EventParser*>(new AvObEventThreadHandleCreateParser()));
	manager->addEventParser(AvThreadHandleDublicate, reinterpret_cast<EventParser*>(new AvObEventThreadHandleDublicateParser()));
	manager->addEventParser(AvProcessCreate, reinterpret_cast<EventParser*>(new AvEventProcessCreateParser()));
	manager->addEventParser(AvProcessExit, reinterpret_cast<EventParser*>(new AvEventProcessEixtParser()));
	manager->addEventParser(AvThreadCreate, reinterpret_cast<EventParser*>(new AvEventThreadCreateParser()));
	manager->addEventParser(AvThreadExit, reinterpret_cast<EventParser*>(new AvEventThreadExitParser()));
	manager->addEventParser(AvImageLoad, reinterpret_cast<EventParser*>(new AvEventImageLoadParser()));
	manager->addEventParser(AvRegCreateKey, reinterpret_cast<EventParser*>(new AvEventRegCreateKeyParser()));
	manager->addEventParser(AvRegOpenKey, reinterpret_cast<EventParser*>(new AvEventRegOpenKeyParser()));
	manager->addEventParser(AvApcProcessInject, reinterpret_cast<EventParser*>(new AvEventProcessCreateParser()));
	manager->addEventParser(AvWinApiCall, nullptr);
	this->logger->log("AVCore. Populated parsers map.");

	std::list<std::string>* plugins = manager->getConfig()->getListParam("Plugins");
	this->logger->log("AVCore. Got plugin list.");
	std::string pluginsFolder = manager->getConfig()->getStringParam("PluginsPath");
	this->logger->log("AVCore. Got plugin path: " + pluginsFolder);

	for (std::list<std::string>::iterator it = plugins->begin(); it != plugins->end(); it++)
	{
		this->logger->log("AVCore. Loading plugin from: " + pluginsFolder + (*it));
		manager->loadPlugin(pluginsFolder + (*it));
	}
	this->logger->log("AVCore. Loaded all plugins.");

#ifdef TESTBUILD
	testEventsParsers(this->manager);

	this->logger->log("AVCore. Starting UM events listener.");
	this->umEventsManager = new UMEventsManager(this->manager);
	this->logger->log("AVCore. UM events listener started.");
	this->logger->log("AVCore. Startup finished.");

	// Test plugin load/unload logic
	while (1)
	{
		std::string str;
		std::getline(std::cin, str);
		if (str == "exit")
			break;
		IPlugin* plugin = manager->getPluginByName(str);
		if (plugin != nullptr)
		{
			manager->unloadPlugin(plugin->getName());
			std::cout << "Unloaded plugin\n";
		}
		else
		{
			std::string pluginsFolder = manager->getConfig()->getStringParam("PluginsPath");
			plugin = manager->loadPlugin(pluginsFolder + str + ".dll");
			if (plugin != nullptr)
				std::cout << "Loaded plugin " << plugin->getName() << "\n";
			else
				std::cout << "Plugin not found\n";

		}
	}
	
	pipeManager->join();
#else

	this->logger->log("AVCore. Starting UM events listener.");
	this->umEventsManager = new UMEventsManager(this->manager);
	this->logger->log("AVCore. UM events listener started.");
	this->logger->log("AVCore. Starting KM events listener.");
	this->portServer->start(manager);
	this->logger->log("AVCore. KM events listener started.");
	this->logger->log("AVCore. Startup finished.");
#endif
}

// Реализация ICoreImage

ILogger * AVCore::getLogger()
{
	return logger;
}

SettingsManager * AVCore::getSettingsManager()
{
	return settingsManager;
}

CommandsManager * AVCore::getCommandsManager()
{
	return commandsManager;
}

// Реализация IPluginManagerImage

PluginManager * AVCore::getPluginManager()
{
	return manager;
}

MessageManager * AVCore::getMessageManager()
{
	return messageManager;
}
