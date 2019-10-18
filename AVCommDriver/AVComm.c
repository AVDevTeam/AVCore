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
#include "EventsKM.h"

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

NTSTATUS AVCommSendUnloadingToUser(
	VOID
);

NTSTATUS memmoveUM(void*, PSIZE_T, void**);

void setFilter(PFLT_FILTER);

void closeCommunicationPorts(VOID);

HANDLE getAVCorePID(VOID);

HANDLE getAVCoreHandle(VOID);

NTSTATUS sendEvent(void*, int, PAV_EVENT_RESPONSE, PULONG);

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)
	#pragma alloc_text(PAGE, DllInitialize)
	#pragma alloc_text(PAGE, DllUnload)
	#pragma alloc_text(PAGE, AVCommConnectNotifyCallback)
	#pragma alloc_text(PAGE, AVCommDisconnectNotifyCallback)
	#pragma alloc_text(PAGE, AVCommPrepareServerPort)
	#pragma alloc_text(PAGE, AVCommSendUnloadingToUser)
	#pragma alloc_text(PAGE, memmoveUM)
	#pragma alloc_text(PAGE, setFilter)
	#pragma alloc_text(PAGE, closeCommunicationPorts)
	#pragma alloc_text(PAGE, getAVCorePID)
	#pragma alloc_text(PAGE, sendEvent)
	#pragma alloc_text(PAGE, getAVCoreHandle)
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
		FltCloseClientPort(Globals.Filter, &Globals.ScanClientPort);
		Globals.ScanClientPort = NULL;
		break;
	case AvConnectForAbort:
		FltCloseClientPort(Globals.Filter, &Globals.AbortClientPort);
		Globals.AbortClientPort = NULL;
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
		AVCommConnectNotifyCallback,
		AVCommDisconnectNotifyCallback,
		NULL,
		maxConnections);

	return status;
}

NTSTATUS AVCommSendUnloadingToUser(
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

	status = FltSendMessage(Globals.Filter,
		&Globals.AbortClientPort,
		&event,
		sizeof(AV_MESSAGE),
		NULL,
		NULL,
		NULL);

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
	of the buffer allocated in the UM address space. Should be zero.
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

void setFilter(PFLT_FILTER Filter)
{
	Globals.Filter = Filter;
}

void closeCommunicationPorts(VOID)
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

HANDLE getAVCorePID(VOID)
{
	return Globals.AVCoreServicePID;
}

HANDLE getAVCoreHandle(VOID)
{
	return Globals.AVCoreServiceHandle;
}

NTSTATUS sendEvent(void* eventBuffer, int eventBufferSize, PAV_EVENT_RESPONSE UMResponse, PULONG UMResponseLength)
{
	// Prepare AV_MESSAGE structure that will be sent to UM via comm port.
	AV_MESSAGE avMessage = { 0 };
	avMessage.MessageType = AvMsgEvent;
	avMessage.EventBuffer = NULL;
	avMessage.EventBufferLength = eventBufferSize;

	// Put Event structure to UM memory and save address in AV_MESSAGE structure.
	SIZE_T umBuffEventSize = avMessage.EventBufferLength;
	NTSTATUS status = memmoveUM(eventBuffer, &umBuffEventSize, &avMessage.EventBuffer);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	// Send event to the AVCore UM service and wait for the response
	status = FltSendMessage(Globals.Filter,
		&Globals.ScanClientPort,
		&avMessage,
		sizeof(AV_MESSAGE),
		UMResponse,
		UMResponseLength,
		NULL);

	NTSTATUS freeStatus = ZwFreeVirtualMemory(Globals.AVCoreServiceHandle, &avMessage.EventBuffer, &umBuffEventSize, MEM_DECOMMIT);
	if (freeStatus != STATUS_SUCCESS) { return freeStatus; }

	return status;
}