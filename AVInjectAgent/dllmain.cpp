// dllmain.cpp : Defines the entry point for the DLL application.
#include "AVInjectAgent.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		InjectAgent::hook();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		// this dll shouldn't be unloaded... ever
        break;
    }
    return TRUE;
}

