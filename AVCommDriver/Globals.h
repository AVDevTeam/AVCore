#pragma once
#include <fltKernel.h>

typedef struct _AV_COMM_GLOBAL_DATA
{
	//  The global FLT_FILTER pointer. Many API needs this, such as 
	//  FltAllocateContext(...)
	PFLT_FILTER Filter;

	PFLT_PORT EventsClientPort;

	//  Server-side communicate ports.
	PFLT_PORT EventsServerPort;

	HANDLE AVCoreServicePID;
	HANDLE AVCoreServiceHandle;
	PEPROCESS AVCoreServiceEprocess;

	// Global variables that are used to store
	// parameters of memmoveUM before the stack switch.
	VOID* Source;
	VOID* Target;
	SIZE_T Size;

} AV_COMM_GLOBAL_DATA, * PAV_COMM_GLOBAL_DATA;