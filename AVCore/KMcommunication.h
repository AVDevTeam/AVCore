#pragma once
/*++

Copyright (c) 2011  Microsoft Corporation

Module Name:

	userscan.h

Abstract:

	The scanning module. This module defines the thread contexts,
	and user scan contexts, and the definitions of functions.

Environment:

	User mode

--*/

#ifndef __USERSCAN_H__
#define __USERSCAN_H__

#include <windows.h>
#include <fltUser.h>
#include "avlib.h"

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif

typedef struct _SCANNER_THREAD_CONTEXT {

	//
	//   Threand Handle
	//

	HANDLE   Handle;

	//
	//   Threand Id
	//

	DWORD   ThreadId;

} SCANNER_THREAD_CONTEXT, * PSCANNER_THREAD_CONTEXT;

typedef struct _USER_SCAN_CONTEXT {

	//
	//  Scan thread contexts
	//

	PSCANNER_THREAD_CONTEXT  ScanThreadCtxes;

	//
	//  The abortion thread handle
	//

	HANDLE   AbortThreadHandle;

	//
	//  Finalize flag, set at UserScanFinalize(...)
	//

	BOOLEAN  Finalized;

	//
	//  Handle of connection port to the filter.
	//

	HANDLE   ConnectionPort;

	//
	//  Completion port for asynchronous message passing
	//

	HANDLE   Completion;

} USER_SCAN_CONTEXT, * PUSER_SCAN_CONTEXT;

HRESULT KMCommInit(
	_Inout_  PUSER_SCAN_CONTEXT Context
);

HRESULT KMCommFinalize(
	_In_  PUSER_SCAN_CONTEXT Context
);

#endif

