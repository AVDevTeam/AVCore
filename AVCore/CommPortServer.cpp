#include "CommPortServer.h"

/*
Method description:
	Connects to communication port.
	Creates IO completion port.
	Creates listeners and pumps initial events to completion port.
*/
void CommPortServer::start()
{
	HRESULT hr = S_OK;
	this->eventsPort = NULL;
	this->completionPort = NULL;
	AV_CONNECTION_CONTEXT connectionCtx;

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

	if (NULL == this->completionPort)
	{
		throw "FAILED";
	}

	for (int i = 0; i < KM_EVENTS_LISTENER_THREAD_COUNT; ++i)
	{
		CommPortListener * listener = new CommPortListener();
		std::thread * thread = new std::thread(&CommPortListener::listen, listener, eventsPort, completionPort);
		listener->thread = thread;
		this->listeners.push_back(listener);
	}

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
			fprintf(stderr, "[UserScanInit]: FilterGetMessage failed.\n");
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
		(*it)->signalStop();

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
		fprintf(stderr, "[UserScanFinalize]: Failed to close the connection port.\n");
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	this->eventsPort = NULL;

	if (!CloseHandle(this->completionPort))
	{
		fprintf(stderr, "[UserScanFinalize]: Failed to close the completion port.\n");
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

void CommPortListener::listen(HANDLE eventsPort, HANDLE completionPort)
{
	HRESULT hr = S_OK;

	PKM_MESSAGE  message = NULL;
	UM_REPLY_MESSAGE replyMsg;
	LPOVERLAPPED pOvlp = NULL;

	DWORD outSize;
	ULONG_PTR key;
	BOOL  success = FALSE;

	PLISTENER_THREAD_CONTEXT threadCtx = NULL;

	ZeroMemory(&replyMsg, UM_REPLY_MESSAGE_SIZE);

	std::cout << "Current listnere thread id: " << this->thread->get_id() << "\n";

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

			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!  TODO! EVENT PROCESSING. !!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (message->Event.EventType == AvFileCreate)
			{
				// Get pointer to Event structure buffer
				PAV_EVENT_FILE_CREATE KMeventFileCreate = (PAV_EVENT_FILE_CREATE)message->Event.EventBuffer;
				AvFSEventCreate* UMeventCreate = new AvFSEventCreate(KMeventFileCreate);

				//printf("AvFileCreate: %ls%ls\n", eventFileCreate->VolumeName, eventFileCreate->FileName);
				std::cout << "AvFileCreate: " << UMeventCreate->FilePath << "\n";
				//if (UMeventCreate->FilePath.rfind("C:\\Users\\user\\testfile.txt", 0) == 0)
				//{
				long size = GetFileSizeMy(UMeventCreate->FilePath);
				if (size < 4096)
				{
					std::ifstream input(UMeventCreate->FilePath, std::ios::binary);
					if (!input.fail())
					{
						std::string eicar_string = "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*";
						std::vector<unsigned char> eicar_signature(eicar_string.begin(), eicar_string.end());
						std::vector<unsigned char> file_binary(std::istreambuf_iterator<char>(input), {});
						auto res = std::search(
							file_binary.begin(),
							file_binary.end(),
							eicar_signature.begin(),
							eicar_signature.end()
						);
						auto found = res != file_binary.end();
						if (found)
						{
							replyMsg.EventResponse.Status = AvEventStatusBlock;
						}
					}
				}
				//}
			}

			hr = FilterReplyMessage(eventsPort,
				&replyMsg.ReplyHeader,
				UM_REPLY_MESSAGE_SIZE);

			if (FAILED(hr))
			{
				fprintf(stderr, "[UserScanWorker]: Failed to reply thread handle to the minifilter, %lu\n", hr);
				break;
			}
		}
		else
		{
			throw "INVALID MESSAGE"; // This thread should not receive other kinds of message.
		}


		if (FAILED(hr))
		{
			fprintf(stderr, "[UserScanWorker]: Failed to handle the message.\n");
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

	std::cout << "***Thread id " << this->thread->get_id() << " exiting\n";
}