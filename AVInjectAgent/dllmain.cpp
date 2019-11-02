// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <detours.h>


// Target pointer for the uninstrumented Sleep API.
//
static VOID(WINAPI* TrueSleep)(DWORD dwMilliseconds) = Sleep;

static LONG dwSlept = 0;

// Detour function that replaces the Sleep API.
//
extern "C" __declspec (dllexport) VOID WINAPI TimedSleep(DWORD dwMilliseconds)
{
	// Save the before and after times around calling the Sleep API.
	DWORD dwBeg = GetTickCount();
	TrueSleep(dwMilliseconds);
	DWORD dwEnd = GetTickCount();

	InterlockedExchangeAdd(&dwSlept, dwEnd - dwBeg);

	int msgboxID = MessageBox(
		NULL,
		(LPCWSTR)L"Sleep was called",
		(LPCWSTR)L"TEST",
		MB_OK
	);

}

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
		DetourRestoreAfterWith();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueSleep, TimedSleep);
		DetourTransactionCommit();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		// this dll shouldn't be unloaded... ever
        break;
    }
    return TRUE;
}

