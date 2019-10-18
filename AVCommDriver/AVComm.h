/*++
Module Name:
    AVComm.h
Abstract:
    Private header file for classpnp.sys modules.  This contains private
    structure and function declarations as well as constant values which do
    not need to be exported.
Environment:
    kernel mode only
--*/

#include <ntddk.h>

// Prototypes
#pragma region Prototypes

NTSTATUS DllInitialize(
	_In_ PUNICODE_STRING RegistryPath
);

NTSTATUS DllUnload(VOID);

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
);

ULONG
Summ(
	_In_  ULONG            Argument1,
	_In_  ULONG            Argument2
);

#pragma endregion Prototypes
