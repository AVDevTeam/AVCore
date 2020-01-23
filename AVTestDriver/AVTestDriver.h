#pragma once

#define AV_CONNECTION_CTX_TAG 'cCvA'


#include <fltKernel.h>

#include <ntddk.h>
#include <wdf.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

#include <fwpmk.h>

#define INITGUID
#include <guiddef.h>

#include <limits.h>
#include <ws2ipdef.h>
#include <in6addr.h>
#include <ip2string.h>

#include "KMUMcomm.h"
#include "EventsKMStructures.h"

NTSTATUS AVCommInit(PFLT_FILTER Filter);
void AVCommStop(VOID);
NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, void** outUmBuffer, PSIZE_T outUmSize);
NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);
NTSTATUS AVCommSendEvent(AV_EVENT_TYPE, void*, int, PAV_EVENT_RESPONSE, PULONG);
UCHAR AVCommIsInitialized(VOID);



// bb6e405b-19f4-4ff3-b501-1a3dc01aae01
DEFINE_GUID(
	TL_INSPECT_OUTBOUND_TRANSPORT_CALLOUT_V4,
	0xbb6e405b,
	0x19f4,
	0x4ff3,
	0xb5, 0x01, 0x1a, 0x3d, 0xc0, 0x1a, 0xae, 0x01
);
// cabf7559-7c60-46c8-9d3b-2155ad5cf83f
DEFINE_GUID(
	TL_INSPECT_OUTBOUND_TRANSPORT_CALLOUT_V6,
	0xcabf7559,
	0x7c60,
	0x46c8,
	0x9d, 0x3b, 0x21, 0x55, 0xad, 0x5c, 0xf8, 0x3f
);
// 07248379-248b-4e49-bf07-24d99d52f8d0
DEFINE_GUID(
	TL_INSPECT_INBOUND_TRANSPORT_CALLOUT_V4,
	0x07248379,
	0x248b,
	0x4e49,
	0xbf, 0x07, 0x24, 0xd9, 0x9d, 0x52, 0xf8, 0xd0
);
// 6d126434-ed67-4285-925c-cb29282e0e06
DEFINE_GUID(
	TL_INSPECT_INBOUND_TRANSPORT_CALLOUT_V6,
	0x6d126434,
	0xed67,
	0x4285,
	0x92, 0x5c, 0xcb, 0x29, 0x28, 0x2e, 0x0e, 0x06
);
// 2e207682-d95f-4525-b966-969f26587f03
DEFINE_GUID(
	TL_INSPECT_SUBLAYER,
	0x2e207682,
	0xd95f,
	0x4525,
	0xb9, 0x66, 0x96, 0x9f, 0x26, 0x58, 0x7f, 0x03
);

void
FillNetwork5Tuple(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const PNET_BUFFER_LIST layerData,
	_In_ ADDRESS_FAMILY addressFamily,
	_Inout_ PAV_EVENT_NETWORK packet
);

void
GetNetwork5TupleIndexesForLayer(
	_In_ UINT16 layerId,
	_Out_ UINT* localAddressIndex,
	_Out_ UINT* remoteAddressIndex,
	_Out_ UINT* localPortIndex,
	_Out_ UINT* remotePortIndex,
	_Out_ UINT* protocolIndex
);

FWP_DIRECTION GetPacketDirectionForLayer(
	_In_ UINT16 layerId
);

ADDRESS_FAMILY GetAddressFamilyForLayer(
	_In_ UINT16 layerId
);


void
FillNetwork5Tuple(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const PNET_BUFFER_LIST layerData,
	_In_ ADDRESS_FAMILY addressFamily,
	_Inout_ PAV_EVENT_NETWORK packet
)
{
	UNREFERENCED_PARAMETER(layerData);

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
		packet->isIPV6 = 0;
		*((INT32*)packet->localAddress) = RtlUlongByteSwap(inFixedValues->incomingValue[localAddrIndex].value.uint32);
		*((INT32*)packet->remoteAddress) = RtlUlongByteSwap(inFixedValues->incomingValue[remoteAddrIndex].value.uint32);
	}
	else
	{
		packet->isIPV6 = 1;
		RtlCopyMemory(
			packet->localAddress,
			inFixedValues->incomingValue[localAddrIndex].value.byteArray16,
			sizeof(FWP_BYTE_ARRAY16)
		);
		RtlCopyMemory(
			packet->remoteAddress,
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

	//packet->data = layerData;
	return;
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