#include "AVNetworkDriver.h"

__drv_allocatesMem(Mem)
TL_INSPECT_PENDED_PACKET*
AllocateAndInitializePendedPacket(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData
)
{
	TL_INSPECT_PENDED_PACKET* pendedPacket;	

	pendedPacket = ExAllocatePoolWithTag(
		NonPagedPool,
		sizeof(TL_INSPECT_PENDED_PACKET),
		TL_INSPECT_PENDED_PACKET_POOL_TAG
	);

	if (pendedPacket == NULL)
	{
		return NULL;
	}

	RtlZeroMemory(pendedPacket, sizeof(TL_INSPECT_PENDED_PACKET));

	pendedPacket->direction = GetPacketDirectionForLayer(inFixedValues->layerId);
	pendedPacket->addressFamily = GetAddressFamilyForLayer(inFixedValues->layerId);

	FillNetwork5Tuple(
		inFixedValues,
		pendedPacket->addressFamily,
		pendedPacket
	);

	if (layerData != NULL)
	{
		pendedPacket->netBufferList = layerData;

		//
		// Reference the net buffer list to make it accessible outside of
		// classifyFn.
		//
		FwpsReferenceNetBufferList(pendedPacket->netBufferList, TRUE);
	}

	NT_ASSERT(FWPS_IS_METADATA_FIELD_PRESENT(inMetaValues,
		FWPS_METADATA_FIELD_COMPARTMENT_ID));
	pendedPacket->compartmentId = inMetaValues->compartmentId;

	if ((pendedPacket->direction == FWP_DIRECTION_OUTBOUND) &&
		(layerData != NULL))
	{
		NT_ASSERT(FWPS_IS_METADATA_FIELD_PRESENT(
			inMetaValues,
			FWPS_METADATA_FIELD_TRANSPORT_ENDPOINT_HANDLE));
		pendedPacket->endpointHandle = inMetaValues->transportEndpointHandle;

		pendedPacket->remoteScopeId = inMetaValues->remoteScopeId;

		if (FWPS_IS_METADATA_FIELD_PRESENT(
			inMetaValues,
			FWPS_METADATA_FIELD_TRANSPORT_CONTROL_DATA))
		{
			NT_ASSERT(inMetaValues->controlDataLength > 0);

			pendedPacket->controlData = ExAllocatePoolWithTag(
				NonPagedPool,
				inMetaValues->controlDataLength,
				TL_INSPECT_CONTROL_DATA_POOL_TAG
			);
			if (pendedPacket->controlData == NULL)
			{
				goto Exit;
			}

			RtlCopyMemory(
				pendedPacket->controlData,
				inMetaValues->controlData,
				inMetaValues->controlDataLength
			);

			pendedPacket->controlDataLength = inMetaValues->controlDataLength;
		}
	}
	else if (pendedPacket->direction == FWP_DIRECTION_INBOUND)
	{
		UINT interfaceIndexIndex = 0;
		UINT subInterfaceIndexIndex = 0;

		GetDeliveryInterfaceIndexesForLayer(
			inFixedValues->layerId,
			&interfaceIndexIndex,
			&subInterfaceIndexIndex
		);

		pendedPacket->interfaceIndex =
			inFixedValues->incomingValue[interfaceIndexIndex].value.uint32;
		pendedPacket->subInterfaceIndex =
			inFixedValues->incomingValue[subInterfaceIndexIndex].value.uint32;

		NT_ASSERT(FWPS_IS_METADATA_FIELD_PRESENT(
			inMetaValues,
			FWPS_METADATA_FIELD_IP_HEADER_SIZE));
		NT_ASSERT(FWPS_IS_METADATA_FIELD_PRESENT(
			inMetaValues,
			FWPS_METADATA_FIELD_TRANSPORT_HEADER_SIZE));
		pendedPacket->ipHeaderSize = inMetaValues->ipHeaderSize;
		pendedPacket->transportHeaderSize = inMetaValues->transportHeaderSize;

		if (pendedPacket->netBufferList != NULL)
		{
			FWPS_PACKET_LIST_INFORMATION packetInfo = { 0 };
			FwpsGetPacketListSecurityInformation(
				pendedPacket->netBufferList,
				FWPS_PACKET_LIST_INFORMATION_QUERY_IPSEC |
				FWPS_PACKET_LIST_INFORMATION_QUERY_INBOUND,
				&packetInfo
			);

			pendedPacket->ipSecProtected =
				(BOOLEAN)packetInfo.ipsecInformation.inbound.isSecure;

			pendedPacket->nblOffset =
				NET_BUFFER_DATA_OFFSET(\
					NET_BUFFER_LIST_FIRST_NB(pendedPacket->netBufferList));
		}
	}

	return pendedPacket;

Exit:

	if (pendedPacket != NULL)
	{
		FreePendedPacket(pendedPacket);
	}

	return NULL;
}

void
FreePendedPacket(
	_Inout_ __drv_freesMem(Mem) TL_INSPECT_PENDED_PACKET* packet
)
{
	if (packet->netBufferList != NULL)
	{
		FwpsDereferenceNetBufferList(packet->netBufferList, FALSE);
	}
	if (packet->controlData != NULL)
	{
		ExFreePoolWithTag(packet->controlData, TL_INSPECT_CONTROL_DATA_POOL_TAG);
	}
	if (packet->completionContext != NULL)
	{
		FwpsCompleteOperation(packet->completionContext, NULL);
	}
	ExFreePoolWithTag(packet, TL_INSPECT_PENDED_PACKET_POOL_TAG);
}

void
GetNetwork5TupleIndexesForLayer(
	_In_ UINT16 layerId,
	_Out_ UINT* localAddressIndex,
	_Out_ UINT* remoteAddressIndex,
	_Out_ UINT* localPortIndex,
	_Out_ UINT* remotePortIndex,
	_Out_ UINT* protocolIndex
)
{
	switch (layerId)
	{
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
		*localAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS;
		*remoteAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS;
		*localPortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT;
		*remotePortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_PORT;
		*protocolIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_PROTOCOL;
		break;
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V6:
		*localAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_LOCAL_ADDRESS;
		*remoteAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_REMOTE_ADDRESS;
		*localPortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_LOCAL_PORT;
		*remotePortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_REMOTE_PORT;
		*protocolIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_PROTOCOL;
		break;
	case FWPS_LAYER_INBOUND_TRANSPORT_V4:
		*localAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS;
		*remoteAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS;
		*localPortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT;
		*remotePortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_PORT;
		*protocolIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_PROTOCOL;
		break;
	case FWPS_LAYER_INBOUND_TRANSPORT_V6:
		*localAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_LOCAL_ADDRESS;
		*remoteAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_REMOTE_ADDRESS;
		*localPortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_LOCAL_PORT;
		*remotePortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_REMOTE_PORT;
		*protocolIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_PROTOCOL;
		break;
	default:
		*localAddressIndex = UINT_MAX;
		*remoteAddressIndex = UINT_MAX;
		*localPortIndex = UINT_MAX;
		*remotePortIndex = UINT_MAX;
		*protocolIndex = UINT_MAX;
		NT_ASSERT(0);
	}
}

FWP_DIRECTION GetPacketDirectionForLayer(
	_In_ UINT16 layerId
)
{
	FWP_DIRECTION direction;

	switch (layerId)
	{
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V6:
		direction = FWP_DIRECTION_OUTBOUND;
		break;
	case FWPS_LAYER_INBOUND_TRANSPORT_V4:
	case FWPS_LAYER_INBOUND_TRANSPORT_V6:
		direction = FWP_DIRECTION_INBOUND;
		break;
	default:
		direction = FWP_DIRECTION_MAX;
		NT_ASSERT(0);
	}

	return direction;
}

ADDRESS_FAMILY GetAddressFamilyForLayer(
	_In_ UINT16 layerId
)
{
	ADDRESS_FAMILY addressFamily;

	switch (layerId)
	{
	case FWPS_LAYER_ALE_AUTH_CONNECT_V4:
	case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4:
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
	case FWPS_LAYER_INBOUND_TRANSPORT_V4:
		addressFamily = AF_INET;
		break;
	case FWPS_LAYER_ALE_AUTH_CONNECT_V6:
	case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V6:
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V6:
	case FWPS_LAYER_INBOUND_TRANSPORT_V6:
		addressFamily = AF_INET6;
		break;
	default:
		addressFamily = AF_UNSPEC;
		NT_ASSERT(0);
	}

	return addressFamily;
}

void
FillNetwork5Tuple(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ ADDRESS_FAMILY addressFamily,
	_Inout_ TL_INSPECT_PENDED_PACKET* packet
)
{
	UINT localAddrIndex;
	UINT remoteAddrIndex;
	UINT localPortIndex;
	UINT remotePortIndex;
	UINT protocolIndex;

	GetNetwork5TupleIndexesForLayer(
		inFixedValues->layerId,
		&localAddrIndex,
		&remoteAddrIndex,
		&localPortIndex,
		&remotePortIndex,
		&protocolIndex
	);

	if (addressFamily == AF_INET)
	{
		packet->ipv4LocalAddr =
			RtlUlongByteSwap( /* host-order -> network-order conversion */
				inFixedValues->incomingValue[localAddrIndex].value.uint32
			);
		packet->ipv4RemoteAddr =
			RtlUlongByteSwap( /* host-order -> network-order conversion */
				inFixedValues->incomingValue[remoteAddrIndex].value.uint32
			);
	}
	else
	{
		RtlCopyMemory(
			(UINT8*)&packet->localAddr,
			inFixedValues->incomingValue[localAddrIndex].value.byteArray16,
			sizeof(FWP_BYTE_ARRAY16)
		);
		RtlCopyMemory(
			(UINT8*)&packet->remoteAddr,
			inFixedValues->incomingValue[remoteAddrIndex].value.byteArray16,
			sizeof(FWP_BYTE_ARRAY16)
		);
	}

	packet->localPort =
		RtlUshortByteSwap(
			inFixedValues->incomingValue[localPortIndex].value.uint16
		);
	packet->remotePort =
		RtlUshortByteSwap(
			inFixedValues->incomingValue[remotePortIndex].value.uint16
		);

	packet->protocol = inFixedValues->incomingValue[protocolIndex].value.uint8;

	return;
}

void
GetDeliveryInterfaceIndexesForLayer(
	_In_ UINT16 layerId,
	_Out_ UINT* interfaceIndexIndex,
	_Out_ UINT* subInterfaceIndexIndex
)
{
	*interfaceIndexIndex = 0;

	*subInterfaceIndexIndex = 0;

	switch (layerId)
	{
	case FWPS_LAYER_ALE_AUTH_CONNECT_V4:
		*interfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_CONNECT_V4_INTERFACE_INDEX;
		*subInterfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_CONNECT_V4_SUB_INTERFACE_INDEX;
		break;
	case FWPS_LAYER_ALE_AUTH_CONNECT_V6:
		*interfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_CONNECT_V6_INTERFACE_INDEX;
		*subInterfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_CONNECT_V6_SUB_INTERFACE_INDEX;
		break;
	case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4:
		*interfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_INTERFACE_INDEX;
		*subInterfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_SUB_INTERFACE_INDEX;
		break;
	case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V6:
		*interfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_INTERFACE_INDEX;
		*subInterfaceIndexIndex =
			FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_SUB_INTERFACE_INDEX;
		break;
	case FWPS_LAYER_INBOUND_TRANSPORT_V4:
		*interfaceIndexIndex =
			FWPS_FIELD_INBOUND_TRANSPORT_V4_INTERFACE_INDEX;
		*subInterfaceIndexIndex =
			FWPS_FIELD_INBOUND_TRANSPORT_V4_SUB_INTERFACE_INDEX;
		break;
	case FWPS_LAYER_INBOUND_TRANSPORT_V6:
		*interfaceIndexIndex =
			FWPS_FIELD_INBOUND_TRANSPORT_V6_INTERFACE_INDEX;
		*subInterfaceIndexIndex =
			FWPS_FIELD_INBOUND_TRANSPORT_V6_SUB_INTERFACE_INDEX;
		break;
	default:
		NT_ASSERT(0);
		break;
	}
}

void
copyPacketInfo(
	_In_ const PTL_INSPECT_PENDED_PACKET pandedPacket,
	_Inout_ PAV_EVENT_NETWORK eventInfo,
	_Inout_ PSIZE_T umBufferSize
)
{
	memset(eventInfo, 0, sizeof(AV_EVENT_NETWORK));

	if (pandedPacket->direction == FWP_DIRECTION_OUTBOUND) {
		if (pandedPacket->addressFamily == AF_INET)
		{
			eventInfo->isIPV6 = 0;
			*((INT32*)eventInfo->localAddress) = pandedPacket->ipv4LocalAddr;
			*((INT32*)eventInfo->remoteAddress) = pandedPacket->ipv4RemoteAddr;
		}
		else
		{
			eventInfo->isIPV6 = 1;
			RtlCopyMemory(
				eventInfo->localAddress,
				&pandedPacket->localAddr,
				sizeof(FWP_BYTE_ARRAY16)
			);
			RtlCopyMemory(
				eventInfo->remoteAddress,
				&pandedPacket->remoteAddr,
				sizeof(FWP_BYTE_ARRAY16)
			);
		}
		eventInfo->localPort = pandedPacket->localPort;
		eventInfo->remotePort = pandedPacket->remotePort;
	}
	else {
		if (pandedPacket->addressFamily == AF_INET)
		{
			eventInfo->isIPV6 = 0;
			*((INT32*)eventInfo->localAddress) = pandedPacket->ipv4RemoteAddr;
			*((INT32*)eventInfo->remoteAddress) = pandedPacket->ipv4LocalAddr;
		}
		else
		{
			eventInfo->isIPV6 = 1;
			RtlCopyMemory(
				eventInfo->localAddress,
				&pandedPacket->remoteAddr,
				sizeof(FWP_BYTE_ARRAY16)
			);
			RtlCopyMemory(
				eventInfo->remoteAddress,
				&pandedPacket->localAddr,
				sizeof(FWP_BYTE_ARRAY16)
			);
		}
		eventInfo->localPort = pandedPacket->remotePort;
		eventInfo->remotePort = pandedPacket->localPort;
	}
	

	PNET_BUFFER netBuffer = pandedPacket->netBufferList->FirstNetBuffer;
	PMDL pMdl = netBuffer->CurrentMdl;
	SIZE_T totalLength = netBuffer->DataLength;
	ULONG offset = netBuffer->CurrentMdlOffset;
	SIZE_T bufferLength = 0;
	PCHAR buffer;
	PCHAR data = ExAllocatePool(PagedPool, totalLength);

	NdisQueryMdl(pMdl, &buffer, &bufferLength, NormalPagePriority | MdlMappingNoExecute);
	if (buffer == NULL) {
		bufferLength = 0;
	}
	bufferLength -= offset;
	RtlCopyMemory(data, buffer + offset, bufferLength);
	SIZE_T recived = bufferLength;

	for (;pMdl != NULL && recived < totalLength; pMdl = pMdl->Next) {
		NdisQueryMdl(pMdl, &buffer, &bufferLength, NormalPagePriority | MdlMappingNoExecute);
		if (buffer == NULL || bufferLength == 0) { break; }
		if (recived + bufferLength > totalLength) bufferLength = totalLength - recived;
		RtlCopyMemory(data + recived, buffer, bufferLength);
		recived += bufferLength;
	} 
	NTSTATUS status = AVCommCreateBuffer(data, totalLength, &eventInfo->data, umBufferSize);
	if (status != STATUS_SUCCESS)
	{
		eventInfo->data = NULL;
	}
	eventInfo->dataLength = (unsigned long long)totalLength;
	DbgPrint("AVNetworkDriver | copyPacketInfo | umBufferSize %ull, totalLength %ull, eventInfo->dataLength %ull, data %p, eventInfo->data %p\n", umBufferSize, totalLength, eventInfo->dataLength, data, eventInfo->data);
	ExFreePool(data);
	return;
}
