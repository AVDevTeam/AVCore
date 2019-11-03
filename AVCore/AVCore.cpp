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

	std::list<std::string>* plugins = manager->getConfig()->getListParam("Plugins");
	std::string pluginsFolder = manager->getConfig()->getStringParam("PluginsPath");

	for (std::list<std::string>::iterator it = plugins->begin(); it != plugins->end(); it++)
		manager->loadPlugin(pluginsFolder + (*it));

#ifdef TESTBUILD
	testEventsParsers(this->manager);
#else
	this->umEventsManager = new UMEventsManager(this->manager);
	this->portServer->start(manager);
#endif
}
