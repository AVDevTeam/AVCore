/*++

Copyright (c) 2011  Microsoft Corporation

Module Name:

	userscan.c

Abstract:

	The implementation of user space scanning module. You have to install filter driver first, and
	have filter manager load the minifilter. When filter driver is in its position, after calling
	UserScanInit(...) all the subsequent CreateFile or CloseHandle would trigger the data scan if
	the file is dirty.

	Before the user space scanner exit, it must call UserScanFinalize(...) to cleanup the data structure
	and close the listening threads.

Environment:

	User mode

--*/

#include <stdio.h>
#include <assert.h>
#include "KMcommunication.h"

#define  KM_EVENTS_LISTENER_THREAD_COUNT   6      // the number of scanning worker threads.

typedef struct _SCANNER_MESSAGE 
{
	//  Required structure header.
	FILTER_MESSAGE_HEADER MessageHeader;

	//  Private scanner-specific fields begin here.
	AV_EVENT Event;

	//  Overlapped structure: this is not really part of the message
	//  However we embed it here so that when we get pOvlp in 
	//  GetQueuedCompletionStatus(...), we can restore the message 
	//  via CONTAINING_RECORD macro.
	OVERLAPPED Ovlp;

} SCANNER_MESSAGE, * PSCANNER_MESSAGE;

#define SCANNER_MESSAGE_SIZE   (sizeof(FILTER_MESSAGE_HEADER) + sizeof(AV_EVENT))

typedef struct _SCANNER_REPLY_MESSAGE 
{
	//  Required structure header.
	FILTER_REPLY_HEADER ReplyHeader;

	//  Private scanner-specific fields begin here.
	ULONG   ReturnStatus;

} SCANNER_REPLY_MESSAGE, * PSCANNER_REPLY_MESSAGE;

#define SCANNER_REPLY_MESSAGE_SIZE   (sizeof(FILTER_REPLY_HEADER) + sizeof(ULONG))

#pragma region Local routines declarations
HRESULT
KMEventListener(
	_Inout_ PUSER_SCAN_CONTEXT Context
);

HRESULT
KMCommListenAbortProc(
	_Inout_ PUSER_SCAN_CONTEXT Context
);

DWORD
WaitForAll(
	_In_  PSCANNER_THREAD_CONTEXT  ScanThreadCtxes
);

HRESULT
KMCommGetThreadContextById(
	_In_  DWORD   ThreadId,
	_In_  PUSER_SCAN_CONTEXT Context,
	_Out_ PSCANNER_THREAD_CONTEXT* ScanThreadCtx
);

VOID
KMCommSynchronizedCancel(
	_In_  PUSER_SCAN_CONTEXT Context
);

HRESULT
KMCommClosePorts(
	_In_  PUSER_SCAN_CONTEXT Context
);

HRESULT
KMCommCleanup(
	_In_  PUSER_SCAN_CONTEXT Context
);
#pragma endregion Local routines

//  Implementation of exported routines.
//  Declared in KMcommunication.h
HRESULT KMCommInit(_Inout_  PUSER_SCAN_CONTEXT Context)
/*++
Routine Description:
	This routine initializes all the necessary data structures and forks listening threads.
	The caller thread is responsible for calling UserScanFinalize(...) to cleanup the
	data structures and close the listening threads.
Arguments:
	Context    - User scan context, please see userscan.h
Return Value:
	S_OK if successful. Otherwise, it returns a HRESULT error value.
--*/
{
	HRESULT  hr = S_OK;
	ULONG    i = 0;
	HANDLE   hEvent = NULL;
	PSCANNER_THREAD_CONTEXT  scanThreadCtxes = NULL;
	HANDLE   hListenAbort = NULL;
	AV_CONNECTION_CONTEXT connectionCtx;

	if (NULL == Context) 
	{
		return MAKE_HRESULT(SEVERITY_ERROR, 0, E_POINTER);
	}

	//  Create the abort listening thead.
	//  This thread is particularly listening the abortion event.
	hListenAbort = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)KMCommListenAbortProc,
		Context,
		CREATE_SUSPENDED,
		NULL);

	if (NULL == hListenAbort) 
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Cleanup;
	}

	//  Initialize scan thread contexts.
	scanThreadCtxes = (PSCANNER_THREAD_CONTEXT)HeapAlloc(GetProcessHeap(), 0, sizeof(SCANNER_THREAD_CONTEXT) * KM_EVENTS_LISTENER_THREAD_COUNT);
	if (NULL == scanThreadCtxes) 
	{
		hr = MAKE_HRESULT(SEVERITY_ERROR, 0, E_OUTOFMEMORY);
		goto Cleanup;
	}

	ZeroMemory(scanThreadCtxes, sizeof(SCANNER_THREAD_CONTEXT) * KM_EVENTS_LISTENER_THREAD_COUNT);

	//  Create scan listening threads.
	for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
	{
		scanThreadCtxes[i].Handle = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)KMEventListener,
			Context,
			CREATE_SUSPENDED,
			&scanThreadCtxes[i].ThreadId);

		if (NULL == scanThreadCtxes[i].Handle) 
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto Cleanup;
		}
	}

	//  Prepare the scan communication port.
	connectionCtx.Type = AvConnectForScan;
	connectionCtx.ProcessID = (HANDLE)GetCurrentProcessId();
	hr = FilterConnectCommunicationPort(AV_SCAN_PORT_NAME,
		0,
		&connectionCtx,
		sizeof(AV_CONNECTION_CONTEXT),
		NULL,
		&Context->ConnectionPort);
	if (FAILED(hr)) 
	{
		Context->ConnectionPort = NULL;
		goto Cleanup;
	}

	//  Create the IO completion port for asynchronous message passing. 
	Context->Completion = CreateIoCompletionPort(Context->ConnectionPort,
		NULL,
		0,
		KM_EVENTS_LISTENER_THREAD_COUNT);

	if (NULL == Context->Completion) 
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Cleanup;
	}

	Context->ScanThreadCtxes = scanThreadCtxes;
	Context->AbortThreadHandle = hListenAbort;

	//  Resume all the scanning threads.
	for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
	{
		if (ResumeThread(scanThreadCtxes[i].Handle) == -1) 
		{
			fprintf(stderr, "[UserScanInit]: ResumeThread scan listening thread failed.\n");
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto Cleanup;
		}
	}

	//  Resume abort listening thread.
	if (ResumeThread(hListenAbort) == -1) 
	{
		fprintf(stderr, "[UserScanInit]: ResumeThread abort listening thread failed.\n");
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Cleanup;
	}

	//  Pump messages into queue of completion port.
	for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
	{
		PSCANNER_MESSAGE msg = (PSCANNER_MESSAGE)HeapAlloc(GetProcessHeap(), 0, sizeof(SCANNER_MESSAGE));

		if (NULL == msg) 
		{
			hr = MAKE_HRESULT(SEVERITY_ERROR, 0, E_OUTOFMEMORY);
			goto Cleanup;
		}

		FillMemory(&msg->Ovlp, sizeof(OVERLAPPED), 0);
		hr = FilterGetMessage(Context->ConnectionPort,
			&msg->MessageHeader,
			FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
			&msg->Ovlp);

		if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) 
		{
			hr = S_OK;
		}
		else 
		{
			fprintf(stderr, "[UserScanInit]: FilterGetMessage failed.\n");
			HeapFree(GetProcessHeap(), 0, msg);
			goto Cleanup;
		}
	}
	return hr;

Cleanup:
	if (Context->Completion && !CloseHandle(Context->Completion)) 
	{
		fprintf(stderr, "[UserScanInit] Error! Close completion port failed.\n");
	}
	if (Context->ConnectionPort && !CloseHandle(Context->ConnectionPort)) 
	{
		fprintf(stderr, "[UserScanInit] Error! Close connection port failed.\n");
	}
	if (scanThreadCtxes) 
	{
		for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
		{
			if (scanThreadCtxes[i].Handle && !CloseHandle(scanThreadCtxes[i].Handle)) 
			{
				fprintf(stderr, "[UserScanInit] Error! Close scan thread failed.\n");
			}
		}
		HeapFree(GetProcessHeap(), 0, scanThreadCtxes);
	}
	if (hListenAbort && !CloseHandle(hListenAbort)) 
	{
		fprintf(stderr, "[UserScanInit] Error! Close listen abort thread failed.\n");
	}
	if (hEvent && !CloseHandle(hEvent)) 
	{
		fprintf(stderr, "[UserScanInit] Error! Close event handle failed.\n");
	}

	return hr;
}

HRESULT KMCommFinalize(
	_In_  PUSER_SCAN_CONTEXT Context
)
/*++
Routine Description:
	This routine cleans up all the necessary data structures and closes listening threads.
	It does the following things:
	  1) Cancel all the scanning threads and wait for them to terminate.
	  2) Close all the thread handles
	  3) Close all the port handles
	  4) Free memory of scan thread contexts.
Arguments:
	Context    - User scan context, please see userscan.h
Return Value:
	S_OK if successful. Otherwise, it returns a HRESULT error value.
--*/
{
	HRESULT  hr = S_OK;
	printf("=================finalize\n");

	KMCommSynchronizedCancel(Context);

	printf("[UserScanFinalize]: Closing connection port\n");

	hr = KMCommCleanup(Context);

	return hr;
}


DWORD WaitForAll(
	_In_  PSCANNER_THREAD_CONTEXT  ScanThreadCtxes
)
/*++
Routine Description:
	A local helper function that enable the caller to wair for all the scan threads.
Arguments:
	ScanThreadCtxes    - Scan thread contextes.
Return Value:
	Please consult WaitForMultipleObjects(...)
--*/
{
	ULONG i = 0;
	HANDLE hScanThreads[KM_EVENTS_LISTENER_THREAD_COUNT] = { 0 };
	for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
	{
		hScanThreads[i] = ScanThreadCtxes[i].Handle;
	}
	return WaitForMultipleObjects(KM_EVENTS_LISTENER_THREAD_COUNT, hScanThreads, TRUE, INFINITE);
}

HRESULT KMCommGetThreadContextById(
	_In_  DWORD   ThreadId,
	_In_  PUSER_SCAN_CONTEXT Context,
	_Out_ PSCANNER_THREAD_CONTEXT* ScanThreadCtx
)
/*++
Routine Description:
	This routine search for the scan thread context by its thread id.
Arguments:
	ThreadId  - The thread id to be searched.
	Context   - The user scan context.
	ScanThreadCtx  -  Output scan thread context.
Return Value:
	S_OK if found, otherwise not found.
--*/
{
	HRESULT hr = S_OK;
	ULONG i;
	PSCANNER_THREAD_CONTEXT kmListenerThreadCtx = Context->ScanThreadCtxes;

	*ScanThreadCtx = NULL;

	for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
	{
		if (ThreadId == kmListenerThreadCtx[i].ThreadId) 
		{
			*ScanThreadCtx = (kmListenerThreadCtx + i);
			return hr;
		}
	}
	return MAKE_HRESULT(SEVERITY_ERROR, 0, E_FAIL);
}

VOID KMCommSynchronizedCancel(
	_In_  PUSER_SCAN_CONTEXT Context
)
/*++
Routine Description:
	This routine tries to abort all the scanning threads and wait for them to terminate.
Arguments:
	Context    - User scan context, please see userscan.h
Return Value:
	Please consult WaitForMultipleObjects(...)
--*/
{
	ULONG i;
	PSCANNER_THREAD_CONTEXT  scanThreadCtxes = Context->ScanThreadCtxes;

	if (NULL == scanThreadCtxes) 
	{
		fprintf(stderr, "Scan thread contexes are NOT suppoed to be NULL.\n");
		return;
	}

	//  Tell all scanning threads that the program is going to exit.
	Context->Finalized = TRUE;

	//  Wake up the listening thread if it is waiting for message 
	//  via GetQueuedCompletionStatus() 
	CancelIoEx(Context->ConnectionPort, NULL);

	//  Wait for all scan threads to complete cancellation, 
	//  so we will be able to close the connection port and etc.
	WaitForAll(scanThreadCtxes);
	return;
}

HRESULT KMCommClosePorts(
	_In_  PUSER_SCAN_CONTEXT Context
)
/*++
Routine Description:
	This routine cleans up all the necessary data structures and closes listening threads.
	It does closing the scanning communication port and completion port.
Arguments:
	Context    - User scan context, please see userscan.h
Return Value:
	S_OK if successful. Otherwise, it returns a HRESULT error value.
--*/
{
	HRESULT  hr = S_OK;
	if (!CloseHandle(Context->ConnectionPort)) 
	{
		fprintf(stderr, "[UserScanFinalize]: Failed to close the connection port.\n");
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	Context->ConnectionPort = NULL;

	if (!CloseHandle(Context->Completion)) 
	{
		fprintf(stderr, "[UserScanFinalize]: Failed to close the completion port.\n");
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	Context->Completion = NULL;

	return hr;
}

HRESULT KMCommCleanup(
	_In_  PUSER_SCAN_CONTEXT Context
)
/*++
Routine Description:
	This routine cleans up all the necessary data structures and closes listening threads.
	It does closing abort thread handle and all scanning threads. It also closes the ports
	by calling UserScanClosePorts(...).
Arguments:
	Context    - User scan context, please see userscan.h
Return Value:
	S_OK if successful. Otherwise, it returns a HRESULT error value.
--*/
{
	ULONG i = 0;
	HRESULT  hr = S_OK;
	PSCANNER_THREAD_CONTEXT  scanThreadCtxes = Context->ScanThreadCtxes;

	if (NULL == scanThreadCtxes) 
	{
		fprintf(stderr, "Scan thread contexes are NOT suppoed to be NULL.\n");
		return E_POINTER;
	}

	if (Context->AbortThreadHandle) 
	{
		CloseHandle(Context->AbortThreadHandle);
	}

	hr = KMCommClosePorts(Context);

	//  Clean up scan thread contexts
	for (i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; i++) 
	{
		if (scanThreadCtxes[i].Handle && !CloseHandle(scanThreadCtxes[i].Handle)) {
			fprintf(stderr, "[UserScanInit] Error! Close scan thread failed.\n");
		}
	}

	HeapFree(GetProcessHeap(), 0, scanThreadCtxes);
	Context->ScanThreadCtxes = NULL;
	return hr;
}

HRESULT KMEventListener(
	_Inout_   PUSER_SCAN_CONTEXT Context
)
/*++
Routine Description:
	This routine is the scanning worker thread procedure.
	The pseudo-code of this function is as follows,

	while(TRUE) {
		1) Get a overlap structure from the completion port.
		2) Obtain message from overlap structure.
		3) Process the message via calling UserScanHandleStartScanMsg(...)
		4) Pump overlap structure into completion port using FilterGetMessage(...)
	}

Arguments:
	Context - The user scan context.

Return Value:
	S_OK if no error occurs; Otherwise, it would return appropriate HRESULT.
--*/
{
	HRESULT hr = S_OK;

	PSCANNER_MESSAGE  message = NULL;
	SCANNER_REPLY_MESSAGE replyMsg;
	LPOVERLAPPED pOvlp = NULL;

	DWORD outSize;
	ULONG_PTR key;
	BOOL  success = FALSE;

	PSCANNER_THREAD_CONTEXT threadCtx = NULL;

	hr = KMCommGetThreadContextById(GetCurrentThreadId(), Context, &threadCtx);
	if (FAILED(hr)) 
	{
		fprintf(stderr, "[UserScanWorker]: Failed to get thread context.\n");
		return hr;
	}

	ZeroMemory(&replyMsg, SCANNER_REPLY_MESSAGE_SIZE);

	printf("Current thread handle %p, id:%u\n", threadCtx->Handle, threadCtx->ThreadId);

	//  This thread is waiting for scan message from the driver
	for (;;) 
	{
		message = NULL;

		//  Get overlapped structure asynchronously, the overlapped structure 
		//  was previously pumped by FilterGetMessage(...)
		success = GetQueuedCompletionStatus(Context->Completion, &outSize, &key, &pOvlp, INFINITE);

		if (!success) 
		{
			hr = HRESULT_FROM_WIN32(GetLastError());

			//  The completion port handle associated with it is closed 
			//  while the call is outstanding, the function returns FALSE, 
			//  *lpOverlapped will be NULL, and GetLastError will return ERROR_ABANDONED_WAIT_0
			if (hr == E_HANDLE) 
			{
				printf("Completion port becomes unavailable.\n");
				hr = S_OK;

			}
			else if (hr == HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0))
			{
				printf("Completion port was closed.\n");
				hr = S_OK;
			}

			break;
		}

		//  Recover message strcuture from overlapped structure.
		//  Remember we embedded overlapped structure inside SCANNER_MESSAGE.
		//  This is because the overlapped structure obtained from GetQueuedCompletionStatus(...)
		//  is asynchronously and not guranteed in order. 
		message = CONTAINING_RECORD(pOvlp, SCANNER_MESSAGE, Ovlp);

		if (AvMsgEvent == message->Event.MessageType) 
		{
			//  Reply the scanning worker thread handle to the filter
			//  This is important because the filter will also wait for the scanning thread 
			//  in case that the scanning thread is killed before telling filter 
			//  the scan is done or aborted.
			ZeroMemory(&replyMsg, SCANNER_REPLY_MESSAGE_SIZE);
			replyMsg.ReplyHeader.MessageId = message->MessageHeader.MessageId;

			printf("FILE: %ls\n", message->Event.EventBuffer);
			if (wcsstr((wchar_t*)message->Event.EventBuffer, L"block_access.txt"))
			{
				replyMsg.ReturnStatus = 0;
			}
			else
			{
				replyMsg.ReturnStatus = 1;
			}

			hr = FilterReplyMessage(Context->ConnectionPort,
				&replyMsg.ReplyHeader,
				SCANNER_REPLY_MESSAGE_SIZE);

			if (FAILED(hr)) 
			{
				fprintf(stderr, "[UserScanWorker]: Failed to reply thread handle to the minifilter, %lu\n", hr);
				break;
			}
			fprintf(stderr, "[UserScanWorker]: Sent reply.\n");
		}
		else 
		{
			assert(FALSE); // This thread should not receive other kinds of message.
		}


		if (FAILED(hr)) 
		{
			fprintf(stderr, "[UserScanWorker]: Failed to handle the message.\n");
		}

		//  If fianlized flag is set from main thread, 
		//  then it would break the while loop.
		if (Context->Finalized) 
		{
			break;
		}

		//  After we process the message, pump a overlapped structure into completion port again.
		hr = FilterGetMessage(Context->ConnectionPort,
			&message->MessageHeader,
			FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
			&message->Ovlp);

		if (hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)) 
		{
			printf("FilterGetMessage aborted.\n");
			break;

		}
		else if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) 
		{
			fprintf(stderr, "[UserScanWorker]: Failed to get message from the minifilter. \n0x%x, 0x%x\n", hr, HRESULT_FROM_WIN32(GetLastError()));
			break;
		}

	}  // end of while(TRUE)

	if (message) 
	{
		//  Free the memory, which originally allocated at KMCommInit(...)
		HeapFree(GetProcessHeap(), 0, message);
	}

	printf("***Thread id %u exiting\n", threadCtx->ThreadId);

	return hr;
}

HRESULT KMCommListenAbortProc(
	_Inout_   PUSER_SCAN_CONTEXT Context
)
/*++
Routine Description:
	This routine is the abort listening thread procedure.
	This thread is particularly listening the abortion notifcation from the filter.
	The pseudo-code of this function is as follows,

	while(TRUE) {
		1) Wair for and get a message from the filter via FilterGetMessage(...)
		2) Find the scan thread context by its thread id.
		3) Set the cancel flag to be TRUE.
	}
Arguments:
	Context - The user scan context.
Return Value:
	S_OK if no error occurs; Otherwise, it would return appropriate HRESULT.
--*/
{
	HRESULT hr = S_OK;
	HANDLE abortPort = NULL;  //  A port for listening the abort notification from driver.
	SCANNER_MESSAGE message;
	DWORD  dwThisThread = GetCurrentThreadId();
	SCANNER_REPLY_MESSAGE replyMsg;
	AV_CONNECTION_CONTEXT connectionCtx;
	PSCANNER_THREAD_CONTEXT threadCtx = NULL;

	ZeroMemory(&message, SCANNER_MESSAGE_SIZE);

	//  Prepare the abort communication port.
	connectionCtx.Type = AvConnectForAbort;
	hr = FilterConnectCommunicationPort(AV_ABORT_PORT_NAME,
		0,
		&connectionCtx,
		sizeof(AV_CONNECTION_CONTEXT),
		NULL,
		&abortPort);
	if (FAILED(hr)) 
	{
		abortPort = NULL;
		return hr;
	}

	//  This thread is listening an scan abortion notifcation or filter unloading from the kernel
	//  If it receives notification, its type must be AvMsgAbortScanning or AvMsgFilterUnloading
	for (;;) 
	{
		//  Wait until an abort command is sent from filter.
		hr = FilterGetMessage(abortPort,
			&message.MessageHeader,
			SCANNER_MESSAGE_SIZE,
			NULL);

		if (hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED)) 
		{
			printf("[UserScanListenAbortProc]: FilterGetMessage aborted.\n");
			hr = S_OK;
			break;

		}
		else if (FAILED(hr))
		{
			fprintf(stderr, "[UserScanListenAbortProc]: Failed to get message from the minifilter.\n");
			continue;
		}

		printf("[UserScanListenAbortProc]: Got message %llu. \n", message.MessageHeader.MessageId);

		if (AvMsgFilterUnloading == message.Event.MessageType) 
		{
			//  After this thread receives AvMsgFilterUnloading
			//  it does
			//    1) Cancell all the scanning threads
			//    2) Wait for them to finish the cancel.
			//    3) Reply to filter so that the filter can know it can close the server ports.
			//    4) Close scan port, completion port, and abortion port.
			//    5) Exit the process
			KMCommSynchronizedCancel(Context);
			printf("The filter is unloading, exit!\n");
			ZeroMemory(&replyMsg, SCANNER_REPLY_MESSAGE_SIZE);
			replyMsg.ReplyHeader.MessageId = message.MessageHeader.MessageId;
			replyMsg.ReturnStatus = dwThisThread;
			hr = FilterReplyMessage(abortPort,
				&replyMsg.ReplyHeader,
				SCANNER_REPLY_MESSAGE_SIZE);

			if (FAILED(hr)) 
			{
				fprintf(stderr, "[UserScanListenAbortProc]: Error! FilterReplyMessage failed.\n");
			}

			KMCommClosePorts(Context);
			CloseHandle(abortPort);
			ExitProcess(0);
			break;

		}
		else 
		{
			assert(FALSE); // This thread should not receive other kinds of message.
		}

		if (FAILED(hr)) 
		{
			fprintf(stderr, "[UserScanListenAbortProc]: Failed to handle the message.\n");
		}
	} // end of while(TRUE)

	if (!CloseHandle(abortPort)) 
	{
		fprintf(stderr, "[UserScanListenAbortProc]: Failed to close the connection port.\n");
	}
	abortPort = NULL;

	return hr;
}
