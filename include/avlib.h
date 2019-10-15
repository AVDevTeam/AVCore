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

//
//  Message type enumeration, please see AV_SCANNER_NOTIFICATION below
//

typedef enum _AVSCAN_MESSAGE {

	AvMsgEvent,
    AvMsgFilterUnloading

} AVSCAN_MESSAGE;

typedef enum _AVSCAN_REASON {
    AvScanOnOpen,
    AvScanOnCleanup

} AVSCAN_REASON;

typedef enum _AVSCAN_RESULT {

    AvScanResultUndetermined,
    AvScanResultInfected,
    AvScanResultClean

} AVSCAN_RESULT;

//
//  Message: Kernel -> User Message
//

typedef struct _SCANNER_NOTIFICATION {

    //
    //  Message type
    //
    
    AVSCAN_MESSAGE Message;

    //
    //  Reason
    //

    AVSCAN_REASON  Reason;
    
    //
    //  Scan identifier.
    //  This argument will be checked in message notificaiton callback.
    //
    
    LONGLONG  ScanId;
    
    //
    //  Scan thread id. This id will be used in cancel message passing.
    //  So that we will know which scan thread to cancel.
    //
    
    ULONG  ScanThreadId;

	ULONG BufferLength;
	WCHAR Buffer[2048]; // test buf
    
} AV_SCANNER_NOTIFICATION, *PAV_SCANNER_NOTIFICATION;

//
//  Connection type enumeration. It would be mainly used in connection context.
//

typedef enum _AVSCAN_CONNECTION_TYPE 
{
    AvConnectForScan = 1,
    AvConnectForAbort,
} AVSCAN_CONNECTION_TYPE, *PAVSCAN_CONNECTION_TYPE;

//
//  Connection context. It will be passed through FilterConnectCommunicationPort(...)
//

typedef struct _AV_CONNECTION_CONTEXT {

    AVSCAN_CONNECTION_TYPE   Type;

} AV_CONNECTION_CONTEXT, *PAV_CONNECTION_CONTEXT;


#endif

