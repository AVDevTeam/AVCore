/**
@file
@brief AVEventsDriver implements file system, registry and objects events filtering logic.
*/

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

#include <fltKernel.h>
#include "KMUMcomm.h"
#include "EventsKMStructures.h"
#include "KMEventsAPI.h"

#define AV_CONNECTION_CTX_TAG 'cCvA'

EXTERN_C_START

FLT_PREOP_CALLBACK_STATUS AVEventsPreMjCreate(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

EX_CALLBACK_FUNCTION AVEventsRegistryCallback;

OB_PREOP_CALLBACK_STATUS AVObPreProcessCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pObPreOperationInfo
);

OB_PREOP_CALLBACK_STATUS AVObPreThreadCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pObPreOperationInfo
);

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

/**
Exports from AVCommDriver.
*/
DECLSPEC_IMPORT NTSTATUS AVCommInit(PFLT_FILTER Filter);
DECLSPEC_IMPORT void AVCommStop(VOID);