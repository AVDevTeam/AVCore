#include "AVEventsDriver.h"

NTSTATUS AVEventsRegistryCallback(
	PVOID CallbackContext,
	PVOID Argument1,
	PVOID Argument2
)
{
	UNREFERENCED_PARAMETER(CallbackContext);
	UNREFERENCED_PARAMETER(Argument1);
	UNREFERENCED_PARAMETER(Argument2);
	return STATUS_SUCCESS;
}