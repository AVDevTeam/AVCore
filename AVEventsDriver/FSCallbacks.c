#include "AVEventsDriver.h"

FLT_PREOP_CALLBACK_STATUS AVEventsPreMjCreate(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
/*++
Routine Description:
	This routine is a pre-operation dispatch routine for this miniFilter.
	This is non-pageable because it could be called on the paging path
Arguments:
	Data - Pointer to the filter callbackData that is passed to us.
	FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
		opaque handles to this filter, instance, its associated volume and
		file object.
	CompletionContext - The context for the completion routine for this
		operation.
Return Value:
	The return value is the status of the operation.
--*/
{
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(FltObjects);

	ULONG_PTR stackLow;
	ULONG_PTR stackHigh;
	PFILE_OBJECT FileObject = Data->Iopb->TargetFileObject;

	PAGED_CODE();

	//  Stack file objects are never scanned.
	IoGetStackLimits(&stackLow, &stackHigh);

	if (((ULONG_PTR)FileObject > stackLow) &&
		((ULONG_PTR)FileObject < stackHigh))
	{
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	HANDLE AVCorePID = AVCommGetUmPID();

	if (!AVCorePID)
	{
		// AVCore service is not listening skip event.
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	HANDLE curProcess = PsGetCurrentProcessId();
	if (curProcess == AVCorePID)
	{
		// ignore events triggered by AVCore.exe
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	UCHAR volumeInformationBuffer[256];
	SIZE_T volumeInformationSize = sizeof(volumeInformationBuffer);

	NTSTATUS status = STATUS_SUCCESS;
	status = FltGetVolumeInformation(FltObjects->Volume, FilterVolumeBasicInformation, &volumeInformationBuffer, (ULONG)volumeInformationSize, (PULONG)& volumeInformationSize);
	if (status != STATUS_SUCCESS)
	{
		// couldn't get volume information
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	PFILTER_VOLUME_BASIC_INFORMATION volumeInformation = (PFILTER_VOLUME_BASIC_INFORMATION)volumeInformationBuffer;

	// Start forming Event structure on KM stack
	AV_EVENT_FILE_CREATE eventFileCreate = { 0 };

	eventFileCreate.RequestorMode = Data->RequestorMode;
	eventFileCreate.RequestorPID = (int)(__int64)curProcess;

	// Put file name information to UM memory and save address in Event stucture.
	eventFileCreate.FileNameSize = FileObject->FileName.Length;
	SIZE_T umBuffFileNameSize;
	status = AVCommCreateBuffer(FileObject->FileName.Buffer, FileObject->FileName.Length, &eventFileCreate.FileName, &umBuffFileNameSize);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	// Put volume name information to UM memory and save address in Event stucture.
	eventFileCreate.VolumeNameSize = volumeInformation->FilterVolumeNameLength;
	SIZE_T umBuffVolumeNameSize;
	status = AVCommCreateBuffer(volumeInformation->FilterVolumeName, eventFileCreate.VolumeNameSize, &eventFileCreate.VolumeName, &umBuffVolumeNameSize);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	AV_EVENT_RESPONSE UMResponse;
	ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

	DbgPrint("PID %ul opens %wZ\n", curProcess, FileObject->FileName);

	// Send event to the AVCore UM service and wait for the response
	status = AVCommSendEvent(AvFileCreate,
		&eventFileCreate,
		sizeof(AV_EVENT_FILE_CREATE),
		&UMResponse,
		&replyLength);

	// Got reply. Free memory. We need to free all UM-allocated buffers.
#pragma region free UM memory

	NTSTATUS freeStatus = AVCommFreeBuffer(&eventFileCreate.FileName, &umBuffFileNameSize);
	if (freeStatus != STATUS_SUCCESS) { return FLT_PREOP_SUCCESS_NO_CALLBACK; }

	freeStatus = AVCommFreeBuffer(&eventFileCreate.VolumeName, &umBuffVolumeNameSize);
	if (freeStatus != STATUS_SUCCESS) { return FLT_PREOP_SUCCESS_NO_CALLBACK; }
#pragma endregion free UM memory

	if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
	{
		if (UMResponse.Status == AvEventStatusBlock)
		{
			Data->IoStatus.Status = STATUS_ACCESS_DENIED;
			return FLT_PREOP_COMPLETE;
		}
	}
	// allow access if we were not able to communicate with UM.
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}