/*++
Module Name:
    AVComm.c
Abstract:
    Exports Driver that implements KM-UM communication interface
	in KM. Communication interfaces are provided via exports.
Environment:
    kernel mode only
--*/

#define CLASS_INIT_GUID 1
#define DEBUG_MAIN_SOURCE 1

#include "AVComm.h"
#include "Globals.h"
#include "KMUMcomm.h"
#include "EventsKMStructures.h"

NTSTATUS AVCommConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
);

VOID AVCommDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS AVCommPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AV_CONNECTION_TYPE  ConnectionType
);

NTSTATUS AVCommInit(PFLT_FILTER Filter);

void AVCommStop(VOID);

NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, PVOID *outUmBuffer, PSIZE_T outUmSize);

NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);

NTSTATUS AVCommSendEvent(AV_EVENT_TYPE, void*, int, PAV_EVENT_RESPONSE, PULONG);

HANDLE AVCommGetUmPID(VOID);

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)
	#pragma alloc_text(PAGE, DllInitialize)
	#pragma alloc_text(PAGE, DllUnload)
	#pragma alloc_text(PAGE, AVCommConnectNotifyCallback)
	#pragma alloc_text(PAGE, AVCommDisconnectNotifyCallback)
	#pragma alloc_text(PAGE, AVCommPrepareServerPort)
	#pragma alloc_text(PAGE, AVCommInit)
	#pragma alloc_text(PAGE, AVCommStop)
	#pragma alloc_text(PAGE, AVCommCreateBuffer)
	#pragma alloc_text(PAGE, AVCommFreeBuffer)
	#pragma alloc_text(PAGE, AVCommSendEvent)
	#pragma alloc_text(PAGE, AVCommGetUmPID)

#endif

#pragma prefast(disable:28159, "There are certain cases when we have to bugcheck...")

// Globals
AV_COMM_GLOBAL_DATA Globals;

/*
Routine Description:
	This function is called when DLL is loaded.
*/
NTSTATUS DllInitialize(
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("AVCommDriver.sys is now loading\n");
	return STATUS_SUCCESS;
}

/*
Routine Description:
	This function is called when DLL is being unloaded.
*/
NTSTATUS DllUnload(VOID)
{
    DbgPrint("AVCommDriver.sys is now unloading\n");
    return STATUS_SUCCESS;
}

/*++
DriverEntry()
Routine Description:
    Temporary entry point needed to initialize the class system dll.
    It doesn't do anything.
Arguments:
    DriverObject - Pointer to the driver object created by the system.
Return Value:
   STATUS_SUCCESS
--*/
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
	UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    return STATUS_SUCCESS;
}

NTSTATUS AVCommConnectNotifyCallback(
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
		Globals.EventsClientPort = ClientPort;

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
	default:
		ExFreePoolWithTag(connectionCookie,
			AV_CONNECTION_CTX_TAG);
		*ConnectionCookie = NULL;
		return STATUS_INVALID_PARAMETER_3;
	}

	return STATUS_SUCCESS;
}

VOID AVCommDisconnectNotifyCallback(
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
		FltCloseClientPort(Globals.Filter, &Globals.EventsClientPort);
		Globals.EventsClientPort = NULL;
		break;
	default:
		return;
	}

	ExFreePoolWithTag(connectionType,
		AV_CONNECTION_CTX_TAG);
}

NTSTATUS AVCommPrepareServerPort(
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

	switch (ConnectionType)
	{
	case AvConnectForScan:
		portName = AV_SCAN_PORT_NAME;
		pServerPort = &Globals.EventsServerPort;
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
		AVCommConnectNotifyCallback,
		AVCommDisconnectNotifyCallback,
		NULL,
		maxConnections);

	return status;
}

/*
Routine Desription:
	Initialises minifilter-driver that is used for KM-UM communications.
*/
NTSTATUS AVCommInit(PFLT_FILTER Filter)
{
	NTSTATUS status = STATUS_SUCCESS;
	PSECURITY_DESCRIPTOR sd = NULL;

	Globals.Filter = Filter;

	try
	{
		if (!NT_SUCCESS(status))
		{
			leave;
		}

		//  Builds a default security descriptor for use with FltCreateCommunicationPort.
		status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);

		if (!NT_SUCCESS(status))
		{
			leave;
		}

		//  Prepare ports between kernel and user.
		status = AVCommPrepareServerPort(sd, AvConnectForScan);
		if (!NT_SUCCESS(status))
		{
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
			AVCommStop();
		}
	}

	return status;
}

void AVCommStop(VOID)
{
	if (NULL != Globals.EventsServerPort)
	{
		FltCloseCommunicationPort(Globals.EventsServerPort);
	}
	if (NULL != Globals.Filter)
	{
		Globals.Filter = NULL;
	}
}

/*
Routine Description:
	Allocates a block of memory in UM AVCore service and transferes
	given KM memory block there.
Arguments:
	_in_ srcBuffer - pointer to the source buffer located in KM address space.
	_in_ srcSize - number of bytes to move from srcBuffer to UM.
	_out_ outUmBuffer - pointer to the pointer that will recieve the address
	_out_ outUmSize - pointer to the variable where the size of allocated UM buffer will be stored.
	of the buffer allocated in the UM address space. Should be zero.
*/
NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, PVOID *outUmBuffer, PSIZE_T outUmSize)
{
	PVOID UmBuffer = NULL;
	*outUmSize = srcSize;
	// allocat memory in UM address space of AVCore service.
	NTSTATUS status = ZwAllocateVirtualMemory(Globals.AVCoreServiceHandle, &UmBuffer, 0, outUmSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return status;
	}

	// Chnage stack to that of the target UM process (AVCore service)
	KAPC_STATE pkapcState;
	KeStackAttachProcess(Globals.AVCoreServiceEprocess, &pkapcState);
	// copy buffer from KM to allocated buffer in UM.
	memcpy(UmBuffer, srcBuffer, srcSize);
	// Restore stack
	KeUnstackDetachProcess(&pkapcState);

	*outUmBuffer = UmBuffer;
	return status;
}

/*
Routine Description:
	Frees the block of memory in UM address space
	that was allocated via AVCommCreateBuffer.
Arguments:
	UmBuffer - pointer to the UM buffer (outUmBuffer)
	UmBufferSize - size of UM buffer received from AVCommCreateBuffer (outUmSize)
*/
NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize)
{
	return ZwFreeVirtualMemory(Globals.AVCoreServiceHandle, &UmBuffer, UmBufferSize, MEM_DECOMMIT);
}

/*
Routine Description:
	Getter function for AVCoreServicePID that was recieved
	in AVCommConnectNotifyCallback.
*/
HANDLE AVCommGetUmPID(VOID)
{
	return Globals.AVCoreServicePID;
}

/*
Routine Description:
	Sends given event to UM service via communication port.
Arguments:
	eventBuffer - pointer to the event structure formed in KM memory space.
	eventBufferSize - size of eventBuffer.
	UMResponse - pointer to buffer that will receive AV_EVENT_RESPONSE structure.
	UMResponseLength - size of UMResponse buffer.
*/
NTSTATUS AVCommSendEvent(AV_EVENT_TYPE eventType, void* eventBuffer, int eventBufferSize, PAV_EVENT_RESPONSE UMResponse, PULONG UMResponseLength)
{
	// Prepare AV_MESSAGE structure that will be sent to UM via comm port.
	AV_MESSAGE avMessage = { 0 };
	avMessage.MessageType = AvMsgEvent;
	avMessage.EventType = eventType;
	avMessage.EventBuffer = NULL;
	avMessage.EventBufferLength = eventBufferSize;

	// Put Event structure to UM memory and save address in AV_MESSAGE structure.
	SIZE_T umBuffEventSize = avMessage.EventBufferLength;
	NTSTATUS status = AVCommCreateBuffer(eventBuffer, umBuffEventSize, &avMessage.EventBuffer, &umBuffEventSize);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	// Send event to the AVCore UM service and wait for the response
	status = FltSendMessage(Globals.Filter,
		&Globals.EventsClientPort,
		&avMessage,
		sizeof(AV_MESSAGE),
		UMResponse,
		UMResponseLength,
		NULL);

	NTSTATUS freeStatus = AVCommFreeBuffer(avMessage.EventBuffer, &umBuffEventSize);
	if (freeStatus != STATUS_SUCCESS) { return freeStatus; }

	return status;
}