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

	printf("press any key to start\n");
	getchar();

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	PluginManager manager;
	manager.addEventParser(AvFileCreate, reinterpret_cast<EventParser*>(new AvFSEventCreateParser()));

	manager.loadPlugin((char*)"TestPlugin.dll");

	IConfig* config = manager.getPluginByName("TestPlugin.dll")->getConfig();
	// get plugin parameters list (we don't know them in AVCore.exe). The list of params
	// is provided by plugin.
	paramMap* pMap = config->getParamMap();
	// iterate plugin parameter
	for (paramMap::iterator it = pMap->begin(); it != pMap->end(); it++)
	{
		switch ((*it).second)
		{
		case DwordParam:
			std::cout << "DWORD param " << (*it).first << " = " << config->getDwordParam((*it).first);
			break;
		case StringParam:
			std::cout << "STRING param " << (*it).first << " = " << config->getStringParam((*it).first);
			break;
		case ListParam:
			std::list<std::string>* tmp = config->getListParam((*it).first);
			std::cout << "STRING param " << (*it).first << " = \n";
			for (std::list<std::string>::iterator it2 = tmp->begin(); it2 != tmp->end(); it2++)
				std::cout << "\t" << (*it2) << "\n";
			break;

		}
	}

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
