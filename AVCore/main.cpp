#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "SettingsManager.h"
#include "CommPortServer.h"
#include "PluginManager.h"
#include "EventsParser.h"
#include "ConfigManager.h"
#include "FileLogger.h"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME  "AVCore"
#ifdef _WIN64
#define LOG_PATH "\\\\vmware-host\\Shared Folders\\build\\log.txt"
#else
#define LOG_PATH "C:\\log.txt"
#endif

// DEBUG MEMORY LEAKS
/*
#define _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
*/

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

int main(int argc, TCHAR* argv[])
{
	OutputDebugString("AVCore: Main: Entry");
	std::string test("C:\\Users\\user\\Desktop\\secretfile.txt");
	if (test.find("secretfile.txt") != std::string::npos)
	{
		int t = 1;
	}

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{(LPSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		OutputDebugString("AVCore: Main: StartServiceCtrlDispatcher returned error");
		return GetLastError();
	}

	OutputDebugString("AVCore: Main: Exit");
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	HANDLE hThread;
	DWORD Status = E_FAIL;

	OutputDebugString("AVCore: ServiceMain: Entry");

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		OutputDebugString("AVCore: ServiceMain: RegisterServiceCtrlHandler returned error");
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString("AVCore: ServiceMain: SetServiceStatus returned error");
	}

	/*
	 * Perform tasks neccesary to start the service here
	 */
	OutputDebugString("AVCore: ServiceMain: Performing Service Start Operations");

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		OutputDebugString("AVCore: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString("AVCore: ServiceMain: SetServiceStatus returned error");
		}
		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString("AVCore: ServiceMain: SetServiceStatus returned error");
	}

	// Start the thread that will perform the main task of the service
	hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	OutputDebugString("AVCore: ServiceMain: Waiting for Worker Thread to complete");

	// Wait until our worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);

	OutputDebugString("AVCore: ServiceMain: Worker Thread Stop Event signaled");

	/*
	 * Perform any cleanup tasks
	 */
	OutputDebugString("AVCore: ServiceMain: Performing Cleanup Operations");

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString("AVCore: ServiceMain: SetServiceStatus returned error");
	}

EXIT:
	OutputDebugString("AVCore: ServiceMain: Exit");

	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	OutputDebugString("AVCore: ServiceCtrlHandler: Entry");

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		OutputDebugString("AVCore: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		 * Perform tasks neccesary to stop the service here
		 */
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString("AVCore: ServiceCtrlHandler: SetServiceStatus returned error");
		}
		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}

	OutputDebugString("AVCore: ServiceCtrlHandler: Exit");
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	OutputDebugString("AVCore: ServiceWorkerThread: Entry");
	int i = 0;

	FileLogger* logger = new FileLogger(LOG_PATH);
	PluginManager* manager = new PluginManager(logger);
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

	CommPortServer portServer;
	portServer.start(manager);
	

	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		Sleep(5000);
	}

	portServer.stop();
	delete manager;

	OutputDebugString("AVCore: ServiceWorkerThread: Exit");

	return ERROR_SUCCESS;
}