#ifndef __EVENTS_H__
#define __EVENTS_H__

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

// Defines types of user responses to the event.
// Theses statuses define further driver behavior (whether to block or allow event).
typedef enum _AV_EVENT_RETURN_STATUS
{
	AvEventStatusBlock,
	AvEventStatusAllow,

} AV_EVENT_RETURN_STATUS;

// Format of user mode response to the event.
typedef struct _AV_EVENT_RESPONSE
{
	AV_EVENT_RETURN_STATUS Status;
	void* UMMessage; // RESERVED for passing info from UM -> KM

} AV_EVENT_RESPONSE, *PAV_EVENT_RESPONSE;

// Events structures
#pragma region Events structures

// Defines parameters of AvFileCreate (KMUMcomm.h) event
typedef struct _AV_EVENT_FILE_CREATE
{
	int RequestorPID;
	char RequestorMode;

    int VolumeNameSize;
    wchar_t* VolumeName;
    int FileNameSize;
    wchar_t* FileName;

} AV_EVENT_FILE_CREATE, *PAV_EVENT_FILE_CREATE;

#pragma endregion Events structures

#endif