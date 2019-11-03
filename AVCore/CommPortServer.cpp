#include "CommPortServer.h"
#include "EventsParser.h"

/*
Method description:
	Connects to communication port.
	Creates IO completion port.
	Creates listeners and pumps initial events to completion port.
*/
void CommPortServer::start(IManager* manager)
{
	this->pluginManager = manager;
	HRESULT hr = S_OK;
	this->eventsPort = NULL;
	this->completionPort = NULL;
	AV_CONNECTION_CONTEXT connectionCtx;

	this->pluginManager->lockEventsProcessing();

	// Start listeners.
	for (int i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; ++i)
	{
		CommPortListener* listener = new CommPortListener(this->pluginManager);
		std::thread* thread = new std::thread(&CommPortListener::listen, listener);
		listener->thread = thread;
		this->listeners.push_back(listener);
	}

	//  Prepare the scan communication port.
	connectionCtx.Type = AvConnectForScan;
	connectionCtx.ProcessID = (HANDLE)GetCurrentProcessId();
	hr = FilterConnectCommunicationPort(AV_SCAN_PORT_NAME,
		0,
		&connectionCtx,
		sizeof(AV_CONNECTION_CONTEXT),
		NULL,
		&this->eventsPort);
	if (FAILED(hr))
	{
		eventsPort = NULL;
		throw "FAILED";
	}

	//  Create the IO completion port for asynchronous message passing. 
	this->completionPort = CreateIoCompletionPort(this->eventsPort,
		NULL,
		0,
		KM_EVENTS_LISTENER_THREAD_COUNT);
	this->pluginManager->getLogger()->log("Created completion port.");

	if (NULL == this->completionPort)
	{
		throw "FAILED";
	}

	for (std::list<CommPortListener*>::iterator it = this->listeners.begin(); it != this->listeners.end(); it++)
	{
		(*it)->setEventsPort(this->eventsPort);
		(*it)->setCompletionPort(this->completionPort);
	}

	// comm port and IO completion port were set up
	// we can resume listeners' threads.
	this->pluginManager->unlockEventsProcessing();

	//  Pump messages into queue of completion port.
	for (int i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; ++i)
	{
		PKM_MESSAGE msg = (PKM_MESSAGE)HeapAlloc(GetProcessHeap(), 0, sizeof(KM_MESSAGE));

		if (NULL == msg)
		{
			hr = MAKE_HRESULT(SEVERITY_ERROR, 0, E_OUTOFMEMORY);
			throw "FAILED";
		}

		FillMemory(&msg->Ovlp, sizeof(OVERLAPPED), 0);
		hr = FilterGetMessage(this->eventsPort,
			&msg->MessageHeader,
			FIELD_OFFSET(KM_MESSAGE, Ovlp),
			&msg->Ovlp);

		if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			hr = S_OK;
		}
		else
		{
			this->pluginManager->getLogger()->log("[UserScanInit]: FilterGetMessage failed. Error: " + std::to_string(hr));
			HeapFree(GetProcessHeap(), 0, msg);
			throw "FAILED";
		}
	}
}

/*
Method description:
	Signals listneres that it's time to stop event processing.
	Joins listneres threads.
	Closes ports.
*/
void CommPortServer::stop()
{
	this->signalCancel();
	CancelIoEx(completionPort, NULL);

	for (std::list<CommPortListener*>::iterator it = this->listeners.begin(); it != this->listeners.end(); ++it)
		(*it)->thread->join();
	this->closePorts();
}

void CommPortServer::signalCancel()
{
	for (std::list<CommPortListener*>::iterator it = this->listeners.begin(); it != this->listeners.end(); ++it)
		(*it)->signalStop();
}

void CommPortServer::closePorts()
{
	HRESULT  hr = S_OK;
	if (!CloseHandle(this->eventsPort))
	{
		this->pluginManager->getLogger()->log("[UserScanFinalize]: Failed to close the connection port.");
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	this->eventsPort = NULL;

	if (!CloseHandle(this->completionPort))
	{
		this->pluginManager->getLogger()->log("[UserScanFinalize]: Failed to close the completion port.");
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	this->completionPort = NULL;
}

long GetFileSizeMy(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

CommPortListener::CommPortListener(IManager* manager)
{
	this->pluginManager = manager;
}

void CommPortListener::listen()
{
	this->pluginManager->enterCriticalEventProcessingSection();
	this->pluginManager->leaveCriticalEventProcessingSection();

	HRESULT hr = S_OK;

	PKM_MESSAGE  message = NULL;
	UM_REPLY_MESSAGE replyMsg;
	LPOVERLAPPED pOvlp = NULL;

	DWORD outSize;
	ULONG_PTR key;
	BOOL  success = FALSE;


	ZeroMemory(&replyMsg, UM_REPLY_MESSAGE_SIZE);

	std::ostringstream ss;
	ss << this->thread->get_id();
	this->pluginManager->getLogger()->log("Current listnere thread id: " + ss.str());

	//  This thread is waiting for scan message from the driver
	for (;;)
	{
		message = NULL;

		//  Get overlapped structure asynchronously, the overlapped structure 
		//  was previously pumped by FilterGetMessage(...)
		success = GetQueuedCompletionStatus(completionPort, &outSize, &key, &pOvlp, INFINITE);

		if (!success)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());

			//  The completion port handle associated with it is closed 
			//  while the call is outstanding, the function returns FALSE, 
			//  *lpOverlapped will be NULL, and GetLastError will return ERROR_ABANDONED_WAIT_0
			if (hr == E_HANDLE)
			{
				this->pluginManager->getLogger()->log("Completion port becomes unavailable.");
				hr = S_OK;

			}
			else if (hr == HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0))
			{
				this->pluginManager->getLogger()->log("Completion port was closed.");
				hr = S_OK;
			}

			break;
		}

		//  Recover message strcuture from overlapped structure.
		//  Remember we embedded overlapped structure inside SCANNER_MESSAGE.
		//  This is because the overlapped structure obtained from GetQueuedCompletionStatus(...)
		//  is asynchronously and not guranteed in order. 
		message = CONTAINING_RECORD(pOvlp, KM_MESSAGE, Ovlp);

		if (AvMsgEvent == message->Event.MessageType)
		{
			//  Reply the scanning worker thread handle to the filter
			//  This is important because the filter will also wait for the scanning thread 
			//  in case that the scanning thread is killed before telling filter 
			//  the scan is done or aborted.
			ZeroMemory(&replyMsg, UM_REPLY_MESSAGE_SIZE);
			replyMsg.ReplyHeader.MessageId = message->MessageHeader.MessageId;
			replyMsg.EventResponse.Status = AvEventStatusAllow;
			
			void* UMMessage = NULL;

			// Process event using pluginManager.
			replyMsg.EventResponse.Status = this->pluginManager->processEvent(
				message->Event.EventType, 
				this->pluginManager->parseKMEvent(message->Event.EventType, message->Event.EventBuffer),
				&UMMessage);
			replyMsg.EventResponse.UMMessage = UMMessage;

			hr = FilterReplyMessage(eventsPort,
				&replyMsg.ReplyHeader,
				UM_REPLY_MESSAGE_SIZE);

			if (FAILED(hr))
			{
				this->pluginManager->getLogger()->log("[UserScanWorker]: Failed to reply thread handle to the minifilter. Error: " + std::to_string(hr));
				break;
			}
		}
		else
		{
			throw "INVALID MESSAGE"; // This thread should not receive other kinds of message.
		}


		if (FAILED(hr))
		{
			this->pluginManager->getLogger()->log("[UserScanWorker]: Failed to handle the message. Error code: " + std::to_string(hr));
		}

		//  If fianlized flag is set from main thread, 
		//  then it would break the while loop.
		if (this->stop)
		{
			break;
		}

		//  After we process the message, pump a overlapped structure into completion port again.
		hr = FilterGetMessage(eventsPort,
			&message->MessageHeader,
			FIELD_OFFSET(KM_MESSAGE, Ovlp),
			&message->Ovlp);

		if (hr == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED))
		{
			this->pluginManager->getLogger()->log("FilterGetMessage aborted.");
			break;

		}
		else if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			this->pluginManager->getLogger()->log("[UserScanWorker]: Failed to get message from the minifilter. Error code: " + std::to_string(hr));
			break;
		}

	}  // end of while(TRUE)

	if (message)
	{
		//  Free the memory, which originally allocated at KMCommInit(...)
		HeapFree(GetProcessHeap(), 0, message);
	}

	ss.clear();
	ss << this->thread->get_id();
	this->pluginManager->getLogger()->log("***Thread id " + ss.str() + " exiting.");
}