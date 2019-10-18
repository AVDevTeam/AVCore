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
#include "KMUMcomm.h"
#include "EventsKM.h"
#include "EventsAPI.h"

#define AV_CONNECTION_CTX_TAG                'cCvA'

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

#pragma region EventsAPI import
DECLSPEC_IMPORT NTSTATUS AVCommConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
);

DECLSPEC_IMPORT VOID AVCommDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
);

DECLSPEC_IMPORT NTSTATUS AVCommPrepareServerPort(
	_In_  PSECURITY_DESCRIPTOR SecurityDescriptor,
	_In_  AV_CONNECTION_TYPE  ConnectionType
);

DECLSPEC_IMPORT NTSTATUS AVCommSendUnloadingToUser(
	VOID
);

DECLSPEC_IMPORT NTSTATUS memmoveUM(void*, PSIZE_T, void**);

DECLSPEC_IMPORT void setFilter(PFLT_FILTER);

DECLSPEC_IMPORT void closeCommunicationPorts(VOID);

DECLSPEC_IMPORT HANDLE getAVCorePID(VOID);

DECLSPEC_IMPORT HANDLE getAVCoreHandle(VOID);

DECLSPEC_IMPORT NTSTATUS sendEvent(void*, int, PAV_EVENT_RESPONSE, PULONG);
#pragma endregion EventsAPI import

#endif

