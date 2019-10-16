#pragma once
#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

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
	SIZE_T Size;

#if DBG
	// Field to control nature of debug output
	ULONG DebugLevel;
#endif
	//  A flag that indicating that the filter is being unloaded.
	BOOLEAN  Unloading;

} AV_SCANNER_GLOBAL_DATA, * PAV_SCANNER_GLOBAL_DATA;

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