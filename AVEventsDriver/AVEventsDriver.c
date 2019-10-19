/*++
Module Name:
    AVEventsDriver.c
Abstract:
    This is the main module of the AVEventsDriver miniFilter driver.
Environment:
    Kernel mode
--*/

#include "AVEventsDriver.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PFLT_FILTER gFilterHandle;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

// Prototypes
#pragma region Prototypes

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;

NTSTATUS DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS AVEventsInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID AVEventsInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID AVEventsInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS DriverUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS AVEventsInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS AVEventsPreMjCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

EXTERN_C_END
#pragma endregion Prototypes

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, AVEventsInstanceQueryTeardown)
#pragma alloc_text(PAGE, AVEventsInstanceSetup)
#pragma alloc_text(PAGE, AVEventsInstanceTeardownStart)
#endif

// Globals
PFLT_FILTER GlobalFilter;

//  operation registration
#pragma region operation registration
CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      AVEventsPreMjCreate,
      NULL },

    { IRP_MJ_OPERATION_END }
};
#pragma endregion operation registration

//  This defines what we want to filter with FltMgr
CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    DriverUnload,                           //  MiniFilterUnload

    AVEventsInstanceSetup,                    //  InstanceSetup
    AVEventsInstanceQueryTeardown,            //  InstanceQueryTeardown
    AVEventsInstanceTeardownStart,            //  InstanceTeardownStart
    AVEventsInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

NTSTATUS AVEventsInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
/*++
Routine Description:
    This routine is called whenever a new instance is created on a volume. This
    gives us a chance to decide if we need to attach to this volume or not.

    If this routine is not defined in the registration structure, automatic
    instances are always created.
Arguments:
    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.
    Flags - Flags describing the reason for this attach request.
Return Value:
    STATUS_SUCCESS - attach
    STATUS_FLT_DO_NOT_ATTACH - do not attach
--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );
    PAGED_CODE();
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("AVEventsDriver!AVEventsDriverInstanceSetup: Entered\n") );
    return STATUS_SUCCESS;
}

NTSTATUS AVEventsInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
/*++
Routine Description:
    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach thereby giving us a
    chance to fail that detach request.

    If this routine is not defined in the registration structure, explicit
    detach requests via FltDetachVolume or FilterDetach will always be
    failed.
Arguments:
    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.
    Flags - Indicating where this detach request came from.
Return Value:
    Returns the status of this operation.
--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("AVEventsDriver!AVEventsDriverInstanceQueryTeardown: Entered\n") );
    return STATUS_SUCCESS;
}

VOID AVEventsInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++
Routine Description:
    This routine is called at the start of instance teardown.
Arguments:
    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.
    Flags - Reason why this instance is being deleted.
Return Value:
    None.
--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("AVEventsDriver!AVEventsDriverInstanceTeardownStart: Entered\n") );
}

VOID AVEventsInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++
Routine Description:
    This routine is called at the end of instance teardown.
Arguments:
    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.
    Flags - Reason why this instance is being deleted.
Return Value:
    None.
--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("AVEventsDriver!AVEventsDriverInstanceTeardownComplete: Entered\n") );
}


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

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
	UNREFERENCED_PARAMETER(RegistryPath);

	//  Register with FltMgr to tell it our callback routines
	NTSTATUS status = FltRegisterFilter(DriverObject,
		&FilterRegistration,
		&GlobalFilter);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = AVCommInit(GlobalFilter);

	//  Start filtering i/o
	status = FltStartFiltering(GlobalFilter);
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		return status;
	}

	return status;
}

NTSTATUS DriverUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
	/*++
	Routine Description:
		This is the unload routine for this miniFilter driver. This is called
		when the minifilter is about to be unloaded. We can fail this unload
		request if this is not a mandatory unloaded indicated by the Flags
		parameter.
	Arguments:
		Flags - Indicating if this is a mandatory unload.
	Return Value:
		Returns the final status of this operation.
	--*/
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(Flags);

	//  This function will wait for the user to abort the outstanding scan and 
	//  close the section 
	AVCommStop();
	FltUnregisterFilter(GlobalFilter);
	

	return STATUS_SUCCESS;
}

/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/

FLT_PREOP_CALLBACK_STATUS AVEventsPreMjCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
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
	status = FltGetVolumeInformation(FltObjects->Volume, FilterVolumeBasicInformation, &volumeInformationBuffer, (ULONG)volumeInformationSize, (PULONG)&volumeInformationSize);
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
	status = AVCommSendEvent(&eventFileCreate,
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