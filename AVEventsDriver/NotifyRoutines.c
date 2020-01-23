/**
\file
\brief Implements notify routines callbacks.
*/

#include "AVEventsDriver.h"

/**
\brief Implements process create/exit callbacks.

Prepares AV_EVENT_PROCESS_CREATE and AV_EVENT_PROCESS_EXIT event buffers and
passes them to the UM component.

\param[in] Process Pointer to the KM structure for the new or exiting process.

\param[in] ProcessId ID of the process that caused the event.

\param[in] CreateInfo Pointer to the structure with information about new process
(NULL for exiting process).

\return None.
*/
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
#ifdef PROCESS_CREATE_EVENTS
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
#else
		UNREFERENCED_PARAMETER(ProcessId);
#endif
	}
	else
	{
#ifdef PROCESS_EXIT_EVENTS
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
#else
	}
		UNREFERENCED_PARAMETER(ProcessId);
#endif
}

/**
\brief Thread create/exit callback.

Prepares AV_EVENT_THREAD_CREATE, AV_EVENT_THREAD_EXIT event buffers and passes them
to UM components.

\param[in] ProcessId ID of the process that caused the event.

\param[in] ThreadId ID of the new or exiting thread.

\param[in] Create Indicates whether the thread starts or stops.

\return None.
*/
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
#ifdef THREAD_CREATE_EVENTS
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
			
		}
#else
		UNREFERENCED_PARAMETER(ThreadId);
		UNREFERENCED_PARAMETER(ProcessId);
#endif
	}
	else
	{
#ifdef THREAD_EXIT_EVENTS
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
#else
		UNREFERENCED_PARAMETER(ThreadId);
		UNREFERENCED_PARAMETER(ProcessId);
#endif
	}

}

/**
\brief Image load notification callback.

Prepares AV_EVENT_IMAGE_LOAD event buffer and passes it to the UM componetns.

\param[in] FullImageName Path to the image.

\param[in] ProcessId ID of the process that loads the image.

\param[in] ImageInfo Pointer to the structure with additional information
about the image.

\return None.
*/
void AVLoadImageCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
)
{
#ifdef IMAGE_EVENTS
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
	}
#else
	UNREFERENCED_PARAMETER(FullImageName);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ImageInfo);
#endif
}