/*++

Copyright (c) 1989-2011  Microsoft Corporation

Module Name:

	avscan.h

Abstract:

	Header file which contains the structures, type definitions,
	constants, global variables and function prototypes that are
	only visible within the kernel. Mainly used by avscan module.

Environment:

	Kernel mode

--*/
#ifndef __AVSCAN_H__
#define __AVSCAN_H__

#ifndef RTL_USE_AVL_TABLES
#define RTL_USE_AVL_TABLES
#endif // RTL_USE_AVL_TABLES

#define AV_VISTA    (NTDDI_VERSION >= NTDDI_VISTA)

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "avlib.h"

#define AV_CONNECTION_CTX_TAG                'cCvA'


#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

//  The global variable
typedef struct _AV_CORE_GLOBAL_DATA 
{
	//  The global FLT_FILTER pointer. Many API needs this, such as 
	//  FltAllocateContext(...)
	PFLT_FILTER Filter;

	//  Server-side communicate ports.
	PFLT_PORT EventsServerPort;
	PFLT_PORT AbortServerPort;

	//  The scan client ports.
	//  These ports are assigned at AvConnectNotifyCallback and cleaned at AvDisconnectNotifyCallback
	//
	//  ScanClientPort is the connection port regarding the scan message.
	//  AbortClientPort is the connection port regarding the abort message.
	//  QueryClient is the connection port regarding the query command.
	PFLT_PORT ScanClientPort;
	PFLT_PORT AbortClientPort;

	HANDLE AVCoreServiceHandle;
	PEPROCESS AVCoreServiceEprocess;

	VOID* Source;
	VOID* Target;
	ULONG Size;

#if DBG
	// Field to control nature of debug output
	ULONG DebugLevel;
#endif
	//  A flag that indicating that the filter is being unloaded.
	BOOLEAN  Unloading;

} AV_SCANNER_GLOBAL_DATA, * PAV_SCANNER_GLOBAL_DATA;

AV_SCANNER_GLOBAL_DATA Globals;

#if DBG

//  Debugging level flags.
#define AVDBG_TRACE_ROUTINES            0x00000001
#define AVDBG_TRACE_OPERATION_STATUS    0x00000002
#define AVDBG_TRACE_DEBUG               0x00000004
#define AVDBG_TRACE_ERROR               0x00000008

#define AV_DBG_PRINT( _dbgLevel, _string )          \
    if(FlagOn(Globals.DebugLevel,(_dbgLevel))) {    \
        DbgPrint _string;                           \
    }

#else

#define AV_DBG_PRINT(_dbgLevel, _string)            {NOTHING;}

#endif


NTSTATUS AVEventsDriverConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
);

VOID AVEventsDriverDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS AVEventsDriverPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AV_CONNECTION_TYPE  ConnectionType
);

NTSTATUS AVEventsDriverSendUnloadingToUser(
	VOID
);

#endif

