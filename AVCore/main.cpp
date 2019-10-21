#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "PipeServer.h"
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
	const std::string pipeName = "\\\\.\\pipe\\AVCorePipe";
	PipeServer pipe(pipeName);

	pipe.createNamedPipe();
	pipe.waitForClient();

	pipe.sendMessage("HELLO");
	
	std::string message;
	pipe.receiveMessage(message);

	getchar();
}
*/

int _cdecl
main(
	_Unreferenced_parameter_ int argc,
	_Unreferenced_parameter_ char* argv[]
)
{

	UMModuleConfig configMgr;
	configMgr.init("PluginManager");
	DWORD test = configMgr.getDwordParam("TEST");
	configMgr.setDwordParam("TEST", 123);
	test = configMgr.getDwordParam("TEST");
	std::string testStr = configMgr.getStringParam("STR");
	configMgr.setStringParam("STR", "newstr");
	testStr = configMgr.getStringParam("STR");

	std::list<std::string> testList;
	testList.push_back("VAL1");
	testList.push_back("VAL2");
	testList.push_back("VAL3");
	configMgr.setListParam("LIST", testList);

	std::list<std::string>* resList = configMgr.getListParam("LIST");

	printf("press any key to start\n");
	getchar();

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	PluginManager manager;
	manager.addEventParser(AvFileCreate, reinterpret_cast<EventParser*>(new AvFSEventCreateParser()));

	CommPortServer portServer;
	portServer.start(&manager);

	std::cout << "$ ";
	for (std::string cmd; std::getline(std::cin, cmd);)
	{

		if (cmd == std::string("load"))
		{
			manager.loadPlugin((char*)"TestPlugin.dll");
		}
		else if (cmd == std::string("unload"))
		{
			if (manager.getPluginByName("TestPlugin.dll") != nullptr)
				manager.unloadPlugin("TestPlugin.dll");
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
