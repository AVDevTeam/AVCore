#pragma once
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

#pragma warning(push)
#pragma warning(disable: 4201) //NAMELESS_STRUCT_UNION

typedef struct TL_INSPECT_PENDED_PACKET_
{
	LIST_ENTRY listEntry;

	ADDRESS_FAMILY addressFamily;
	FWP_DIRECTION  direction;

	UINT32 authConnectDecision;
	HANDLE completionContext;

	//
	// Common fields for inbound and outbound traffic.
	//
	UINT8 protocol;
	NET_BUFFER_LIST* netBufferList;
	COMPARTMENT_ID compartmentId;
	union
	{
		FWP_BYTE_ARRAY16 localAddr;
		UINT32 ipv4LocalAddr;
	};
	union
	{
		UINT16 localPort;
		UINT16 icmpType;
	};
	union
	{
		UINT16 remotePort;
		UINT16 icmpCode;
	};

	//
	// Data fields for outbound packet re-injection.
	//
	UINT64 endpointHandle;
	union
	{
		FWP_BYTE_ARRAY16 remoteAddr;
		UINT32 ipv4RemoteAddr;
	};

	SCOPE_ID remoteScopeId;
	WSACMSGHDR* controlData;
	ULONG controlDataLength;

	//
	// Data fields for inbound packet re-injection.
	//
	BOOLEAN ipSecProtected;
	ULONG nblOffset;
	UINT32 ipHeaderSize;
	UINT32 transportHeaderSize;
	IF_INDEX interfaceIndex;
	IF_INDEX subInterfaceIndex;
} TL_INSPECT_PENDED_PACKET, * PTL_INSPECT_PENDED_PACKET;

#pragma warning(pop)

//
// Pooltags used by this callout driver.
//
#define TL_INSPECT_PENDED_PACKET_POOL_TAG 'kppD'
#define TL_INSPECT_CONTROL_DATA_POOL_TAG 'dcdD'

__drv_allocatesMem(Mem)
TL_INSPECT_PENDED_PACKET*
AllocateAndInitializePendedPacket(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData
);

void
FreePendedPacket(
	_Inout_ __drv_freesMem(Mem) TL_INSPECT_PENDED_PACKET* packet
);

void
FillNetwork5Tuple(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ ADDRESS_FAMILY addressFamily,
	_Inout_ TL_INSPECT_PENDED_PACKET* packet
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
GetDeliveryInterfaceIndexesForLayer(
	_In_ UINT16 layerId,
	_Out_ UINT* interfaceIndexIndex,
	_Out_ UINT* subInterfaceIndexIndex
);

void
copyPacketInfo(
	_In_ const PTL_INSPECT_PENDED_PACKET pandedPacket,
	_Inout_ PAV_EVENT_NETWORK evenInfo,
	_Inout_ PSIZE_T umBufferSize
);