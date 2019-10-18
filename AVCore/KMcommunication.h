#pragma once
/*++
Module Name:
	KMcommunication.h
Abstract:
	TODO! DOCUMENTATION!
	The scanning module. This module defines the thread contexts,
	and user scan contexts, and the definitions of functions.
Environment:
	User mode
--*/

#ifndef __USERSCAN_H__
#define __USERSCAN_H__

#include <windows.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "EventsKM.h"
#include "EventsUM.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif

typedef struct _LISTENER_THREAD_CONTEXT
{
	//   Threand Handle
	HANDLE   Handle;

	//   Threand Id
	DWORD   ThreadId;

} LISTENER_THREAD_CONTEXT, * PLISTENER_THREAD_CONTEXT;

typedef struct _AV_CORE_CONTEXT
{
	//  Scan thread contexts
	PLISTENER_THREAD_CONTEXT  ScanThreadCtxes;

	//  The abortion thread handle
	HANDLE   AbortThreadHandle;

	//  Finalize flag, set at UserScanFinalize(...)
	BOOLEAN  Finalized;

	//  Handle of connection port to the filter.
	HANDLE   ConnectionPort;

	//  Completion port for asynchronous message passing
	HANDLE   Completion;

} AV_CORE_CONTEXT, * PAV_CORE_CONTEXT;

#define  KM_EVENTS_LISTENER_THREAD_COUNT   4 // DEBUG !!!      // the number of scanning worker threads.

typedef struct _KM_MESSAGE
{
	//  Required structure header.
	FILTER_MESSAGE_HEADER MessageHeader;

	//  Private scanner-specific fields begin here.
	AV_MESSAGE Event;

	//  Overlapped structure: this is not really part of the message
	//  However we embed it here so that when we get pOvlp in 
	//  GetQueuedCompletionStatus(...), we can restore the message 
	//  via CONTAINING_RECORD macro.
	OVERLAPPED Ovlp;

} KM_MESSAGE, * PKM_MESSAGE;

#define KM_MESSAGE_SIZE   (sizeof(FILTER_MESSAGE_HEADER) + sizeof(AV_MESSAGE))

typedef struct _UM_REPLY_MESSAGE
{
	//  Required structure header.
	FILTER_REPLY_HEADER ReplyHeader;

	//  Private scanner-specific fields begin here.
	AV_EVENT_RESPONSE   EventResponse;

} UM_REPLY_MESSAGE, * PUM_REPLY_MESSAGE;

#define UM_REPLY_MESSAGE_SIZE   (sizeof(FILTER_REPLY_HEADER) + sizeof(ULONG))

HRESULT KMCommInit(
	_Inout_  PAV_CORE_CONTEXT Context
);

HRESULT KMCommFinalize(
	_In_  PAV_CORE_CONTEXT Context
);

#endif

