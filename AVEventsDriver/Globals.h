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

	HANDLE AVCoreServicePID;
	HANDLE AVCoreServiceHandle;
	PEPROCESS AVCoreServiceEprocess;

	// Global variables that are used to store
	// parameters of memmoveUM before the stack switch.
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

