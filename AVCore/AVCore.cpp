#include "AVCore.h"

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

	manager->loadPlugin("TestPlugin.dll");

	portServer->start(manager);
}
