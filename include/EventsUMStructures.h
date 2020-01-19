#pragma once
/**
\file
\brief Declares UM structures used to receive events passed from KM via KMEventsAPI.
*/

#include "EventsKMStructures.h"
#include "KMUMcomm.h"

/**
\brief Overlapped communication port message structure.
*/
typedef struct _KM_MESSAGE
{
	FILTER_MESSAGE_HEADER MessageHeader; /*!<  Required structure header. */

	AV_MESSAGE Event;

	OVERLAPPED Ovlp; /**!  Overlapped structure: this is not really part of the message
		However we embed it here so that when we get pOvlp in 
		GetQueuedCompletionStatus(...), we can restore the message 
		via CONTAINING_RECORD macro. */

} KM_MESSAGE, * PKM_MESSAGE;

/**
Size of KM massaged.
*/
#define KM_MESSAGE_SIZE   (sizeof(FILTER_MESSAGE_HEADER) + sizeof(AV_MESSAGE))

/**
\brief Structure that wraps UM reply to the server.
*/
typedef struct _UM_REPLY_MESSAGE
{
	
	FILTER_REPLY_HEADER ReplyHeader; /*!< Required structure header. */

	AV_EVENT_RESPONSE   EventResponse;

} UM_REPLY_MESSAGE, * PUM_REPLY_MESSAGE;

/**
Size of UM reply message.
*/
#define UM_REPLY_MESSAGE_SIZE   (sizeof(FILTER_REPLY_HEADER) + sizeof(AV_EVENT_RESPONSE))