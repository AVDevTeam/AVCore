#include "AVRegistryDriver.h"

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
);

EXTERN_C_END






NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
/*++
Routine Description:
	This is the initialization routine for this miniFilter driver.  This
	registers with FltMgr and initializes all global data structures.
Arguments:
	DriverObject - Pointer to driver object created by the system to
		represent this driver.
	RegistryPath - Unicode string identifying where the parameters for this
		driver are located in the registry.
Return Value:
	Returns the final status of this operation.
--*/
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	if (!AVCommGetUmPID())
	{
		// KM-UM communication wasn't sent up on KM side.
		// AVEventsDriver initializes KM-UM comm port.
		// It should be started first.
		return STATUS_NOT_FOUND;
	}


	return STATUS_SUCCESS;
}