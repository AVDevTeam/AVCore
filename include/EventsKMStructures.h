/**
\file
\brief Declares KM events structures that are passed to UM via KMEventsAPI.
*/
#pragma once

/**
\brief List of possible event processing results.

Defines types of user responses to the event.
Theses statuses define further driver behavior (whether to block or allow event).
*/
typedef enum _AV_EVENT_RETURN_STATUS
{
	AvEventStatusBlock, /*!< One of the plagins return Block status. */
	AvEventStatusAllow, /*!< Event was processed by all plugins and is allowed to complete. */
	AvEventStatusInjectAPC, /*!< Internal status used by InjectPlugin to signal APC inject start. */

} AV_EVENT_RETURN_STATUS;

/**
\brief Format of user mode response to the event.
*/
typedef struct _AV_EVENT_RESPONSE
{
	AV_EVENT_RETURN_STATUS Status;
	void* UMMessage; /*!< Used for passing info from UM -> KM */

} AV_EVENT_RESPONSE, *PAV_EVENT_RESPONSE;

// Events structures
#pragma region Events structures

/**
\brief Defines parameters of AvFileCreate (KMUMcomm.h) event
*/
typedef struct _AV_EVENT_FILE_CREATE
{
	int RequestorPID;
	char RequestorMode;

    int VolumeNameSize;
    wchar_t* VolumeName;
    int FileNameSize;
    wchar_t* FileName;

} AV_EVENT_FILE_CREATE, *PAV_EVENT_FILE_CREATE;

/**
\brief Defines parameters of AvProcessHandleCreate (KMUMcomm.h) event
*/
typedef struct _AV_EVENT_PROCESS_HANDLE_CREATE
{
	int RequestorPID;
	char KernelHandle;

	int TargetPID;
	unsigned long DesiredAccess;

} AV_EVENT_PROCESS_HANDLE_CREATE, * PAV_EVENT_PROCESS_HANDLE_CREATE;

/**
\brief Defines parameters of AvProcessHandleCreate (KMUMcomm.h) event
*/
typedef struct _AV_EVENT_PROCESS_HANDLE_DUBLICATE
{
	int RequestorPID;
	char KernelHandle;

	int TargetPID;
	unsigned long DesiredAccess;

	int DublicateSourcePID;
	int DublicateTargetPID;

} AV_EVENT_PROCESS_HANDLE_DUBLICATE, * PAV_EVENT_PROCESS_HANDLE_DUBLICATE;


/**
\brief Defines parameters of AvProcessThreadCreate (KMUMcomm.h) event
*/
typedef struct _AV_EVENT_THREAD_HANDLE_CREATE
{
	int RequestorPID;
	int RequestorTID;
	char KernelHandle;

	int TargetPID;
	int TargetTID;
	unsigned long DesiredAccess;

} AV_EVENT_THREAD_HANDLE_CREATE, * PAV_EVENT_THREAD_HANDLE_CREATE;

/**
\brief Defines parameters of AvProcessThreadDublicate (KMUMcomm.h) event
*/
typedef struct _AV_EVENT_THREAD_HANDLE_DUBLICATE
{
	int RequestorPID;
	int RequestorTID;
	char KernelHandle;

	int TargetPID;
	int TargetTID;
	unsigned long DesiredAccess;

	int DublicateSourcePID;
	int DublicateTargetPID;

} AV_EVENT_THREAD_HANDLE_DUBLICATE, * PAV_EVENT_THREAD_HANDLE_DUBLICATE;

/**
\brief Defines parameters of AvProcessCreate event
*/
typedef struct _AV_EVENT_PROCESS_CREATE
{
	int PID;
	int parentPID;
	int creatingPID;
	int creatingTID;
	wchar_t* imageFileName;
	int imageFileNameSize;
	wchar_t* commandLine;
	int commandLineSize;

} AV_EVENT_PROCESS_CREATE, * PAV_EVENT_PROCESS_CREATE, AV_EVENT_APC_PROCESS_INJECT, * PAV_EVENT_APC_PROCESS_INJECT;

/**
\brief Defines parameters of AvProcessCreate event
*/
typedef struct _AV_EVENT_PROCESS_EXIT
{
	int PID;

} AV_EVENT_PROCESS_EXIT, * PAV_EVENT_PROCESS_EXIT;

/**
\brief Defines parameters of AvThreadCreate/Eixt event
*/
typedef struct _AV_EVENT_THREAD_CREATE_EXIT
{
	int PID;
	int TID;

} AV_EVENT_THREAD_CREATE, AV_EVENT_THREAD_EXIT, * PAV_EVENT_THREAD_CREATE, * PAV_EVENT_THREAD_EXIT;

/**
\brief Defines parameters of AvImageLoad event
*/
typedef struct _AV_EVENT_IMAGE_LOAD
{
	int PID;
	wchar_t* imageName;
	int imageNameSize;
	unsigned char systemModeImage;
} AV_EVENT_IMAGE_LOAD, * PAV_EVENT_IMAGE_LOAD;

/**
\brief Defines parameters of AvRegOpenKey and AvRegCreateKey events
*/
typedef struct _AV_EVENT_REG_CREATE_OPEN_KEY
{
	int requestorPID;
	wchar_t* keyPath;
	int keyPathSize;
} AV_EVENT_REG_CREATE_KEY, AV_EVENT_REG_OPEN_KEY, * PAV_EVENT_REG_CREATE_KEY, * PAV_EVENT_REG_OPEN_KEY;

#pragma endregion Events structures