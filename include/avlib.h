/*++

Copyright (c) 2011  Microsoft Corporation

Module Name:

    avlib.h

Abstract:

    This header file defines the common data structure used by kernel and user.

Environment:

    User mode
    Kernel mode

--*/

#ifndef __AVLIB_H__
#define __AVLIB_H__

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

//
//  Name of AV filter server ports
//

#define AV_SCAN_PORT_NAME                    L"\\AVCoreEventsPort"
#define AV_ABORT_PORT_NAME                   L"\\AVCoreAbortPort"


//
//  Definition of invalide section handle for data scan
//

#define AV_INVALID_SECTION_HANDLE   ((HANDLE)((LONG_PTR)(-1)))

//  Message type enumeration, please see AV_SCANNER_NOTIFICATION below
typedef enum _AV_MESSAGE_TYPE
{
	AvMsgEvent,
    AvMsgFilterUnloading

} AV_MESSAGE_TYPE;

//  Event stucture: Kernel -> User Message
typedef struct _AV_EVENT 
{
    //  Message type
    AV_MESSAGE_TYPE MessageType;

	ULONG EventBufferLength;
	PVOID EventBuffer;
    
} AV_EVENT, *PAV_EVENT;

//
//  Connection type enumeration. It would be mainly used in connection context.
//

typedef enum _AV_CONNECTION_TYPE 
{
    AvConnectForScan = 1,
    AvConnectForAbort,
} AV_CONNECTION_TYPE, *PAV_CONNECTION_TYPE;

//
//  Connection context. It will be passed through FilterConnectCommunicationPort(...)
//

typedef struct _AV_CONNECTION_CONTEXT 
{
	AV_CONNECTION_TYPE   Type;
	HANDLE ProcessID;

} AV_CONNECTION_CONTEXT, *PAV_CONNECTION_CONTEXT;


#endif

