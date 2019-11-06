/**
\file
\brief KM-UM communication shared structures.

This header file defines the common data structure used by kernel and user.
*/
#pragma once

// debug switches for KM event generators.
#pragma region EVENTS_SWITCHES
#define REGISTRY_EVENTS

#define PROCESS_HANDLE_EVENTS
#define THREAD_HANDLE_EVENTS

#define PROCESS_CREATE_EVENTS
#define PROCESS_EXIT_EVENTS

#define THREAD_CREATE_EVENTS
#define THREAD_EXIT_EVENTS

#define IMAGE_EVENTS

#define FILE_SYSTEM_EVENTS
#pragma endregion EVENTS_SWITCHES

/**
Name of communication port between AVComm (KM) and AVCore (UM).
*/
#define AV_SCAN_PORT_NAME L"\\AVCoreEventsPort"
/**
Name of pipe that is used to receive UM events from hook engines injected into UM processes.
*/
#define AV_UM_EVENTS_PIPE_NAME "\\\\.\\pipe\\AVCoreUMEvents"

/**
Message type enumeration, please see AV_SCANNER_NOTIFICATION below
*/
typedef enum _AV_MESSAGE_TYPE
{
	AvMsgEvent
} AV_MESSAGE_TYPE;

/**
\brief Enumberation of supported system events.

These constant are used to identify events passed from KM to UM.
*/
typedef enum _AV_EVENT_TYPE
{
	AvFileCreate, /*!< File system (IRP_MJ_CREATE pre) */
	AvProcessHandleCreate, /*!< Process handle create operation (ObFilter process handle create) */
	AvProcessHandleDublicate, /*!< Process handle dublicate operations (ObFilter process handle dublicate) */
	AvThreadHandleCreate, /*!< Thread handle create operation (ObFilter thread handle create) */
	AvThreadHandleDublicate, /*!< Thread handle dublicate operation (ObFilter thread handle dublicate) */
	AvProcessCreate, /*!< New process notification (PsSetCreateProcessNotifyRoutineEx[2]) */
	AvProcessExit, /*!< Process termination notification (PsSetCreateProcessNotifyRoutineEx[2]) */
	AvThreadCreate, /*!< New thread notification (PsSetThreadeCreateNotifyRoutine) */
	AvThreadExit, /*!< Thread termination notification (PsSetThreadeCreateNotifyRoutine) */
	AvImageLoad, /*!< Module load notification (PsSetImageNotifyRoutine[Ex]) */
	AvRegCreateKey, /*!< Registry key create operation (RegNtPreCreateKey[Ex]) */
	AvRegOpenKey, /*!< Registry key open operation (RegNtPreOpenKey[Ex]) */
	AvApcProcessInject, /*!< Internal event for APC injection control (PsSetCreateProcessNotifyRoutineEx sent from injdrv) */
	AvWinApiCall, /*!< UM event passed from injected hooking engine (detours hooks) */
} AV_EVENT_TYPE;

/**
\brief Event stucture: KM->UM Message
*/
typedef struct _AV_MESSAGE
{
    AV_MESSAGE_TYPE MessageType; /*!< Message type*/

	AV_EVENT_TYPE EventType; /*!< Specifies event type*/

	ULONG EventBufferLength; /*!< Size of event buffer*/
	PVOID EventBuffer; /*!< Pointer to the event buffer that was stored in UM virtual address space*/

} AV_MESSAGE, *PAV_MESSAGE;

/**
Connection type enumeration..
*/
typedef enum _AV_CONNECTION_TYPE 
{
    AvConnectForEvents = 1,
} AV_CONNECTION_TYPE, *PAV_CONNECTION_TYPE;

/**
Connection context. It will be passed through FilterConnectCommunicationPort(...)
*/
typedef struct _AV_CONNECTION_CONTEXT 
{
	AV_CONNECTION_TYPE   Type;
	HANDLE ProcessID; /*!< Is used to pass PID of AVCore service from UM. */

} AV_CONNECTION_CONTEXT, *PAV_CONNECTION_CONTEXT;
