#include <Windows.h>
#include "detours.h"

#include <string>
#include "EventsKMStructures.h"
#include "KMUMcomm.h"

#include "json.hpp"
using json = nlohmann::json;

// stubs
HANDLE WINAPI CreateFileWstub(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

static class InjectAgent
{
public:
	static void hook();
	static AV_EVENT_RETURN_STATUS sendUMEvent(std::string jsonEvent);

private:
	std::string pipeName;
};