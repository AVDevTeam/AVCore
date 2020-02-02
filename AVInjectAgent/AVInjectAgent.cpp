#include "pch.h"
#include "AVInjectAgent.h"
#include <sstream>

// original functions
HANDLE(WINAPI* CreateFileWorig)(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	) = CreateFileW;

// functions that will receive execution flow via hooking
HANDLE WINAPI CreateFileWstub(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
)
{
	json j;
	j["func"] = "CreateFileW";
	std::wstring lpFileNameWs(lpFileName);
	j["args"] = { std::string(lpFileNameWs.begin(), lpFileNameWs.end()), std::to_string(dwDesiredAccess), std::to_string(dwShareMode) };

	AV_EVENT_RETURN_STATUS status = InjectAgent::sendUMEvent(j.dump());
	if (status == AvEventStatusBlock)
	{
		return INVALID_HANDLE_VALUE;
	}
	return CreateFileWorig(lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

BOOL(WINAPI* CreateProcessWorig) (
	LPCWSTR               lpApplicationName,
	LPWSTR                lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCWSTR               lpCurrentDirectory,
	LPSTARTUPINFOW        lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	) = CreateProcessW;

BOOL CreateProcessWstub(
	LPCWSTR               lpApplicationName,
	LPWSTR                lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCWSTR               lpCurrentDirectory,
	LPSTARTUPINFOW        lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
)
{
	json j;
	j["func"] = "CreateProcessW";
	std::wstring wApplicationName(lpApplicationName);
	std::wstring wCommandLine(lpCommandLine);
	j["args"] = { std::string(wApplicationName.begin(), wApplicationName.end()), std::string(wCommandLine.begin(), wCommandLine.end()) };

	AV_EVENT_RETURN_STATUS status = InjectAgent::sendUMEvent(j.dump());
	if (status == AvEventStatusBlock)
	{
		return FALSE;
	}
	return CreateProcessWorig(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);
}

void InjectAgent::hook()
{
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)CreateFileWorig, CreateFileWstub);
	DetourAttach(&(PVOID&)CreateProcessWorig, CreateProcessWstub);
	DetourTransactionCommit();
}

AV_EVENT_RETURN_STATUS InjectAgent::sendUMEvent(std::string jsonEvent)
{
	HANDLE hPipe;
	CHAR  chBuf[1024];
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbToWrite, cbWritten, dwMode;
	CONST CHAR* lpszPipename = AV_UM_EVENTS_PIPE_NAME;

	// Try to open a named pipe; wait for it, if necessary. 
	while (1)
	{
		hPipe = CreateFileA(
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			return AvEventStatusAllow;
		}

		// All pipe instances are busy, so wait for 20 seconds. 
		if (!WaitNamedPipeA(lpszPipename, 20000))
		{
			return AvEventStatusAllow;
		}
	}

	// The pipe connected; change to message-read mode. 
	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess)
	{
		return AvEventStatusAllow;
	}

	// Send a message to the pipe server. 
	cbToWrite = (strlen(jsonEvent.c_str()) + 1) * sizeof(CHAR);
	fSuccess = WriteFile(
		hPipe,                  // pipe handle 
		jsonEvent.c_str(),      // message 
		cbToWrite,              // message length 
		&cbWritten,             // bytes written 
		NULL);                  // not overlapped 

	if (!fSuccess)
	{
		return AvEventStatusAllow;
	}

	do
	{
		// Read from the pipe. 
		fSuccess = ReadFile(
			hPipe,    // pipe handle 
			chBuf,    // buffer to receive reply 
			1024*sizeof(CHAR),  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 

		if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
			break;
	} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

	if (!fSuccess)
	{
		return AvEventStatusAllow;
	}

	CloseHandle(hPipe);
	std::stringstream result;
	result << std::string(chBuf);

	int return_status;
	result >> return_status;
	return (AV_EVENT_RETURN_STATUS)return_status;
}
