/**
\file
\brief Implements KMEventsAPI

Exports Driver that implements KM-UM communication interface
in KM. Communication interfaces are provided via exports.
*/

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

NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, void** outUmBuffer, PSIZE_T outUmSize);

NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);

NTSTATUS AVCommSendEvent(AV_EVENT_TYPE, void*, int, PAV_EVENT_RESPONSE, PULONG);

HANDLE AVCommGetUmPID(VOID);

UCHAR AVCommIsExcludedPID(HANDLE PID);

UCHAR AVCommIsInitialized(VOID);

VOID AVCommGetUmBuffer(PVOID umAddr, PVOID outKmBuffer, SIZE_T size);

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
	#pragma alloc_text(PAGE, AVCommGetUmBuffer)
	#pragma alloc_text(PAGE, AVCommFreeBuffer)
	#pragma alloc_text(PAGE, AVCommSendEvent)
	#pragma alloc_text(PAGE, AVCommIsExcludedPID)
	#pragma alloc_text(PAGE, AVCommIsInitialized)

#endif

/**
Global parameters. Holds KM-UM communication context.
*/
AV_COMM_GLOBAL_DATA Globals;

/**
This function is called when DLL is loaded.

\param[in] RegistryPath Path to the service regisy key.

\return Initialization status.
*/
NTSTATUS DllInitialize(
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("AVCommDriver.sys is now loading\n");
	return STATUS_SUCCESS;
}

/**
\brief Unload routine.

This function is called when DLL is being unloaded.

\return STATUS_SUCCESS.
*/
NTSTATUS DllUnload(VOID)
{
    DbgPrint("AVCommDriver.sys is now unloading\n");
    return STATUS_SUCCESS;
}

/**
Temporary entry point needed to initialize the class system dll.
It only zeros out the globals.

\param[in] DriverObject Pointer to the driver object created by the system.
\return STATUS_SUCCESS
*/
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
	UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

	RtlZeroMemory(&Globals, sizeof(Globals));

    return STATUS_SUCCESS;
}

/**
Communication connection callback routine.
This is called when user-mode connects to the server port.
This function sets up KM-UM global communication context.

\param[in] ClientPort This is the client connection port that will be used to send messages from the filter

\param[in] ServerPortCookie Unused

\param[in] ConnectionContext The connection context passed from the user. This is to recognize which type
connection the user is trying to connect.

\param[in] SizeofContext The size of the connection context.

\param[out] ConnectionCookie Propagation of the connection context to disconnection callback.

\return STATUS_SUCCESS - to accept the connection
STATUS_INSUFFICIENT_RESOURCES - if memory is not enough
STATUS_INVALID_PARAMETER_3 - Connection context is not valid.
*/
NTSTATUS AVCommConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
)
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
	case AvConnectForEvents:
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

/**
Communication disconnection callback routine.
This is called when user-mode disconnects the server port.

\param[in,out] ConnectionCookie The cookie set in AvConnectNotifyCallback(...). It is connection context.
*/
VOID AVCommDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
)
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
	case AvConnectForEvents:
		FltCloseClientPort(Globals.Filter, &Globals.EventsClientPort);
		Globals.EventsClientPort = NULL;
		break;
	default:
		return;
	}

	ExFreePoolWithTag(connectionType,
		AV_CONNECTION_CTX_TAG);
}

/**
A wrapper function that prepare the communicate port.

\param[in] SecurityDescriptor Specifies a security descriptor to InitializeObjectAttributes(...).
\param[in] ConnectionType The type of connection: AvConnectForEvents

\return Status of the prepartion.
*/
NTSTATUS AVCommPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AV_CONNECTION_TYPE  ConnectionType
)
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
	case AvConnectForEvents:
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

/**
Initialises minifilter-driver that is used for KM-UM communications.

\param[in] Filter Pointer to minifilter driver instance that will be used to register communication port.

\return Status of initialization.
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
		status = AVCommPrepareServerPort(sd, AvConnectForEvents);
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

/**
\brief Closes KM-UM communication.

Stops communication port. Zeros out Filter pointer.
*/
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

/**
\brief Transfers KM buffer to UM.

Allocates a block of memory in UM AVCore service and transferes
given KM memory block there.

\param[in] srcBuffer Pointer to the source buffer located in KM address space.
\param[in] srcSize Number of bytes to move from srcBuffer to UM.
\param[out] outUmBuffer Pointer to the pointer that will recieve the address of created buffer.
\param[out\ outUmSize Pointer to the variable where the size of allocated UM buffer will be stored.
of the buffer allocated in the UM address space. Should be zero.

\return Status of operation.
*/
NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, void **outUmBuffer, PSIZE_T outUmSize)
{
	*outUmSize = srcSize;
	// allocat memory in UM address space of AVCore service.
	NTSTATUS status = ZwAllocateVirtualMemory(Globals.AVCoreServiceHandle, outUmBuffer, 0, outUmSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (status != STATUS_SUCCESS)
	{
		// couldn't allocate memory in UM
		return status;
	}

	// Chnage stack to that of the target UM process (AVCore service)
	KAPC_STATE pkapcState;
	KeStackAttachProcess(Globals.AVCoreServiceEprocess, &pkapcState);
	// copy buffer from KM to allocated buffer in UM.
	memcpy(*outUmBuffer, srcBuffer, srcSize);
	// Restore stack
	KeUnstackDetachProcess(&pkapcState);

	return status;
}

/**
\brief Retrieves buffer from UM.

Copies buffer from UM. This API is used to copy UM responses buffer
to KM memory. KM buffer should be large enough to receive size bytes.

\param[in] umAddr Address of the buffer provided from UM.
\param[out] outKmBuffer Pointer to the KM bufferthat will recieve contents of specified UM buffer.
\param[in] size Size of the UM buffer.
*/
VOID AVCommGetUmBuffer(PVOID umAddr, PVOID outKmBuffer, SIZE_T size)
{
	// Chnage stack to that of the target UM process (AVCore service)
	KAPC_STATE pkapcState;
	KeStackAttachProcess(Globals.AVCoreServiceEprocess, &pkapcState);
	// copy buffer from KM to allocated buffer in UM.
	memcpy(outKmBuffer, umAddr, size);
	// Restore stack
	KeUnstackDetachProcess(&pkapcState);
}

/**
\brief Frees UM memory

Frees the block of memory in UM address space
that was allocated via AVCommCreateBuffer.

\param[in] UmBuffer Pointer to the UM buffer (outUmBuffer)
\param[in] UmBufferSize Size of UM buffer received from AVCommCreateBuffer (outUmSize)

\return Status of operation.
*/
NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize)
{
	return ZwFreeVirtualMemory(Globals.AVCoreServiceHandle, UmBuffer, UmBufferSize, MEM_RELEASE);
}

/**
\brief Sends given event to UM service via communication port.

This API is blocking. Function will return after event processing in UM.

\param[in] eventBuffer Pointer to the event structure formed in KM memory space.
\param[in] eventBufferSize Size of eventBuffer.
\param[out] UMResponse Pointer to buffer that will receive AV_EVENT_RESPONSE structure.
\param[out] UMResponseLength Size of UMResponse buffer.

\return Status of event submition.
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

	NTSTATUS freeStatus = AVCommFreeBuffer(&avMessage.EventBuffer, &umBuffEventSize);
	if (freeStatus != STATUS_SUCCESS) { return freeStatus; }

	return status;
}

/**
\brief Implements KM event scanning exclusion based on PID.

This API should be called before submitting events via AVCommSendEvent
to check whether the process is exluded (truested).
By default AVCore service process is trusted.

\param[in] PID Current PID.

\return Excluded (TRUE).
*/
UCHAR AVCommIsExcludedPID(HANDLE PID)
{
	return Globals.AVCoreServicePID == PID;
}

/**
\brief Checks KM-UM communication state.

Checks weather client comm port was set up (KM-UM communication
was established).

\return BOOLEAN
*/
UCHAR AVCommIsInitialized()
{
	return Globals.EventsClientPort != NULL;
}