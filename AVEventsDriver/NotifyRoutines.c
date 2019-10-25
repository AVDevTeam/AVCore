#include "AVEventsDriver.h"

// TODO. IFDEF x86/x64 (-/-Ex2 support)
void AVCreateProcessCallback(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	UNREFERENCED_PARAMETER(Process);

	if (!AVCommIsInitialized())
	{
		return;
	}

	if (CreateInfo)
	{ // Process create notification
		if (AVCommIsExcludedPID(CreateInfo->CreatingThreadId.UniqueProcess))
		{
			return;
		}

		AV_EVENT_PROCESS_CREATE eventProcessCreate = { 0 };
		eventProcessCreate.PID = (int)(__int64)ProcessId;
		eventProcessCreate.parentPID = (int)(__int64)CreateInfo->ParentProcessId;
		eventProcessCreate.creatingPID = (int)(__int64)CreateInfo->CreatingThreadId.UniqueProcess;
		eventProcessCreate.creatingTID = (int)(__int64)CreateInfo->CreatingThreadId.UniqueThread;

		SIZE_T umBuffCommandLineSize, umBuffFileNameSize;
		NTSTATUS status = STATUS_SUCCESS;
		if (CreateInfo->FileOpenNameAvailable && CreateInfo->ImageFileName)
		{
			// Put file name info to UM buffer
			eventProcessCreate.imageFileNameSize = CreateInfo->ImageFileName->Length;
			status = AVCommCreateBuffer(CreateInfo->ImageFileName->Buffer, eventProcessCreate.imageFileNameSize, &eventProcessCreate.imageFileName, &umBuffFileNameSize);
			if (status != STATUS_SUCCESS)
			{
				// couldn't allocate memory in UM
				return;
			}

			// Put command line info to UM buffer
			eventProcessCreate.commandLineSize = CreateInfo->CommandLine->Length;
			status = AVCommCreateBuffer(CreateInfo->CommandLine->Buffer, eventProcessCreate.commandLineSize, &eventProcessCreate.commandLine, &umBuffCommandLineSize);
			if (status != STATUS_SUCCESS)
			{
				// couldn't allocate memory in UM
				return;
			}
		}

		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		// Send event to the AVCore UM service and wait for the response
		status = AVCommSendEvent(AvProcessCreate,
			&eventProcessCreate,
			sizeof(AV_EVENT_PROCESS_CREATE),
			&UMResponse,
			&replyLength);

		if (CreateInfo->FileOpenNameAvailable && CreateInfo->ImageFileName)
		{
			// free memory in UM
			NTSTATUS freeStatus = AVCommFreeBuffer(&eventProcessCreate.imageFileName, &umBuffFileNameSize);
			if (freeStatus != STATUS_SUCCESS) { return; }
			freeStatus = AVCommFreeBuffer(&eventProcessCreate.commandLine, &umBuffCommandLineSize);
			if (freeStatus != STATUS_SUCCESS) { return; }
		}

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			if (UMResponse.Status == AvEventStatusBlock)
			{
				CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
			}
		}
	}
	else
	{
		AV_EVENT_PROCESS_EXIT eventProcessExit = { 0 };
		eventProcessExit.PID = (int)(__int64)ProcessId;

		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(AvProcessExit,
			&eventProcessExit,
			sizeof(AV_EVENT_PROCESS_EXIT),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			// TODO. Maybe some exlude list logic.
		}
	}
}

void AVCreateThreadCallback(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
)
{
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ThreadId);
	UNREFERENCED_PARAMETER(Create);
}

// TODO. IFDEF x86/x64 (x64 support for x86 modules).
void AVLoadImageCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	UNREFERENCED_PARAMETER(FullImageName);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ImageInfo);
}