#pragma once
#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

#include "EventsKMStructures.h"
#include "KMUMcomm.h"

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

#define UM_REPLY_MESSAGE_SIZE   (sizeof(FILTER_REPLY_HEADER) + sizeof(AV_EVENT_RESPONSE))