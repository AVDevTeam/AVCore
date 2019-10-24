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
#ifndef RTL_USE_AVL_TABLES
#define RTL_USE_AVL_TABLES
#endif // RTL_USE_AVL_TABLES

#define AV_VISTA    (NTDDI_VERSION >= NTDDI_VISTA)

#include <fltKernel.h>
#include "KMUMcomm.h"
#include "EventsKMStructures.h"
#include "KMEventsAPI.h"

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

EXTERN_C_START

FLT_PREOP_CALLBACK_STATUS AVEventsPreMjCreate(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

EX_CALLBACK_FUNCTION AVEventsRegistryCallback;

OB_PREOP_CALLBACK_STATUS AVObPreProcessCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pObPreOperationInfo);

OB_PREOP_CALLBACK_STATUS AVObPreThreadCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pObPreOperationInfo);

void AVCreateProcessCallback(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

void AVCreateThreadCallback(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
);

void AVLoadImageCallback(
	PUNICODE_STRING FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO ImageInfo
);

EXTERN_C_END

// KMEventsAPI init deinit imports.
DECLSPEC_IMPORT NTSTATUS AVCommInit(PFLT_FILTER Filter);
DECLSPEC_IMPORT void AVCommStop(VOID);

