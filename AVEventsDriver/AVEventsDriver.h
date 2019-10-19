/*++
Module Name:
	AVEventsDriver.h
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

// Exports from AVCommDriver
#pragma region EventsAPI import

DECLSPEC_IMPORT NTSTATUS AVCommInit(PFLT_FILTER Filter);
DECLSPEC_IMPORT void AVCommStop(VOID);
DECLSPEC_IMPORT NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, PVOID* outUmBuffer, PSIZE_T outUmSize);
DECLSPEC_IMPORT NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);
DECLSPEC_IMPORT NTSTATUS AVCommSendEvent(void*, int, PAV_EVENT_RESPONSE, PULONG);
DECLSPEC_IMPORT HANDLE AVCommGetUmPID(VOID);

#pragma endregion EventsAPI import

#endif

