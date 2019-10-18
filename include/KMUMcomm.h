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

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

//  Name of AV filter server ports
#define AV_SCAN_PORT_NAME                    L"\\AVCoreEventsPort"
#define AV_ABORT_PORT_NAME                   L"\\AVCoreAbortPort"

//  Message type enumeration, please see AV_SCANNER_NOTIFICATION below
typedef enum _AV_MESSAGE_TYPE
{
	AvMsgEvent,
    AvMsgFilterUnloading
} AV_MESSAGE_TYPE;

typedef enum _AV_EVENT_TYPE
{
	AvFileCreate,
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
    AvConnectForAbort,
} AV_CONNECTION_TYPE, *PAV_CONNECTION_TYPE;

//  Connection context. It will be passed through FilterConnectCommunicationPort(...)
typedef struct _AV_CONNECTION_CONTEXT 
{
	AV_CONNECTION_TYPE   Type;
	HANDLE ProcessID; // is used to pass PID of AVCore service.

} AV_CONNECTION_CONTEXT, *PAV_CONNECTION_CONTEXT;


#endif


