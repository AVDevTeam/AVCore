#pragma once
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

// Defines parameters of AvProcessHandleCreate (KMUMcomm.h) event
typedef struct _AV_EVENT_PROCESS_HANDLE_CREATE
{
	int RequestorPID;
	char KernelHandle;

	int TargetPID;
	unsigned long DesiredAccess;

} AV_EVENT_PROCESS_HANDLE_CREATE, * PAV_EVENT_PROCESS_HANDLE_CREATE;

// Defines parameters of AvProcessHandleCreate (KMUMcomm.h) event
typedef struct _AV_EVENT_PROCESS_HANDLE_DUBLICATE
{
	int RequestorPID;
	char KernelHandle;

	int TargetPID;
	unsigned long DesiredAccess;

	int DublicateSourcePID;
	int DublicateTargetPID;

} AV_EVENT_PROCESS_HANDLE_DUBLICATE, * PAV_EVENT_PROCESS_HANDLE_DUBLICATE;


// Defines parameters of AvProcessThreadCreate (KMUMcomm.h) event
typedef struct _AV_EVENT_THREAD_HANDLE_CREATE
{
	int RequestorPID;
	int RequestorTID;
	char KernelHandle;

	int TargetPID;
	int TargetTID;
	unsigned long DesiredAccess;

} AV_EVENT_THREAD_HANDLE_CREATE, * PAV_EVENT_THREAD_HANDLE_CREATE;

// Defines parameters of AvProcessThreadDublicate (KMUMcomm.h) event
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

// Defines parameters of AvProcessCreate event
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

} AV_EVENT_PROCESS_CREATE, * PAV_EVENT_PROCESS_CREATE;

// Defines parameters of AvProcessCreate event
typedef struct _AV_EVENT_PROCESS_EXIT
{
	int PID;

} AV_EVENT_PROCESS_EXIT, * PAV_EVENT_PROCESS_EXIT;

// Defines parameters of AvThreadCreate/Eixt event
typedef struct _AV_EVENT_THREAD_CREATE_EXIT
{
	int PID;
	int TID;

} AV_EVENT_THREAD_CREATE, AV_EVENT_THREAD_EXIT, * PAV_EVENT_THREAD_CREATE, * PAV_EVENT_THREAD_EXIT;

// Defines parameters of AvImageLoad event
typedef struct _AV_EVENT_IMAGE_LOAD
{
	int PID;
	wchar_t* imageName;
	int imageNameSize;
	unsigned char systemModeImage;
} AV_EVENT_IMAGE_LOAD, * PAV_EVENT_IMAGE_LOAD;

#pragma endregion Events structures