/*++
Module Name:
    KMUMcomm.h
Abstract:
    This header file defines the common data structure used by kernel and user.
Environment:
    User mode
    Kernel mode
--*/

#ifndef __KMUMCOMM_H__
#define __KMUMCOMM_H__
#pragma once

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

//  Name of AV filter server ports
#define AV_SCAN_PORT_NAME                    L"\\AVCoreEventsPort"

//  Message type enumeration, please see AV_SCANNER_NOTIFICATION below
typedef enum _AV_MESSAGE_TYPE
{
	AvMsgEvent
} AV_MESSAGE_TYPE;

typedef enum _AV_EVENT_TYPE
{
	AvFileCreate, // (IRP_MJ_CREATE)
	AvProcessHandleCreate, // (ObFilter process handle create)
	AvProcessHandleDublicate, // (ObFilter process handle dublicate)
	AvThreadHandleCreate, // (ObFilter thread handle create)
	AvThreadHandleDublicate, // (ObFilter thread handle dublicate)
	AvProcessCreate, // (PsSetCreateProcessNotifyRoutineEx[2]) 
	AvProcessExit, // (PsSetCreateProcessNotifyRoutineEx[2]) 
	AvThreadCreate, // (PsSetThreadeCreateNotifyRoutine)
	AvThreadExit, // (PsSetThreadeCreateNotifyRoutine)
	AvImageLoad, // (PsSetImageNotifyRoutine[Ex])
	AvRegCreateKey, // (RegNtPreCreateKey[Ex])
	AvRegOpenKey, // (RegNtPreOpenKey[Ex])
} AV_EVENT_TYPE;

//  Event stucture: Kernel -> User Message
typedef struct _AV_MESSAGE
{
    //  Message type
    AV_MESSAGE_TYPE MessageType;

	AV_EVENT_TYPE EventType;

	ULONG EventBufferLength;
	PVOID EventBuffer;

} AV_MESSAGE, *PAV_MESSAGE;

//  Connection type enumeration. It would be mainly used in connection context.
typedef enum _AV_CONNECTION_TYPE 
{
    AvConnectForScan = 1,
} AV_CONNECTION_TYPE, *PAV_CONNECTION_TYPE;

//  Connection context. It will be passed through FilterConnectCommunicationPort(...)
typedef struct _AV_CONNECTION_CONTEXT 
{
	AV_CONNECTION_TYPE   Type;
	HANDLE ProcessID; // is used to pass PID of AVCore service.

} AV_CONNECTION_CONTEXT, *PAV_CONNECTION_CONTEXT;

// This structer is used to pass
// information about APC payload
// from UM to KM.
typedef struct _APC_INFO
{
	int PID;
	int TID;
	int apcBufferSize;
	void* apcBuffer;
} APC_INFO, * PAPC_INFO;

#endif


