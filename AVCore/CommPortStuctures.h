#include <windows.h>
#include <fltUser.h>
#include "EventsUM.h"
#include "KMUMcomm.h"

#define  KM_EVENTS_LISTENER_THREAD_COUNT   6 // DEBUG !!!      // the number of scanning worker threads.


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

