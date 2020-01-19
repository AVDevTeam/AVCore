/**
\file
\brief Implements registry filter callbacks.
*/

#include "AVEventsDriver.h"

#pragma region prototypes
// Prototypes
NTSTATUS RegNtPreCreateKeyCallback(PREG_PRE_CREATE_KEY_INFORMATION PreCreateKeyInfo);
NTSTATUS RegNtPreCreateKeyExCallback(PREG_CREATE_KEY_INFORMATION PreCreateKeyInfo);

NTSTATUS RegNtPreOpenKeyCallback(PREG_PRE_OPEN_KEY_INFORMATION PreOpenKeyInfo);
NTSTATUS RegNtPreOpenKeyExCallback(PREG_OPEN_KEY_INFORMATION PreOpenKeyInfo);

/**
\brief Entry point for all registy event callbacks.

\param[in] CallbackContext Custom callback context (not used).

\param[in] RegNotifyClass Registry event class.

\param[in] RegNotifyInfo Pointer to structure that holds information about current
registry event.

\return Status of registry operation.
*/
NTSTATUS AVEventsRegistryCallback(
	PVOID CallbackContext,
	PVOID RegNotifyClass,
	PVOID RegNotifyInfo
)
{
	UNREFERENCED_PARAMETER(CallbackContext);

#ifdef REGISTRY_EVENTS
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
#else
	UNREFERENCED_PARAMETER(RegNotifyClass);
	UNREFERENCED_PARAMETER(RegNotifyInfo);
	return STATUS_SUCCESS;
#endif
}

/**
\brief Implements create key event handling logic.

Prepares AV_EVENT_REG_CREATE_KEY event buffer and sends it to UM components.

\param[in] keyPath Unicode string with the path to the target key.
*/
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

/**
\brief Implements open key event handling logic.

Prepares AV_EVENT_REG_OPEN_KEY event buffer and sends it to UM components.

\param[in] keyPath Unicode string with the path to the target key.
*/
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

/**
\brief Pre key create event handler.

Uses sendRegCreateKey to generate event and send it to UM.

\param[in] PreCreateKeyInfo Event information.
*/
NTSTATUS RegNtPreCreateKeyCallback(PREG_PRE_CREATE_KEY_INFORMATION PreCreateKeyInfo)
{
	return sendRegCreateKey(PreCreateKeyInfo->CompleteName);
}

/**
\brief Pre key create event handler (ex).

Uses sendRegCreateKey to generate event and send it to UM.

\param[in] PreCreateKeyInfo Event information.
*/
NTSTATUS RegNtPreCreateKeyExCallback(PREG_CREATE_KEY_INFORMATION PreCreateKeyInfo)
{
	return sendRegCreateKey(PreCreateKeyInfo->CompleteName);
}

/**
\brief Pre key open event handler.

Uses sendRegOpenKey to generate event and send it to UM.

\param[in] PreOpenKeyInfo Event information.
*/
NTSTATUS RegNtPreOpenKeyCallback(PREG_PRE_OPEN_KEY_INFORMATION PreOpenKeyInfo)
{
	return sendRegOpenKey(PreOpenKeyInfo->CompleteName);
}

/**
\brief Pre key open event handler (ex).

Uses sendRegOpenKey to generate event and send it to UM.

\param[in] PreOpenKeyInfo Event information.
*/
NTSTATUS RegNtPreOpenKeyExCallback(PREG_OPEN_KEY_INFORMATION PreOpenKeyInfo)
{
	return sendRegOpenKey(PreOpenKeyInfo->CompleteName);
}