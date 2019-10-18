/*++
Module Name:
    AVComm.c
Abstract:
    Exports Driver that implements KM-UM communication interface
	in KM. Communication interfaces are provided via exports.
Environment:
    kernel mode only
--*/

#define CLASS_INIT_GUID 1
#define DEBUG_MAIN_SOURCE 1

#include "AVComm.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)
	#pragma alloc_text(PAGE, DllInitialize)
	#pragma alloc_text(PAGE, DllUnload)
    #pragma alloc_text(PAGE, Summ)
#endif

#pragma prefast(disable:28159, "There are certain cases when we have to bugcheck...")

/*
Routine Description:
	This function is called when DLL is loaded.
*/
NTSTATUS DllInitialize(
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrint("AVCommDriver.sys is now loading\n");
	return STATUS_SUCCESS;
}

/*
Routine Description:
	This function is called when DLL is being unloaded.
*/
NTSTATUS DllUnload(VOID)
{
    DbgPrint("AVCommDriver.sys is now unloading\n");
    return STATUS_SUCCESS;
}

/*++
DriverEntry()
Routine Description:
    Temporary entry point needed to initialize the class system dll.
    It doesn't do anything.
Arguments:
    DriverObject - Pointer to the driver object created by the system.
Return Value:
   STATUS_SUCCESS
--*/
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    return STATUS_SUCCESS;
}

/*
Routine Description:
	Test function that is exported from this module.
*/
ULONG Summ(
    _In_  ULONG            Argument1,
    _In_  ULONG            Argument2
    )
{
    return Argument1 + Argument2;
}