#include "AVEventsDriver.h"

#pragma region prototypes
// Prototypes
NTSTATUS RegNtPreCreateKeyCallback(PREG_PRE_CREATE_KEY_INFORMATION PreCreateKeyInfo);
NTSTATUS RegNtPreCreateKeyExCallback(PREG_CREATE_KEY_INFORMATION PreCreateKeyInfo);

NTSTATUS RegNtPreOpenKeyCallback(PREG_PRE_OPEN_KEY_INFORMATION PreOpenKeyInfo);
NTSTATUS RegNtPreOpenKeyExCallback(PREG_OPEN_KEY_INFORMATION PreOpenKeyInfo);

NTSTATUS AVEventsRegistryCallback(
	PVOID CallbackContext,
	PVOID RegNotifyClass,
	PVOID RegNotifyInfo
)
{
	UNREFERENCED_PARAMETER(CallbackContext);

	// debug
	UNREFERENCED_PARAMETER(RegNotifyClass);
	UNREFERENCED_PARAMETER(RegNotifyInfo);
	return STATUS_SUCCESS;

	/*

	if (!AVCommIsInitialized() || AVCommIsExcludedPID(PsGetCurrentProcessId()))
	{
		return STATUS_SUCCESS;
	}

	switch ((REG_NOTIFY_CLASS)(ULONG_PTR)RegNotifyClass)
	{
	case RegNtPreCreateKey:
		return RegNtPreCreateKeyCallback(RegNotifyInfo);
	case RegNtPreCreateKeyEx:
		return RegNtPreCreateKeyExCallback(RegNotifyInfo);
	case RegNtPreOpenKey:
		return RegNtPreOpenKeyCallback(RegNotifyInfo);
	case RegNtPreOpenKeyEx:
		return RegNtPreOpenKeyExCallback(RegNotifyInfo);
	default:
		return STATUS_SUCCESS;
	}

	*/
}

NTSTATUS sendRegCreateKey(PUNICODE_STRING keyPath)
{
	ASSERT(keyPath != NULL);

	AV_EVENT_REG_CREATE_KEY eventCreateKey = { 0 };
	SIZE_T umBuffKeyPathSize;
	NTSTATUS status = STATUS_SUCCESS;

	eventCreateKey.requestorPID = (int)(__int64)PsGetCurrentProcessId();
	eventCreateKey.keyPathSize = keyPath->Length;
	status = AVCommCreateBuffer(keyPath->Buffer, eventCreateKey.keyPathSize, &eventCreateKey.keyPath, &umBuffKeyPathSize);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return STATUS_SUCCESS;
	}

	AV_EVENT_RESPONSE UMResponse;
	ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

	// Send event to the AVCore UM service and wait for the response
	status = AVCommSendEvent(AvRegCreateKey,
		&eventCreateKey,
		sizeof(AV_EVENT_REG_CREATE_KEY),
		&UMResponse,
		&replyLength);

	NTSTATUS freeStatus = AVCommFreeBuffer(&eventCreateKey.keyPath, &umBuffKeyPathSize);
	if (freeStatus != STATUS_SUCCESS) { return STATUS_SUCCESS; }

	if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
	{
		if (UMResponse.Status == AvEventStatusBlock)
		{
			return STATUS_ACCESS_DENIED;
		}
	}
	return STATUS_SUCCESS;
}

NTSTATUS sendRegOpenKey(PUNICODE_STRING keyPath)
{
	ASSERT(keyPath != NULL);

	AV_EVENT_REG_OPEN_KEY eventOpenKey = { 0 };
	SIZE_T umBuffKeyPathSize;
	NTSTATUS status = STATUS_SUCCESS;

	eventOpenKey.requestorPID = (int)(__int64)PsGetCurrentProcessId();
	eventOpenKey.keyPathSize = keyPath->Length;
	status = AVCommCreateBuffer(keyPath->Buffer, eventOpenKey.keyPathSize, &eventOpenKey.keyPath, &umBuffKeyPathSize);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return STATUS_SUCCESS;
	}

	AV_EVENT_RESPONSE UMResponse;
	ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

	// Send event to the AVCore UM service and wait for the response
	status = AVCommSendEvent(AvRegOpenKey,
		&eventOpenKey,
		sizeof(AV_EVENT_REG_OPEN_KEY),
		&UMResponse,
		&replyLength);

	NTSTATUS freeStatus = AVCommFreeBuffer(&eventOpenKey.keyPath, &umBuffKeyPathSize);
	if (freeStatus != STATUS_SUCCESS) { return STATUS_SUCCESS; }

	if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
	{
		if (UMResponse.Status == AvEventStatusBlock)
		{
			return STATUS_ACCESS_DENIED;
		}
	}
	return STATUS_SUCCESS;
}

NTSTATUS RegNtPreCreateKeyCallback(PREG_PRE_CREATE_KEY_INFORMATION PreCreateKeyInfo)
{
	return sendRegCreateKey(PreCreateKeyInfo->CompleteName);
}
NTSTATUS RegNtPreCreateKeyExCallback(PREG_CREATE_KEY_INFORMATION PreCreateKeyInfo)
{
	return sendRegCreateKey(PreCreateKeyInfo->CompleteName);
}

NTSTATUS RegNtPreOpenKeyCallback(PREG_PRE_OPEN_KEY_INFORMATION PreOpenKeyInfo)
{
	return sendRegOpenKey(PreOpenKeyInfo->CompleteName);
}

NTSTATUS RegNtPreOpenKeyExCallback(PREG_OPEN_KEY_INFORMATION PreOpenKeyInfo)
{
	return sendRegOpenKey(PreOpenKeyInfo->CompleteName);
}