#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "PipeServer.h"
#include "CommPortServer.h"
#include "PluginManager.h"
#include "EventsParser.h"

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
