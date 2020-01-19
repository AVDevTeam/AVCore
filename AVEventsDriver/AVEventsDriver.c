/**
@file
\brief AVEventsDriver load and unload routines.

This file implements AVEventsDriver startup and unload logic.
AVEventsDriver initializes KM-UM communication interface that may
be used by other drivers via AVCommDriver exports.
*/

#include "AVEventsDriver.h"

#pragma region Prototypes

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;

NTSTATUS DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS AVEventsInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID AVEventsInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID AVEventsInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS DriverUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS AVEventsInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

EXTERN_C_END
#pragma endregion Prototypes

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, AVEventsInstanceQueryTeardown)
#pragma alloc_text(PAGE, AVEventsInstanceSetup)
#pragma alloc_text(PAGE, AVEventsInstanceTeardownStart)
#endif

// Globals
PFLT_FILTER GlobalFilter;
LARGE_INTEGER RegFilterCookie;
HANDLE ObRegistrationHandle;

//  operation registration
#pragma region operation registration
/**
Minifilter callbacks list.
*/
CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      AVEventsPreMjCreate,
      NULL },

    { IRP_MJ_OPERATION_END }
};
#pragma endregion operation registration

/**
Filter registration structure.
*/
CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    DriverUnload,                           //  MiniFilterUnload

    AVEventsInstanceSetup,                    //  InstanceSetup
    AVEventsInstanceQueryTeardown,            //  InstanceQueryTeardown
    AVEventsInstanceTeardownStart,            //  InstanceTeardownStart
    AVEventsInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

/**
\brief Volume attachment callback.

This routine is called whenever a new instance is created on a volume. This
gives us a chance to decide if we need to attach to this volume or not.

If this routine is not defined in the registration structure, automatic
instances are always created.

\param[in] FltObjects Pointer to the FLT_RELATED_OBJECTS data structure containing 
opaque handles to this filter, instance and its associated volume.

\param[in] Flags Flags describing the reason for this attach request.
\return STATUS_SUCCESS - attach, STATUS_FLT_DO_NOT_ATTACH - do not attach
*/
NTSTATUS AVEventsInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );
    PAGED_CODE();
    return STATUS_SUCCESS;
}

/**
\brief Minifilter driver teardown query callback.

This is called when an instance is being manually deleted by a
call to FltDetachVolume or FilterDetach thereby giving us a
chance to fail that detach request.

If this routine is not defined in the registration structure, explicit
detach requests via FltDetachVolume or FilterDetach will always be
failed.


\param[in] FltObjects Pointer to the FLT_RELATED_OBJECTS data structure containing
opaque handles to this filter, instance and its associated volume.

\param[in] Flags Indicating where this detach request came from.

\return	Status of this operation.
*/
NTSTATUS AVEventsInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
    return STATUS_SUCCESS;
}

/**
\brief Minifilter driver instance teardown callback.

This routine is called at the start of instance teardown.

\param[in] FltObjects Pointer to the FLT_RELATED_OBJECTS data structure containing
opaque handles to this filter, instance and its associated volume.

\param[in] Flags Reason why this instance is being deleted.
*/
VOID AVEventsInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
}

/**
\brief Minifilter driver teardown end callback.

This routine is called at the end of instance teardown.

\param[in] FltObjects Pointer to the FLT_RELATED_OBJECTS data structure containing
opaque handles to this filter, instance and its associated volume.

\param[in] Flags Reason why this instance is being deleted.
*/
VOID AVEventsInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
}

/**
\brief AVEventsDriver entry point.

This is the initialization routine for AVEventsDriver. It registers
callbacks for file system events, registry events, handle operation events
and process/thread/imageload notifications.

In case of an error during the initialization all already registered callbacks
will be removed.

This function initializes KM-UM communication interface based on the
communication port using AVCommInit export from AVCommDriver.

\param[in] DriverObject Pointer to driver object created by the system to
represent this driver.

\param[in] RegistryPath Unicode string identifying where the parameters for this
driver are located in the registry.

\return Status of driver initialization.
*/
NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	//  Register with FltMgr to tell it our callback routines
	NTSTATUS status = FltRegisterFilter(DriverObject,
		&FilterRegistration,
		&GlobalFilter);

	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = AVCommInit(GlobalFilter);

	//  Start filtering i/o
	status = FltStartFiltering(GlobalFilter);
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		return status;
	}

	status = CmRegisterCallback(AVEventsRegistryCallback, NULL, &RegFilterCookie);
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		FltUnregisterFilter(GlobalFilter);
		return status;
	}

	OB_CALLBACK_REGISTRATION obCallbackReg = { 0 };
	OB_OPERATION_REGISTRATION obOperationReg[2] = { 0 };
	RtlZeroMemory(&obCallbackReg, sizeof(OB_CALLBACK_REGISTRATION));
	RtlZeroMemory(&obOperationReg, sizeof(OB_OPERATION_REGISTRATION)*2);

	// setup callback registratio structure
	obCallbackReg.Version = ObGetFilterVersion();
	obCallbackReg.OperationRegistrationCount = 2;
	obCallbackReg.RegistrationContext = NULL;
	RtlInitUnicodeString(&obCallbackReg.Altitude, L"321000");
	obCallbackReg.OperationRegistration = obOperationReg;

	// setup operation registration structures
	obOperationReg[0].ObjectType = PsProcessType;
	obOperationReg[0].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOperationReg[0].PreOperation = (POB_PRE_OPERATION_CALLBACK)AVObPreProcessCallback;

	obOperationReg[1].ObjectType = PsThreadType;
	obOperationReg[1].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOperationReg[1].PreOperation = (POB_PRE_OPERATION_CALLBACK)AVObPreThreadCallback;

	status = ObRegisterCallbacks(&obCallbackReg, &ObRegistrationHandle);
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		FltUnregisterFilter(GlobalFilter);
		CmUnRegisterCallback(RegFilterCookie);
		return status;
	}

#ifdef _WIN64
	status = PsSetCreateProcessNotifyRoutineEx2(PsCreateProcessNotifySubsystems, (PVOID)AVCreateProcessCallback, FALSE);
#else
	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)AVCreateProcessCallback, FALSE);
#endif
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		FltUnregisterFilter(GlobalFilter);
		CmUnRegisterCallback(RegFilterCookie);
		ObUnRegisterCallbacks(ObRegistrationHandle);
		return status;
	}

	status = PsSetCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)AVCreateThreadCallback);
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		FltUnregisterFilter(GlobalFilter);
		CmUnRegisterCallback(RegFilterCookie);
		ObUnRegisterCallbacks(ObRegistrationHandle);
#ifdef _WIN64
		PsSetCreateProcessNotifyRoutineEx2(PsCreateProcessNotifySubsystems, (PVOID)AVCreateProcessCallback, TRUE);
#else
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)AVCreateProcessCallback, TRUE);
#endif
		return status;
	}

#ifdef _WIN64
	status = PsSetLoadImageNotifyRoutineEx((PLOAD_IMAGE_NOTIFY_ROUTINE)AVLoadImageCallback, PS_IMAGE_NOTIFY_CONFLICTING_ARCHITECTURE);
#else
	status = PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)AVLoadImageCallback);
#endif
	if (!NT_SUCCESS(status))
	{
		AVCommStop();
		FltUnregisterFilter(GlobalFilter);
		CmUnRegisterCallback(RegFilterCookie);
		ObUnRegisterCallbacks(ObRegistrationHandle);
#ifdef _WIN64
		PsSetCreateProcessNotifyRoutineEx2(PsCreateProcessNotifySubsystems, (PVOID)AVCreateProcessCallback, TRUE);
#else
		PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)AVCreateProcessCallback, TRUE);
#endif
		PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)AVCreateThreadCallback);
		return status;
	}

	return status;
}

/**
\brief Driver unload routine.

This is called when the driver is about to be unloaded. It removes
the callbacks that were registered in DriverEntry routine and closes
communication port.

\param[in] Flags Indicating if this is a mandatory unload.

\return The final status of unload operation.
*/
NTSTATUS DriverUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
	
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(Flags);

	//  This function will close communication port.
	AVCommStop();
	FltUnregisterFilter(GlobalFilter);
	CmUnRegisterCallback(RegFilterCookie);
	ObUnRegisterCallbacks(ObRegistrationHandle);
#ifdef _WIN64
	PsSetCreateProcessNotifyRoutineEx2(PsCreateProcessNotifySubsystems, (PVOID)AVCreateProcessCallback, TRUE);
#else
	PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)AVCreateProcessCallback, TRUE);
#endif
	PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)AVCreateThreadCallback);
	PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)AVLoadImageCallback);

	return STATUS_SUCCESS;
}