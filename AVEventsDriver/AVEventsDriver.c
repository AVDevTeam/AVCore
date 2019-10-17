/*++
Module Name:
    AVEventsDriver.c
Abstract:
    This is the main module of the AVEventsDriver miniFilter driver.
Environment:
    Kernel mode
--*/

#include "Globals.h"
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

NTSTATUS AVEventsDriverInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID AVEventsDriverInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID AVEventsDriverInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS AVEventsDriverUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS AVEventsDriverInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS AVEventsDriverPreMjCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

NTSTATUS AVEventsDriverConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
);

VOID AVEventsDriverDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS AVEventsDriverPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AV_CONNECTION_TYPE  ConnectionType
);

NTSTATUS AVEventsDriverSendUnloadingToUser(
	VOID
);

NTSTATUS memmoveUM(void* srcBuffer, PSIZE_T size, void* outUmBuffer);

EXTERN_C_END
#pragma endregion Prototypes

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, AVEventsDriverUnload)
#pragma alloc_text(PAGE, AVEventsDriverInstanceQueryTeardown)
#pragma alloc_text(PAGE, AVEventsDriverInstanceSetup)
#pragma alloc_text(PAGE, AVEventsDriverInstanceTeardownStart)
#pragma alloc_text(PAGE, AVEventsDriverConnectNotifyCallback)
#pragma alloc_text(PAGE, AVEventsDriverDisconnectNotifyCallback)
#pragma alloc_text(PAGE, AVEventsDriverPrepareServerPort)
#pragma alloc_text(PAGE, AVEventsDriverSendUnloadingToUser)
#pragma alloc_text(PAGE, memmoveUM)
#endif

//  operation registration
#pragma region operation registration
CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      AVEventsDriverPreMjCreate,
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

    AVEventsDriverUnload,                           //  MiniFilterUnload

    AVEventsDriverInstanceSetup,                    //  InstanceSetup
    AVEventsDriverInstanceQueryTeardown,            //  InstanceQueryTeardown
    AVEventsDriverInstanceTeardownStart,            //  InstanceTeardownStart
    AVEventsDriverInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

AV_SCANNER_GLOBAL_DATA Globals;

NTSTATUS AVEventsDriverInstanceSetup (
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


NTSTATUS AVEventsDriverInstanceQueryTeardown (
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


VOID AVEventsDriverInstanceTeardownStart (
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


VOID AVEventsDriverInstanceTeardownComplete (
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
	NTSTATUS status = STATUS_SUCCESS;
	PSECURITY_DESCRIPTOR sd = NULL;

	UNREFERENCED_PARAMETER(RegistryPath);

	AV_DBG_PRINT(AVDBG_TRACE_ROUTINES,
		("[AV] DriverEntry: Entered\n"));

	//  Set default global configuration
	RtlZeroMemory(&Globals, sizeof(Globals));

#if DBG

	Globals.DebugLevel = 0xffffffff; // AVDBG_TRACE_ERROR | AVDBG_TRACE_DEBUG;

#endif

	Globals.AVCoreServiceHandle = NULL;
	Globals.AVCoreServiceEprocess = NULL;

	try 
	{
		//  Register with FltMgr to tell it our callback routines
		status = FltRegisterFilter(DriverObject,
			&FilterRegistration,
			&Globals.Filter);

		if (!NT_SUCCESS(status)) 
		{
			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: FltRegisterFilter FAILED. status = 0x%x\n", status));
			leave;
		}

		//  Builds a default security descriptor for use with FltCreateCommunicationPort.
		status = FltBuildDefaultSecurityDescriptor(&sd,
			FLT_PORT_ALL_ACCESS);


		if (!NT_SUCCESS(status)) 
		{
			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: FltBuildDefaultSecurityDescriptor FAILED. status = 0x%x\n", status));
			leave;
		}


		//  Prepare ports between kernel and user.
		status = AVEventsDriverPrepareServerPort(sd, AvConnectForScan);
		if (!NT_SUCCESS(status))
		{
			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: AvPrepareServerPort Scan Port FAILED. status = 0x%x\n", status));
			leave;
		}

		status = AVEventsDriverPrepareServerPort(sd, AvConnectForAbort);
		if (!NT_SUCCESS(status)) 
		{
			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: AvPrepareServerPort Abort Port FAILED. status = 0x%x\n", status));
			leave;
		}

		//  Start filtering i/o
		status = FltStartFiltering(Globals.Filter);

		if (!NT_SUCCESS(status)) 
		{
			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: FltStartFiltering FAILED. status = 0x%x\n", status));
			leave;
		}

	}
	finally
	{
		if (sd != NULL) 
		{
			FltFreeSecurityDescriptor(sd);
		}

		if (!NT_SUCCESS(status)) 
		{
			if (NULL != Globals.EventsServerPort) 
			{
				FltCloseCommunicationPort(Globals.EventsServerPort);
			}
			if (NULL != Globals.AbortServerPort)
			{
				FltCloseCommunicationPort(Globals.AbortServerPort);
			}
			if (NULL != Globals.Filter) 
			{
				FltUnregisterFilter(Globals.Filter);
				Globals.Filter = NULL;
			}
		}
	}

	return status;
}

NTSTATUS AVEventsDriverUnload (
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

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[AV] AvUnload: Entered\n"));

	Globals.Unloading = TRUE;

	//  This function will wait for the user to abort the outstanding scan and 
	//  close the section 
	AVEventsDriverSendUnloadingToUser();

	FltCloseCommunicationPort(Globals.EventsServerPort);
	Globals.EventsServerPort = NULL;
	FltCloseCommunicationPort(Globals.AbortServerPort);
	Globals.AbortServerPort = NULL;
	FltUnregisterFilter(Globals.Filter);  // This will typically trigger instance tear down.
	Globals.Filter = NULL;

	return STATUS_SUCCESS;
}


NTSTATUS AVEventsDriverConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
)
/*++
Routine Description
	Communication connection callback routine.
	This is called when user-mode connects to the server port.
Arguments
	ClientPort - This is the client connection port that will be used to send messages from the filter

	ServerPortCookie - Unused

	ConnectionContext - The connection context passed from the user. This is to recognize which type
			connection the user is trying to connect.

	SizeofContext   - The size of the connection context.

	ConnectionCookie - Propagation of the connection context to disconnection callback.
Return Value
	STATUS_SUCCESS - to accept the connection
	STATUS_INSUFFICIENT_RESOURCES - if memory is not enough
	STATUS_INVALID_PARAMETER_3 - Connection context is not valid.
--*/
{
	PAV_CONNECTION_CONTEXT connectionCtx = (PAV_CONNECTION_CONTEXT)ConnectionContext;
	PAV_CONNECTION_TYPE connectionCookie = NULL;

	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(SizeOfContext);

	if (NULL == connectionCtx) {

		return STATUS_INVALID_PARAMETER_3;
	}

	//  ConnectionContext passed in may be deleted. We need to make a copy of it.
	connectionCookie = (PAV_CONNECTION_TYPE)ExAllocatePoolWithTag(PagedPool,
		sizeof(AV_CONNECTION_TYPE),
		AV_CONNECTION_CTX_TAG);
	if (NULL == connectionCookie) {

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	NTSTATUS status = STATUS_SUCCESS;

	*connectionCookie = connectionCtx->Type;
	switch (connectionCtx->Type)
	{
	case AvConnectForScan:
		Globals.ScanClientPort = ClientPort;

		OBJECT_ATTRIBUTES objectAttributes;
		InitializeObjectAttributes(&objectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
		CLIENT_ID client_id;
		Globals.AVCoreServicePID = connectionCtx->ProcessID;
		client_id.UniqueProcess = connectionCtx->ProcessID;
		client_id.UniqueThread = 0;
		status = ZwOpenProcess(&Globals.AVCoreServiceHandle, PROCESS_ALL_ACCESS, &objectAttributes, &client_id);
		if (status != STATUS_SUCCESS)
		{
			// couldn't get AVCore service process handle
			*ConnectionCookie = NULL;
			return STATUS_INVALID_PARAMETER_3;
		}
		status = PsLookupProcessByProcessId(connectionCtx->ProcessID, &Globals.AVCoreServiceEprocess);
		if (status != STATUS_SUCCESS)
		{
			// couldn't get AVCore service process handle
			*ConnectionCookie = NULL;
			return STATUS_INVALID_PARAMETER_3;
		}
		*ConnectionCookie = connectionCookie;
		break;
	case AvConnectForAbort:
		Globals.AbortClientPort = ClientPort;
		*ConnectionCookie = connectionCookie;
		break;
	default:
		AV_DBG_PRINT(AVDBG_TRACE_ERROR,
			("[AV]: AvConnectNotifyCallback: No such connection type. \n"));
		ExFreePoolWithTag(connectionCookie,
			AV_CONNECTION_CTX_TAG);
		*ConnectionCookie = NULL;
		return STATUS_INVALID_PARAMETER_3;
	}

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[AV]: AvConnectNotifyCallback entered. type: %d \n", connectionCtx->Type));

	return STATUS_SUCCESS;
}


VOID AVEventsDriverDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
)
/*++
Routine Description
	Communication disconnection callback routine.
	This is called when user-mode disconnects the server port.
Arguments
	ConnectionCookie - The cookie set in AvConnectNotifyCallback(...). It is connection context.
Return Value
	None
--*/
{
	PAV_CONNECTION_TYPE connectionType = (PAV_CONNECTION_TYPE)ConnectionCookie;

	PAGED_CODE();

	if (NULL == connectionType)
	{
		return;
	}

	//  Close communication handle
	switch (*connectionType)
	{
	case AvConnectForScan:
		FltCloseClientPort(Globals.Filter, &Globals.ScanClientPort);
		Globals.ScanClientPort = NULL;
		break;
	case AvConnectForAbort:
		FltCloseClientPort(Globals.Filter, &Globals.AbortClientPort);
		Globals.AbortClientPort = NULL;
		break;
	default:
		AV_DBG_PRINT(AVDBG_TRACE_ERROR,
			("[AV]: AvDisconnectNotifyCallback: No such connection type. \n"));
		return;
	}

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[AV]: AvDisconnectNotifyCallback entered. type: %d \n", *connectionType));

	ExFreePoolWithTag(connectionType,
		AV_CONNECTION_CTX_TAG);
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/

FLT_PREOP_CALLBACK_STATUS AVEventsDriverPreMjCreate(
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

	if (!Globals.AVCoreServiceHandle)
	{
		// AVCore service is not listening skip event.
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	HANDLE curProcess = PsGetCurrentProcessId();
	if (curProcess == Globals.AVCoreServicePID)
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
	SIZE_T umBuffFileNameSize = eventFileCreate.FileNameSize;
	status = memmoveUM(FileObject->FileName.Buffer, &umBuffFileNameSize, &eventFileCreate.FileName);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	// Put volume name information to UM memory and save address in Event stucture.
	eventFileCreate.VolumeNameSize = volumeInformation->FilterVolumeNameLength;
	SIZE_T umBuffVolumeNameSize = eventFileCreate.VolumeNameSize;
	status = memmoveUM(volumeInformation->FilterVolumeName, &umBuffVolumeNameSize, &eventFileCreate.VolumeName);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	// Prepare AV_MESSAGE structure that will be sent to UM via comm port.
	AV_MESSAGE avMessage = { 0 };
	avMessage.MessageType = AvMsgEvent;
	avMessage.EventBuffer = NULL;
	avMessage.EventBufferLength = sizeof(AV_EVENT_FILE_CREATE);
	
	// Put Event structure to UM memory and save address in AV_MESSAGE structure.
	SIZE_T umBuffEventSize = avMessage.EventBufferLength;
	status = memmoveUM(&eventFileCreate, &umBuffEventSize, &avMessage.EventBuffer);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	AV_EVENT_RESPONSE UMResponse;
	ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

	// Send event to the AVCore UM service and wait for the response
	status = FltSendMessage(Globals.Filter,
		&Globals.ScanClientPort,
		&avMessage,
		sizeof(AV_MESSAGE),
		&UMResponse,
		&replyLength,
		NULL);

	// Got reply. Free memory. We need to free all UM-allocated buffers.
#pragma region free UM memory
	NTSTATUS freeStatus = ZwFreeVirtualMemory(Globals.AVCoreServiceHandle, &eventFileCreate.FileName, &umBuffFileNameSize, MEM_DECOMMIT);
	if (freeStatus != STATUS_SUCCESS) { return FLT_PREOP_SUCCESS_NO_CALLBACK; }

	freeStatus = ZwFreeVirtualMemory(Globals.AVCoreServiceHandle, &eventFileCreate.VolumeName, &umBuffVolumeNameSize, MEM_DECOMMIT);
	if (freeStatus != STATUS_SUCCESS) { return FLT_PREOP_SUCCESS_NO_CALLBACK; }

	freeStatus = ZwFreeVirtualMemory(Globals.AVCoreServiceHandle, &avMessage.EventBuffer, &umBuffEventSize, MEM_DECOMMIT);
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

NTSTATUS AVEventsDriverPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AV_CONNECTION_TYPE  ConnectionType
)
	/*++
	Routine Description:
		A wrapper function that prepare the communicate port.
	Arguments:
		SecurityDescriptor - Specifies a security descriptor to InitializeObjectAttributes(...).
		ConnectionType - The type of connection: AvConnectForScan, AvConnectForAbort, AvConnectForQuery
	Return Value:
		Returns the status of the prepartion.
	--*/
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniString;
	LONG maxConnections = 1;
	PCWSTR portName = NULL;
	PFLT_PORT* pServerPort = NULL;

	PAGED_CODE();

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[AV]: AvPrepareServerPort entered. \n"));

	switch (ConnectionType)
	{
	case AvConnectForScan:
		portName = AV_SCAN_PORT_NAME;
		pServerPort = &Globals.EventsServerPort;
		break;
	case AvConnectForAbort:
		portName = AV_ABORT_PORT_NAME;
		pServerPort = &Globals.AbortServerPort;
		break;
	default:
		FLT_ASSERTMSG("No such connection type.\n", FALSE);
		return STATUS_INVALID_PARAMETER;
	}

	RtlInitUnicodeString(&uniString, portName);

	InitializeObjectAttributes(&oa,
		&uniString,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		SecurityDescriptor);

	status = FltCreateCommunicationPort(Globals.Filter,
		pServerPort,  // this is the output to server port.
		&oa,
		NULL,
		AVEventsDriverConnectNotifyCallback,
		AVEventsDriverDisconnectNotifyCallback,
		NULL,
		maxConnections);

	return status;
}

NTSTATUS AVEventsDriverSendUnloadingToUser(
	VOID
)
	/*++
	Routine Description:
		This routine sends unloading message to the user program.
	Arguments:
		None.
	Return Value:
		The return value is the status of the operation.
	--*/
{
	NTSTATUS status = STATUS_SUCCESS;
	AV_MESSAGE event;

	PAGED_CODE();

	event.MessageType = AvMsgFilterUnloading;

	//  Tell the user-scanner that we are unloading the filter.
	//  and waits for its reply.
	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[Av]: AvSendUnloadingToUser: BEFORE...\n"));

	status = FltSendMessage(Globals.Filter,
		&Globals.AbortClientPort,
		&event,
		sizeof(AV_MESSAGE),
		NULL,
		NULL,
		NULL);

	if (!NT_SUCCESS(status))
	{
		AV_DBG_PRINT(AVDBG_TRACE_ERROR,
			("[Av]: AvSendUnloadingToUser: Failed to FltSendMessage.\n, 0x%08x\n",
				status));
	}

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[Av]: AvSendUnloadingToUser: After...\n"));

	return status;
}


NTSTATUS memmoveUM(void* srcBuffer, PSIZE_T size, void** outUmBuffer)
	/*
	Routine Description:
		Allocates a block of memory in UM AVCore service and transferes 
		given KM memory block there.
	Arguments:
		_in_ srcBuffer - pointer to the source buffer located in KM address space.
		_in_ size - number of bytes to move from srcBuffer to UM.
		_out_ outUmBuffer - pointer to the pointer that will recieve the address
		of the buffer allocated in the UM address space.
	*/
{
	SIZE_T originalSize = *size;
	NTSTATUS status = ZwAllocateVirtualMemory(Globals.AVCoreServiceHandle, outUmBuffer, 0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return status;
	}

	// Save operands to global kernel address space because
	// they might be unavailable after stack switch.
	Globals.Target = *outUmBuffer;
	Globals.Source = srcBuffer;
	Globals.Size = originalSize;

	// Chnage stack to that of the target UM process (AVCore service)
	KAPC_STATE pkapcState;
	KeStackAttachProcess(Globals.AVCoreServiceEprocess, &pkapcState);
	memcpy(Globals.Target, Globals.Source, Globals.Size);
	// Restore stack
	KeUnstackDetachProcess(&pkapcState);

	return STATUS_SUCCESS;
}