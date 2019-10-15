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
ULONG_PTR OperationStatusCtx = 1;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
AVEventsDriverInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
AVEventsDriverInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
AVEventsDriverInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
AVEventsDriverUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
AVEventsDriverInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
AVEventsDriverPreMjCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

NTSTATUS
AvConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
);

VOID
AvDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS
AvPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AVSCAN_CONNECTION_TYPE  ConnectionType
);

NTSTATUS
AvSendUnloadingToUser(
	VOID
);

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, AVEventsDriverUnload)
#pragma alloc_text(PAGE, AVEventsDriverInstanceQueryTeardown)
#pragma alloc_text(PAGE, AVEventsDriverInstanceSetup)
#pragma alloc_text(PAGE, AVEventsDriverInstanceTeardownStart)
#pragma alloc_text(PAGE, AvConnectNotifyCallback)
#pragma alloc_text(PAGE, AVEventsDriverPreMjCreate)
#pragma alloc_text(PAGE, AvDisconnectNotifyCallback)
#pragma alloc_text(PAGE, AvPrepareServerPort)
#pragma alloc_text(PAGE, AvSendUnloadingToUser)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      AVEventsDriverPreMjCreate,
      NULL },

    { IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

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


NTSTATUS
AVEventsDriverInstanceSetup (
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


NTSTATUS
AVEventsDriverInstanceQueryTeardown (
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


VOID
AVEventsDriverInstanceTeardownStart (
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


VOID
AVEventsDriverInstanceTeardownComplete (
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

NTSTATUS
DriverEntry(
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

	//
	//  Set default global configuration
	//

	RtlZeroMemory(&Globals, sizeof(Globals));
	InitializeListHead(&Globals.ScanCtxListHead);
	ExInitializeResourceLite(&Globals.ScanCtxListLock);

	Globals.ScanIdCounter = 0;
	Globals.LocalScanTimeout = 30000;
	Globals.NetworkScanTimeout = 60000;

#if DBG

	Globals.DebugLevel = 0xffffffff; // AVDBG_TRACE_ERROR | AVDBG_TRACE_DEBUG;

#endif

	try {

		//
		//  Register with FltMgr to tell it our callback routines
		//

		status = FltRegisterFilter(DriverObject,
			&FilterRegistration,
			&Globals.Filter);

		if (!NT_SUCCESS(status)) {

			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: FltRegisterFilter FAILED. status = 0x%x\n", status));
			leave;
		}

		//
		//  Builds a default security descriptor for use with FltCreateCommunicationPort.
		//

		status = FltBuildDefaultSecurityDescriptor(&sd,
			FLT_PORT_ALL_ACCESS);


		if (!NT_SUCCESS(status)) {

			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: FltBuildDefaultSecurityDescriptor FAILED. status = 0x%x\n", status));
			leave;
		}
		//
		//  Prepare ports between kernel and user.
		//

		status = AvPrepareServerPort(sd, AvConnectForScan);

		if (!NT_SUCCESS(status)) {

			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: AvPrepareServerPort Scan Port FAILED. status = 0x%x\n", status));
			leave;
		}

		status = AvPrepareServerPort(sd, AvConnectForAbort);

		if (!NT_SUCCESS(status)) {

			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: AvPrepareServerPort Abort Port FAILED. status = 0x%x\n", status));
			leave;
		}

		status = AvPrepareServerPort(sd, AvConnectForQuery);

		if (!NT_SUCCESS(status)) {

			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: AvPrepareServerPort Query Port FAILED. status = 0x%x\n", status));
			leave;
		}

		//
		//  Start filtering i/o
		//

		status = FltStartFiltering(Globals.Filter);

		if (!NT_SUCCESS(status)) {

			AV_DBG_PRINT(AVDBG_TRACE_ERROR,
				("[AV] DriverEntry: FltStartFiltering FAILED. status = 0x%x\n", status));
			leave;
		}

	}
	finally{

	 if (sd != NULL) {

		 FltFreeSecurityDescriptor(sd);
	 }

	 if (!NT_SUCCESS(status)) {

		 if (NULL != Globals.ScanServerPort) {

			  FltCloseCommunicationPort(Globals.ScanServerPort);
		 }
		 if (NULL != Globals.AbortServerPort) {

			  FltCloseCommunicationPort(Globals.AbortServerPort);
		 }
		 if (NULL != Globals.QueryServerPort) {

			  FltCloseCommunicationPort(Globals.QueryServerPort);
		 }
		 if (NULL != Globals.Filter) {

			 FltUnregisterFilter(Globals.Filter);
			 Globals.Filter = NULL;
		 }

		 ExDeleteResourceLite(&Globals.ScanCtxListLock);
	 }
	}

	return status;
}

NTSTATUS
AVEventsDriverUnload (
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

	//
	//  Traverse the scan context list, and cancel the scan if it exists.
	//

	Globals.Unloading = TRUE;

	//
	//  This function will wait for the user to abort the outstanding scan and 
	//  close the section 
	//

	AvSendUnloadingToUser();

	FltCloseCommunicationPort(Globals.ScanServerPort);
	Globals.ScanServerPort = NULL;
	FltCloseCommunicationPort(Globals.AbortServerPort);
	Globals.AbortServerPort = NULL;
	FltCloseCommunicationPort(Globals.QueryServerPort);
	Globals.QueryServerPort = NULL;
	FltUnregisterFilter(Globals.Filter);  // This will typically trigger instance tear down.
	Globals.Filter = NULL;

	ExDeleteResourceLite(&Globals.ScanCtxListLock);

	return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
AVEventsDriverPreMjCreate(
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

	//
	//  Stack file objects are never scanned.
	//

	IoGetStackLimits(&stackLow, &stackHigh);

	if (((ULONG_PTR)FileObject > stackLow) &&
		((ULONG_PTR)FileObject < stackHigh)) {

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	ULONG RetCode;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG replyLength = sizeof(ULONG);
	AV_SCANNER_NOTIFICATION notification = { 0 };

	notification.Message = AvMsgEvent;

	notification.BufferLength = FileObject->FileName.Length;
	memcpy(notification.Buffer, FileObject->FileName.Buffer, notification.BufferLength);

	//
	//  Tell the user-scanner that we are unloading the filter.
	//  and waits for its reply.
	//

	status = FltSendMessage(Globals.Filter,
		&Globals.ScanClientPort,
		&notification,
		sizeof(AV_SCANNER_NOTIFICATION),
		&RetCode,
		&replyLength,
		NULL);
	if (status == STATUS_SUCCESS)
	{
		if (RetCode)
			return FLT_PREOP_SUCCESS_NO_CALLBACK;
		else
		{
			Data->IoStatus.Status = STATUS_ACCESS_DENIED;
			return FLT_PREOP_COMPLETE;
		}
	}
	else
	{
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}
}

NTSTATUS
AvConnectNotifyCallback(
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
	PAVSCAN_CONNECTION_TYPE connectionCookie = NULL;

	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(SizeOfContext);

	if (NULL == connectionCtx) {

		return STATUS_INVALID_PARAMETER_3;
	}

	//
	//  ConnectionContext passed in may be deleted. We need to make a copy of it.
	//

	connectionCookie = (PAVSCAN_CONNECTION_TYPE)ExAllocatePoolWithTag(PagedPool,
		sizeof(AVSCAN_CONNECTION_TYPE),
		AV_CONNECTION_CTX_TAG);
	if (NULL == connectionCookie) {

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	*connectionCookie = connectionCtx->Type;
	switch (connectionCtx->Type) {
	case AvConnectForScan:
		Globals.ScanClientPort = ClientPort;
		*ConnectionCookie = connectionCookie;
		break;
	case AvConnectForAbort:
		Globals.AbortClientPort = ClientPort;
		*ConnectionCookie = connectionCookie;
		break;
	case AvConnectForQuery:
		Globals.QueryClientPort = ClientPort;
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

VOID
AvDisconnectNotifyCallback(
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
	PAVSCAN_CONNECTION_TYPE connectionType = (PAVSCAN_CONNECTION_TYPE)ConnectionCookie;

	PAGED_CODE();

	if (NULL == connectionType) {

		return;
	}
	//
	//  Close communication handle
	//
	switch (*connectionType) {
	case AvConnectForScan:
		FltCloseClientPort(Globals.Filter, &Globals.ScanClientPort);
		Globals.ScanClientPort = NULL;
		break;
	case AvConnectForAbort:
		FltCloseClientPort(Globals.Filter, &Globals.AbortClientPort);
		Globals.AbortClientPort = NULL;
		break;
	case AvConnectForQuery:
		FltCloseClientPort(Globals.Filter, &Globals.QueryClientPort);
		Globals.QueryClientPort = NULL;
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

NTSTATUS
AvPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AVSCAN_CONNECTION_TYPE  ConnectionType
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

	switch (ConnectionType) {
	case AvConnectForScan:
		portName = AV_SCAN_PORT_NAME;
		pServerPort = &Globals.ScanServerPort;
		break;
	case AvConnectForAbort:
		portName = AV_ABORT_PORT_NAME;
		pServerPort = &Globals.AbortServerPort;
		break;
	case AvConnectForQuery:
		portName = AV_QUERY_PORT_NAME;
		pServerPort = &Globals.QueryServerPort;
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
		AvConnectNotifyCallback,
		AvDisconnectNotifyCallback,
		NULL,
		maxConnections);

	return status;
}

NTSTATUS
AvSendUnloadingToUser(
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
	ULONG abortThreadId;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG replyLength = sizeof(ULONG);
	AV_SCANNER_NOTIFICATION notification;

	PAGED_CODE();

	notification.Message = AvMsgFilterUnloading;

	//
	//  Tell the user-scanner that we are unloading the filter.
	//  and waits for its reply.
	//

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[Av]: AvSendUnloadingToUser: BEFORE...\n"));

	status = FltSendMessage(Globals.Filter,
		&Globals.AbortClientPort,
		&notification,
		sizeof(AV_SCANNER_NOTIFICATION),
		&abortThreadId,
		&replyLength,
		NULL);

	if (!NT_SUCCESS(status)) {

		AV_DBG_PRINT(AVDBG_TRACE_ERROR,
			("[Av]: AvSendUnloadingToUser: Failed to FltSendMessage.\n, 0x%08x\n",
				status));
	}

	AV_DBG_PRINT(AVDBG_TRACE_DEBUG,
		("[Av]: AvSendUnloadingToUser: After...\n"));

	return status;
}

