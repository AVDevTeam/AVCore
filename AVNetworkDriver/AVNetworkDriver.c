#include "AVNetworkDriver.h"
#include "ntstrsafe.h"

HANDLE gEngineHandle, gInjectionHandle;
DEVICE_OBJECT* gWdmDevice;
UINT32 gOutboundTlCalloutIdV4, gInboundTlCalloutIdV4, gOutboundTlCalloutIdV6, gInboundTlCalloutIdV6;

KEVENT gWorkerEvent;
BOOLEAN gDriverUnloading = FALSE;
void* gThreadObj;
HANDLE threadHandle;
HANDLE gInjectionHandle;
KSPIN_LOCK gPacketQueueLock;
KLOCK_QUEUE_HANDLE packetQueueLockHandle;

LIST_ENTRY gPacketQueue;

NTSTATUS
TLInspectCloneReinjectInbound(
	_Inout_ TL_INSPECT_PENDED_PACKET* packet
);

NTSTATUS
TLInspectCloneReinjectOutbound(
	_Inout_ TL_INSPECT_PENDED_PACKET* packet
);

void
TLInspectWorker(
	_In_ void* StartContext
)
{
	UNREFERENCED_PARAMETER(StartContext);

	for (;;)
	{
		KeWaitForSingleObject(
			&gWorkerEvent,
			Executive,
			KernelMode,
			FALSE,
			NULL
		);

		if (gDriverUnloading)
		{
			break;
		}

		while (!IsListEmpty(&gPacketQueue) && AVCommIsInitialized()) 
		{
			KeAcquireInStackQueuedSpinLock(&gPacketQueueLock, &packetQueueLockHandle);
			PLIST_ENTRY listEntry = RemoveHeadList(&gPacketQueue);
			KeReleaseInStackQueuedSpinLock(&packetQueueLockHandle);

			PTL_INSPECT_PENDED_PACKET pandedpacket = CONTAINING_RECORD(listEntry, TL_INSPECT_PENDED_PACKET, listEntry);
			AV_EVENT_NETWORK eventInfo;
			SIZE_T umBufferSize;
			copyPacketInfo(pandedpacket, &eventInfo, &umBufferSize);
			DbgPrint("AVNetworkDriver | TLInspectWorker | event %hhu:%hhu:%hhu:%hhu:%hu -> %hhu:%hhu:%hhu:%hhu:%hu\n", 
				eventInfo.localAddress[0],
				eventInfo.localAddress[1],
				eventInfo.localAddress[2],
				eventInfo.localAddress[3],
				eventInfo.localPort,
				eventInfo.remoteAddress[0], 
				eventInfo.remoteAddress[1], 
				eventInfo.remoteAddress[2], 
				eventInfo.remoteAddress[3],
				eventInfo.remotePort);

			AV_EVENT_RESPONSE UMResponse;
			ULONG replyLength = sizeof(AV_EVENT_RESPONSE);
			AVCommSendEvent(
				AvNetwork,
				&eventInfo,
				sizeof(AV_EVENT_NETWORK),
				&UMResponse,
				&replyLength);
			if (UMResponse.Status == AvEventStatusBlock) {
				DbgPrint("AVNetworkDriver | TLInspectWorker | block result: block\n");
			}
			else {
				DbgPrint("AVNetworkDriver | TLInspectWorker | block result: allow\n");
			}

			AVCommFreeBuffer(&eventInfo.data, &umBufferSize);
			
			NTSTATUS status;
			if (UMResponse.Status == AvEventStatusAllow)
			{
				if (pandedpacket->direction == FWP_DIRECTION_OUTBOUND)
				{
					status = TLInspectCloneReinjectOutbound(pandedpacket);
				}
				else
				{
					status = TLInspectCloneReinjectInbound(pandedpacket);
				}

				if (NT_SUCCESS(status))
				{
					pandedpacket = NULL; // ownership transferred.
				}

			}

			if (pandedpacket != NULL)
			{
				FreePendedPacket(pandedpacket);
			}
		}
	}
	PsTerminateSystemThread(STATUS_SUCCESS);
}

void
TLInspectTransportClassify(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
)
{
	DbgPrint("AVNetworkDriver | TLInspectTransportClassify | enter | ProcessId %p\n", PsGetCurrentProcessId());

	FWPS_PACKET_INJECTION_STATE packetState;
	BOOLEAN signalWorkerThread;
	TL_INSPECT_PENDED_PACKET* pendedPacket = NULL;

	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(inMetaValues);

	//
	// We don't have the necessary right to alter the classify, exit.
	//
	if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0)
	{
		return;
	}

	NT_ASSERT(layerData != NULL);
	_Analysis_assume_(layerData != NULL);

	//
	// We don't re-inspect packets that we've inspected earlier.
	//
	packetState = FwpsQueryPacketInjectionState(
		gInjectionHandle,
		layerData,
		NULL
	);

	if ((packetState == FWPS_PACKET_INJECTED_BY_SELF) ||
		(packetState == FWPS_PACKET_PREVIOUSLY_INJECTED_BY_SELF))
	{
		classifyOut->actionType = FWP_ACTION_PERMIT;
		if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
		{
			classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		}

		return;
	}

	pendedPacket = AllocateAndInitializePendedPacket(
		inFixedValues,
		inMetaValues,
		layerData
	);

	if (pendedPacket == NULL)
	{
		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		goto Exit;
	}

	KeAcquireInStackQueuedSpinLock(
		&gPacketQueueLock,
		&packetQueueLockHandle
	);

	if (!gDriverUnloading)
	{
		signalWorkerThread = IsListEmpty(&gPacketQueue);

		InsertTailList(&gPacketQueue, &pendedPacket->listEntry);
		pendedPacket = NULL; // ownership transferred

		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		classifyOut->flags |= FWPS_CLASSIFY_OUT_FLAG_ABSORB;
	}
	else
	{
		//
		// Driver is being unloaded, permit any connect classify.
		//
		signalWorkerThread = FALSE;

		classifyOut->actionType = FWP_ACTION_PERMIT;
		if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
		{
			classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		}
	}

	KeReleaseInStackQueuedSpinLock(&packetQueueLockHandle);

	if (signalWorkerThread)
	{
		KeSetEvent(
			&gWorkerEvent,
			0,
			FALSE
		);
	}
Exit:

	if (pendedPacket != NULL)
	{
		FreePendedPacket(pendedPacket);
	}

	return;
}


NTSTATUS
TLInspectTransportNotify(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ const FWPS_FILTER* filter
)
{
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	return STATUS_SUCCESS;
}

NTSTATUS
TLInspectAddFilter(
	_In_ const wchar_t* filterName,
	_In_ const wchar_t* filterDesc,
	_In_ UINT64 context,
	_In_ const GUID* layerKey,
	_In_ const GUID* calloutKey
)
{
	NTSTATUS status = STATUS_SUCCESS;

	FWPM_FILTER filter = { 0 };

	filter.layerKey = *layerKey;
	filter.displayData.name = (wchar_t*)filterName;
	filter.displayData.description = (wchar_t*)filterDesc;

	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = *calloutKey;
	filter.filterCondition = NULL;
	filter.subLayerKey = TL_INSPECT_SUBLAYER;
	filter.weight.type = FWP_EMPTY; // auto-weight.
	filter.rawContext = context;
	filter.numFilterConditions = 0;

	status = FwpmFilterAdd(
		gEngineHandle,
		&filter,
		NULL,
		NULL);

	return status;
}

NTSTATUS
TLInspectRegisterTransportCallouts(
	_In_ const GUID* layerKey,
	_In_ const GUID* calloutKey,
	_Inout_ void* deviceObject,
	_Out_ UINT32* calloutId
)
/* ++

   This function registers callouts and filters that intercept transport
   traffic at the following layers --

	  FWPM_LAYER_OUTBOUND_TRANSPORT_V4
	  FWPM_LAYER_OUTBOUND_TRANSPORT_V6
	  FWPM_LAYER_INBOUND_TRANSPORT_V4
	  FWPM_LAYER_INBOUND_TRANSPORT_V6

-- */
{
	NTSTATUS status = STATUS_SUCCESS;

	FWPS_CALLOUT sCallout = { 0 };
	FWPM_CALLOUT mCallout = { 0 };

	FWPM_DISPLAY_DATA displayData = { 0 };

	BOOLEAN calloutRegistered = FALSE;

	sCallout.calloutKey = *calloutKey;
	sCallout.classifyFn = TLInspectTransportClassify;
	sCallout.notifyFn = TLInspectTransportNotify;

	status = FwpsCalloutRegister(
		deviceObject,
		&sCallout,
		calloutId
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	calloutRegistered = TRUE;

	displayData.name = L"Transport Inspect Callout";
	displayData.description = L"Inspect inbound/outbound transport traffic";

	mCallout.calloutKey = *calloutKey;
	mCallout.displayData = displayData;
	mCallout.applicableLayer = *layerKey;

	status = FwpmCalloutAdd(
		gEngineHandle,
		&mCallout,
		NULL,
		NULL
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = TLInspectAddFilter(
		L"Transport Inspect Filter (Outbound)",
		L"Inspect inbound/outbound transport traffic",
		0,
		layerKey,
		calloutKey
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
Exit:

	if (!NT_SUCCESS(status))
	{
		if (calloutRegistered)
		{
			FwpsCalloutUnregisterById(*calloutId);
			*calloutId = 0;
		}
	}

	return status;
}


NTSTATUS
TLInspectRegisterCallouts(
	_Inout_ void* deviceObject
)
/* ++

   This function registers dynamic callouts and filters that intercept
   transport traffic at ALE AUTH_CONNECT/AUTH_RECV_ACCEPT and
   INBOUND/OUTBOUND transport layers.

   Callouts and filters will be removed during DriverUnload.

-- */
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_SUBLAYER TLInspectSubLayer;

	BOOLEAN engineOpened = FALSE;
	BOOLEAN inTransaction = FALSE;

	FWPM_SESSION session = { 0 };

	session.flags = FWPM_SESSION_FLAG_DYNAMIC;

	status = FwpmEngineOpen(
		NULL,
		RPC_C_AUTHN_WINNT,
		NULL,
		&session,
		&gEngineHandle
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	engineOpened = TRUE;

	status = FwpmTransactionBegin(gEngineHandle, 0);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	inTransaction = TRUE;

	RtlZeroMemory(&TLInspectSubLayer, sizeof(FWPM_SUBLAYER));

	TLInspectSubLayer.subLayerKey = TL_INSPECT_SUBLAYER;
	TLInspectSubLayer.displayData.name = L"Transport Inspect Sub-Layer";
	TLInspectSubLayer.displayData.description =
		L"Sub-Layer for use by Transport Inspect callouts";
	TLInspectSubLayer.flags = 0;
	TLInspectSubLayer.weight = 0; // must be less than the weight of 
								  // FWPM_SUBLAYER_UNIVERSAL to be
								  // compatible with Vista's IpSec
								  // implementation.

	status = FwpmSubLayerAdd(gEngineHandle, &TLInspectSubLayer, NULL);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = TLInspectRegisterTransportCallouts(
		&FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
		&TL_INSPECT_OUTBOUND_TRANSPORT_CALLOUT_V4,
		deviceObject,
		&gOutboundTlCalloutIdV4
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = TLInspectRegisterTransportCallouts(
		&FWPM_LAYER_INBOUND_TRANSPORT_V4,
		&TL_INSPECT_INBOUND_TRANSPORT_CALLOUT_V4,
		deviceObject,
		&gInboundTlCalloutIdV4
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = TLInspectRegisterTransportCallouts(
		&FWPM_LAYER_OUTBOUND_TRANSPORT_V6,
		&TL_INSPECT_OUTBOUND_TRANSPORT_CALLOUT_V6,
		deviceObject,
		&gOutboundTlCalloutIdV6
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = TLInspectRegisterTransportCallouts(
		&FWPM_LAYER_INBOUND_TRANSPORT_V6,
		&TL_INSPECT_INBOUND_TRANSPORT_CALLOUT_V6,
		deviceObject,
		&gInboundTlCalloutIdV6
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = FwpmTransactionCommit(gEngineHandle);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	inTransaction = FALSE;

Exit:

	if (!NT_SUCCESS(status))
	{
		if (inTransaction)
		{
			FwpmTransactionAbort(gEngineHandle);
			_Analysis_assume_lock_not_held_(gEngineHandle); // Potential leak if "FwpmTransactionAbort" fails
		}
		if (engineOpened)
		{
			FwpmEngineClose(gEngineHandle);
			gEngineHandle = NULL;
		}
	}

	return status;
}

void
TLInspectUnregisterCallouts(void)
{
	FwpmEngineClose(gEngineHandle);
	gEngineHandle = NULL;

	FwpsCalloutUnregisterById(gOutboundTlCalloutIdV6);
	FwpsCalloutUnregisterById(gOutboundTlCalloutIdV4);
	FwpsCalloutUnregisterById(gInboundTlCalloutIdV6);
	FwpsCalloutUnregisterById(gInboundTlCalloutIdV4);
}

_Function_class_(EVT_WDF_DRIVER_UNLOAD)
_IRQL_requires_same_
_IRQL_requires_max_(PASSIVE_LEVEL)
void
TLInspectEvtDriverUnload(
	_In_ WDFDRIVER driverObject
)
{
	UNREFERENCED_PARAMETER(driverObject);
	KeAcquireInStackQueuedSpinLock(&gPacketQueueLock, &packetQueueLockHandle);
	gDriverUnloading = TRUE;
	KeReleaseInStackQueuedSpinLock(&packetQueueLockHandle);

	if (IsListEmpty(&gPacketQueue))
	{
		KeSetEvent(
			&gWorkerEvent,
			IO_NO_INCREMENT,
			FALSE
		);
	}

	NT_ASSERT(gThreadObj != NULL);

	KeWaitForSingleObject(
		gThreadObj,
		Executive,
		KernelMode,
		FALSE,
		NULL
	);

	ObDereferenceObject(gThreadObj);
	TLInspectUnregisterCallouts();
	FwpsInjectionHandleDestroy(gInjectionHandle);
}

NTSTATUS
TLInspectInitDriverObjects(
	_Inout_ DRIVER_OBJECT* driverObject,
	_In_ const UNICODE_STRING* registryPath,
	_Out_ WDFDRIVER* pDriver,
	_Out_ WDFDEVICE* pDevice
)
{
	NTSTATUS status;
	WDF_DRIVER_CONFIG config;
	PWDFDEVICE_INIT pInit = NULL;

	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);

	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = TLInspectEvtDriverUnload;

	status = WdfDriverCreate(
		driverObject,
		registryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		pDriver
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	pInit = WdfControlDeviceInitAllocate(*pDriver, &SDDL_DEVOBJ_KERNEL_ONLY);

	if (!pInit)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	WdfDeviceInitSetDeviceType(pInit, FILE_DEVICE_NETWORK);
	WdfDeviceInitSetCharacteristics(pInit, FILE_DEVICE_SECURE_OPEN, FALSE);
	WdfDeviceInitSetCharacteristics(pInit, FILE_AUTOGENERATED_DEVICE_NAME, TRUE);

	status = WdfDeviceCreate(&pInit, WDF_NO_OBJECT_ATTRIBUTES, pDevice);
	if (!NT_SUCCESS(status))
	{
		WdfDeviceInitFree(pInit);
		goto Exit;
	}

	WdfControlFinishInitializing(*pDevice);

Exit:
	return status;
}

NTSTATUS
DriverEntry(
	DRIVER_OBJECT* driverObject,
	UNICODE_STRING* registryPath
)
{
	DbgPrint("AVNetworkDriver | DriverEntry | enter | ProcessId %p\n", PsGetCurrentProcessId());


	WDFDRIVER driver;
	WDFDEVICE device;
	NTSTATUS status;

	// Request NX Non-Paged Pool when available
	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

	status = TLInspectInitDriverObjects(
		driverObject,
		registryPath,
		&driver,
		&device
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = FwpsInjectionHandleCreate(
		AF_UNSPEC,
		FWPS_INJECTION_TYPE_TRANSPORT,
		&gInjectionHandle
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	InitializeListHead(&gPacketQueue);
	KeInitializeSpinLock(&gPacketQueueLock);

	gWdmDevice = WdfDeviceWdmGetDeviceObject(device);

	status = TLInspectRegisterCallouts(gWdmDevice);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	KeInitializeEvent(&gWorkerEvent, NotificationEvent, FALSE);

	status = PsCreateSystemThread(
		&threadHandle,
		THREAD_ALL_ACCESS,
		NULL,
		NULL,
		NULL,
		TLInspectWorker,
		NULL
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	status = ObReferenceObjectByHandle(
		threadHandle,
		0,
		NULL,
		KernelMode,
		&gThreadObj,
		NULL
	);
	NT_ASSERT(NT_SUCCESS(status));

	ZwClose(threadHandle);

Exit:

	if (!NT_SUCCESS(status))
	{
		if (gEngineHandle != NULL)
		{
			TLInspectUnregisterCallouts();
		}
		if (gInjectionHandle != NULL)
		{
			FwpsInjectionHandleDestroy(gInjectionHandle);
		}
	}

	return status;
};

void TLInspectInjectComplete(
	_Inout_ void* context,
	_Inout_ NET_BUFFER_LIST* netBufferList,
	_In_ BOOLEAN dispatchLevel
)
{
	TL_INSPECT_PENDED_PACKET* packet = context;

	UNREFERENCED_PARAMETER(dispatchLevel);

	FwpsFreeCloneNetBufferList(netBufferList, 0);

	FreePendedPacket(packet);
}

NTSTATUS
TLInspectCloneReinjectOutbound(
	_Inout_ TL_INSPECT_PENDED_PACKET* packet
)
/* ++

   This function clones the outbound net buffer list and reinject it back.

-- */
{
	NTSTATUS status = STATUS_SUCCESS;

	NET_BUFFER_LIST* clonedNetBufferList = NULL;
	FWPS_TRANSPORT_SEND_PARAMS sendArgs = { 0 };

	status = FwpsAllocateCloneNetBufferList(
		packet->netBufferList,
		NULL,
		NULL,
		0,
		&clonedNetBufferList
	);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	sendArgs.remoteAddress = (UINT8*)(&packet->remoteAddr);
	sendArgs.remoteScopeId = packet->remoteScopeId;
	sendArgs.controlData = packet->controlData;
	sendArgs.controlDataLength = packet->controlDataLength;

	//
	// Send-inject the cloned net buffer list.
	//

	status = FwpsInjectTransportSendAsync(
		gInjectionHandle,
		NULL,
		packet->endpointHandle,
		0,
		&sendArgs,
		packet->addressFamily,
		packet->compartmentId,
		clonedNetBufferList,
		TLInspectInjectComplete,
		packet
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	clonedNetBufferList = NULL; // ownership transferred to the 
								// completion function.

Exit:

	if (clonedNetBufferList != NULL)
	{
		FwpsFreeCloneNetBufferList(clonedNetBufferList, 0);
	}

	return status;
}

NTSTATUS
TLInspectCloneReinjectInbound(
	_Inout_ TL_INSPECT_PENDED_PACKET* packet
)
/* ++

   This function clones the inbound net buffer list and, if needed,
   rebuild the IP header to remove the IpSec headers and receive-injects
   the clone back to the tcpip stack.

-- */
{
	NTSTATUS status = STATUS_SUCCESS;

	NET_BUFFER_LIST* clonedNetBufferList = NULL;
	NET_BUFFER* netBuffer;
	ULONG nblOffset;
	NDIS_STATUS ndisStatus;

	//
	// For inbound net buffer list, we can assume it contains only one 
	// net buffer.
	//
	netBuffer = NET_BUFFER_LIST_FIRST_NB(packet->netBufferList);

	nblOffset = NET_BUFFER_DATA_OFFSET(netBuffer);

	//
	// The TCP/IP stack could have retreated the net buffer list by the 
	// transportHeaderSize amount; detect the condition here to avoid
	// retreating twice.
	//
	if (nblOffset != packet->nblOffset)
	{
		NT_ASSERT(packet->nblOffset - nblOffset == packet->transportHeaderSize);
		packet->transportHeaderSize = 0;
	}

	//
	// Adjust the net buffer list offset to the start of the IP header.
	//
	ndisStatus = NdisRetreatNetBufferDataStart(
		netBuffer,
		packet->ipHeaderSize + packet->transportHeaderSize,
		0,
		NULL
	);
	_Analysis_assume_(ndisStatus == NDIS_STATUS_SUCCESS);

	//
	// Note that the clone will inherit the original net buffer list's offset.
	//

	status = FwpsAllocateCloneNetBufferList(
		packet->netBufferList,
		NULL,
		NULL,
		0,
		&clonedNetBufferList
	);

	//
	// Undo the adjustment on the original net buffer list.
	//

	NdisAdvanceNetBufferDataStart(
		netBuffer,
		packet->ipHeaderSize + packet->transportHeaderSize,
		FALSE,
		NULL
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	if (packet->ipSecProtected)
	{
		//
		// When an IpSec protected packet is indicated to AUTH_RECV_ACCEPT or 
		// INBOUND_TRANSPORT layers, for performance reasons the tcpip stack
		// does not remove the AH/ESP header from the packet. And such 
		// packets cannot be recv-injected back to the stack w/o removing the
		// AH/ESP header. Therefore before re-injection we need to "re-build"
		// the cloned packet.
		// 
		status = FwpsConstructIpHeaderForTransportPacket(
			clonedNetBufferList,
			packet->ipHeaderSize,
			packet->addressFamily,
			(UINT8*)&packet->remoteAddr,
			(UINT8*)&packet->localAddr,
			packet->protocol,
			0,
			NULL,
			0,
			0,
			NULL,
			0,
			0
		);

		if (!NT_SUCCESS(status))
		{
			goto Exit;
		}
	}

	if (packet->completionContext != NULL)
	{
		NT_ASSERT(packet->type == TL_INSPECT_CONNECT_PACKET);

		FwpsCompleteOperation(
			packet->completionContext,
			clonedNetBufferList
		);

		packet->completionContext = NULL;
	}

	status = FwpsInjectTransportReceiveAsync(
		gInjectionHandle,
		NULL,
		NULL,
		0,
		packet->addressFamily,
		packet->compartmentId,
		packet->interfaceIndex,
		packet->subInterfaceIndex,
		clonedNetBufferList,
		TLInspectInjectComplete,
		packet
	);

	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}

	clonedNetBufferList = NULL; // ownership transferred to the 
								// completion function.

Exit:

	if (clonedNetBufferList != NULL)
	{
		FwpsFreeCloneNetBufferList(clonedNetBufferList, 0);
	}

	return status;
}
