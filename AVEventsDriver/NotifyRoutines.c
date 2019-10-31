#include "AVEventsDriver.h"

typedef enum
{
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment

} KAPC_ENVIRONMENT;

NTKERNELAPI
VOID
KeInitializeApc(
	PRKAPC Apc,
	PRKTHREAD Thread,
	KAPC_ENVIRONMENT Environment,
	PVOID KernelRoutine,
	PVOID RundownRoutine,
	PVOID NormalRoutine,
	KPROCESSOR_MODE ApcMode,
	PVOID NormalContext
);

NTKERNELAPI
BOOLEAN
KeInsertQueueApc(
	PKAPC Apc,
	PVOID SystemArgument1,
	PVOID SystemArgument2,
	KPRIORITY Increment
);

DECLSPEC_IMPORT NTSTATUS ZwQueryInformationProcess(
	HANDLE           ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID            ProcessInformation,
	ULONG            ProcessInformationLength,
	PULONG           ReturnLength
);

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

VOID KernelAPC(
	struct _KAPC* Apc,
	PVOID* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2)
{
	UNREFERENCED_PARAMETER(Apc);
	UNREFERENCED_PARAMETER(NormalRoutine);
	UNREFERENCED_PARAMETER(NormalContext);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);
	DbgPrint("TEST\n");
}

VOID DebugFunc(NTSTATUS test)
{
	DbgPrint("status: %d\n", test);
}

void APCInject(PAPC_INFO ApcInfo)
{
	HANDLE pHandle = NULL;
	PEPROCESS pEprocess = NULL;
	PKTHREAD pThread = NULL;
	UCHAR APCPayload[1024] = { 0 };
	
	NTSTATUS apcStatus = PsLookupProcessByProcessId((HANDLE)ApcInfo->PID, &pEprocess);
	if (apcStatus == STATUS_SUCCESS)
	{
		OBJECT_ATTRIBUTES objectAttributes;
		InitializeObjectAttributes(&objectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
		CLIENT_ID client_id;
		client_id.UniqueProcess = (HANDLE)ApcInfo->PID;
		client_id.UniqueThread = 0;
		apcStatus = ZwOpenProcess(&pHandle, PROCESS_ALL_ACCESS, &objectAttributes, &client_id);
		if (apcStatus == STATUS_SUCCESS)
		{

			apcStatus = PsLookupThreadByThreadId((HANDLE)ApcInfo->TID, &pThread);
			if (apcStatus == STATUS_SUCCESS)
			{
				PVOID umAPCBuffer = NULL;
				SIZE_T umAPCBufferSize = ApcInfo->apcBufferSize;
				apcStatus = ZwAllocateVirtualMemory(pHandle, &umAPCBuffer, 0, &umAPCBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				DebugFunc(apcStatus);
				if (apcStatus == STATUS_SUCCESS)
				{
					// copy APC payload from UM to KM
					if (ApcInfo->apcBufferSize > 1024)
						ASSERT(FALSE);
					AVCommGetUmBuffer(ApcInfo->apcBuffer, APCPayload, ApcInfo->apcBufferSize);

					KAPC_STATE pkapcState;
					KeStackAttachProcess(pEprocess, &pkapcState);
					// copy buffer from KM to allocated buffer in UM.
					memcpy(umAPCBuffer, APCPayload, ApcInfo->apcBufferSize);
					// Restore stack
					KeUnstackDetachProcess(&pkapcState);

					PKAPC apc = (PKAPC)ExAllocatePool(NonPagedPool, sizeof(KAPC));
					KeInitializeApc(apc, pThread, OriginalApcEnvironment, (PVOID)KernelAPC, NULL, (PVOID)umAPCBuffer, UserMode, NULL);
					KeInsertQueueApc(apc, 0, NULL, 0);
				}
			}

			ZwClose(pHandle);
		}
	}
}

void AVCreateThreadCallback(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
)
{
	if (!AVCommIsInitialized() || AVCommIsExcludedPID(ProcessId))
	{
		return;
	}

	if (Create)
	{
		UCHAR buffer[128];
		RtlFillMemory(&buffer, sizeof(buffer), 0x90);
		PEPROCESS pHandle = NULL;
		PKTHREAD pThread = NULL;
		NTSTATUS apcStatus = PsLookupProcessByProcessId(ProcessId, &pHandle);
		if (apcStatus == STATUS_SUCCESS)
		{
			apcStatus = PsLookupThreadByThreadId(ThreadId, &pThread);
			if (apcStatus == STATUS_SUCCESS)
			{
				PVOID umAPCBuffer = NULL;
				SIZE_T umAPCBufferSize = sizeof(buffer);
				apcStatus = ZwAllocateVirtualMemory(pHandle, &umAPCBuffer, 0, &umAPCBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (apcStatus == STATUS_SUCCESS)
				{
					KAPC_STATE pkapcState;
					KeStackAttachProcess(pHandle, &pkapcState);
					// copy buffer from KM to allocated buffer in UM.
					memcpy(umAPCBuffer, buffer, sizeof(buffer));
					// Restore stack
					KeUnstackDetachProcess(&pkapcState);

					KAPC apc;
					KeInitializeApc(&apc, pThread, 0, (PVOID)&KernelAPC, NULL, (PVOID)umAPCBuffer, UserMode, NULL);
					KeInsertQueueApc(&apc, 0, NULL, 0);
				}
			}
		}

		AV_EVENT_THREAD_CREATE eventThreadCreate = { 0 };
		eventThreadCreate.PID = (int)(__int64)ProcessId;
		eventThreadCreate.TID = (int)(__int64)ThreadId;

		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(AvThreadCreate,
			&eventThreadCreate,
			sizeof(AV_EVENT_THREAD_CREATE),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			// TODO.Response processing login?
		}
	}
	else
	{
		AV_EVENT_THREAD_EXIT eventThreadExit = { 0 };
		eventThreadExit.PID = (int)(__int64)ProcessId;
		eventThreadExit.TID = (int)(__int64)ThreadId;

		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(AvThreadExit,
			&eventThreadExit,
			sizeof(AV_EVENT_THREAD_EXIT),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			// TODO.Response processing login?
		}
	}
}

// TODO. IFDEF x86/x64 (x64 support for x86 modules).
void AVLoadImageCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
	if (!AVCommIsInitialized() || AVCommIsExcludedPID(ProcessId))
	{
		return;
	}

	AV_EVENT_IMAGE_LOAD eventImageLoad = { 0 };
	eventImageLoad.PID = (int)(__int64)ProcessId;
	eventImageLoad.systemModeImage = (UCHAR)ImageInfo->SystemModeImage;

	SIZE_T umBuffImageNameSize;
	NTSTATUS status = STATUS_SUCCESS;
	if (FullImageName)
	{
		// DEBUG!!! MIGHT DEADLOCK IN WINDOWS 7 ACCORDING TO MS DOCS [https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/nc-ntddk-pload_image_notify_routine, remarks]
		eventImageLoad.imageNameSize = FullImageName->Length;
		status = AVCommCreateBuffer(FullImageName->Buffer, eventImageLoad.imageNameSize, &eventImageLoad.imageName, &umBuffImageNameSize);
		if (status != STATUS_SUCCESS)
		{
			// couldn't allocate memory in UM
			return;
		}
	}

	AV_EVENT_RESPONSE UMResponse;
	ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

	// DEBUG!!! MIGHT DEADLOCK IN WINDOWS 7 ACCORDING TO MS DOCS [https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/nc-ntddk-pload_image_notify_routine, remarks]
	status = AVCommSendEvent(AvImageLoad,
		&eventImageLoad,
		sizeof(AV_EVENT_IMAGE_LOAD),
		&UMResponse,
		&replyLength);

	if (FullImageName)
	{
		NTSTATUS freeStatus = AVCommFreeBuffer(&eventImageLoad.imageName, &umBuffImageNameSize);
		if (freeStatus != STATUS_SUCCESS) { return; }
	}

	if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
	{
		// TODO.Response processing login?
		if (UMResponse.Status == AvEventStatusInjectAPC)
		{
			PAPC_INFO apcInfoInUM = UMResponse.UMMessage;
			APC_INFO apcInfoInKM;
			AVCommGetUmBuffer(apcInfoInUM, &apcInfoInKM, sizeof(APC_INFO));
			APCInject(&apcInfoInKM);
		}
	}

	

}