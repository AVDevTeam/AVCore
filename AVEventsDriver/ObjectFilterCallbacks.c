#include "AVEventsDriver.h"

OB_PREOP_CALLBACK_STATUS AVObPreProcessCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION preOpInfo)
{
	UNREFERENCED_PARAMETER(RegistrationContext);

	// debug
	UNREFERENCED_PARAMETER(preOpInfo);
	return OB_PREOP_SUCCESS;

	/*

	if (!AVCommIsInitialized())
	{
		return OB_PREOP_SUCCESS;
	}

	HANDLE currentPid = PsGetCurrentProcessId();
	if (AVCommIsExcludedPID(currentPid))
	{
		return OB_PREOP_SUCCESS;
	}
		
	BOOLEAN kernelHandle = (BOOLEAN)preOpInfo->KernelHandle;
	HANDLE targetPid = PsGetProcessId(preOpInfo->Object);
	ACCESS_MASK desiredAccess;

	if (targetPid == currentPid)
	{
		// skip self-targeted handle operartions.
		return OB_PREOP_SUCCESS;
	}

	if (preOpInfo->Operation == OB_OPERATION_HANDLE_CREATE)
	{
		desiredAccess = preOpInfo->Parameters->CreateHandleInformation.DesiredAccess;
		// set event structure
		AV_EVENT_PROCESS_HANDLE_CREATE eventPrHandleCreate = { 0 };
		eventPrHandleCreate.DesiredAccess = desiredAccess;
		eventPrHandleCreate.KernelHandle = kernelHandle;
		eventPrHandleCreate.RequestorPID = (int)(__int64)currentPid;
		eventPrHandleCreate.TargetPID = (int)(__int64)targetPid;
		
		// UM response structure
		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(
			AvProcessHandleCreate,
			&eventPrHandleCreate,
			sizeof(AV_EVENT_PROCESS_HANDLE_CREATE),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			if (UMResponse.Status == AvEventStatusBlock)
			{
				// TODO. Maybe get new access mask from UM.
				preOpInfo->Parameters->CreateHandleInformation.DesiredAccess = 0x1000; // limited information
			}
		}
	}
	else
	{
		desiredAccess = preOpInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
		HANDLE dublicateSourcePid = PsGetProcessId(
			preOpInfo->Parameters->DuplicateHandleInformation.SourceProcess);
		HANDLE dublicateTargetPid = PsGetProcessId(
			preOpInfo->Parameters->DuplicateHandleInformation.TargetProcess);

		// set event structure
		AV_EVENT_PROCESS_HANDLE_DUBLICATE eventPrHandleDublicate = { 0 };
		eventPrHandleDublicate.DesiredAccess = desiredAccess;
		eventPrHandleDublicate.KernelHandle = kernelHandle;
		eventPrHandleDublicate.RequestorPID = (int)(__int64)currentPid;
		eventPrHandleDublicate.TargetPID = (int)(__int64)targetPid;
		eventPrHandleDublicate.DublicateSourcePID = (int)(__int64)dublicateSourcePid;
		eventPrHandleDublicate.DublicateTargetPID = (int)(__int64)dublicateTargetPid;

		// UM response structure
		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(
			AvProcessHandleDublicate,
			&eventPrHandleDublicate,
			sizeof(AV_EVENT_PROCESS_HANDLE_DUBLICATE),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			if (UMResponse.Status == AvEventStatusBlock)
			{
				// TODO. Maybe get new access mask from UM.
				preOpInfo->Parameters->DuplicateHandleInformation.DesiredAccess = 0x1000; // limited information
			}
		}
	}

	return OB_PREOP_SUCCESS;

	*/
};

OB_PREOP_CALLBACK_STATUS AVObPreThreadCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION preOpInfo)
{
	UNREFERENCED_PARAMETER(RegistrationContext);

	// debug
	UNREFERENCED_PARAMETER(preOpInfo);
	return OB_PREOP_SUCCESS;

	/*

	if (!AVCommIsInitialized())
	{
		return OB_PREOP_SUCCESS;
	}

	HANDLE currentPid = PsGetCurrentProcessId();
	if (AVCommIsExcludedPID(currentPid))
	{
		return OB_PREOP_SUCCESS;
	}

	HANDLE currentTid = PsGetCurrentThreadId();
	BOOLEAN kernelHandle = (BOOLEAN)preOpInfo->KernelHandle;
	HANDLE targetPid = PsGetProcessId(PsGetThreadProcess(preOpInfo->Object));
	HANDLE targetTid = PsGetThreadId(preOpInfo->Object);

	ACCESS_MASK desiredAccess;

	if (targetPid == currentPid)
	{
		// skip self-targeted handle operartions.
		return OB_PREOP_SUCCESS;
	}

	if (preOpInfo->Operation == OB_OPERATION_HANDLE_CREATE)
	{
		desiredAccess = preOpInfo->Parameters->CreateHandleInformation.DesiredAccess;
		// set event structure
		AV_EVENT_THREAD_HANDLE_CREATE eventThHandleCreate = { 0 };
		eventThHandleCreate.DesiredAccess = desiredAccess;
		eventThHandleCreate.KernelHandle = kernelHandle;
		eventThHandleCreate.RequestorPID = (int)(__int64)currentPid;
		eventThHandleCreate.RequestorTID = (int)(__int64)currentTid;
		eventThHandleCreate.TargetPID = (int)(__int64)targetPid;
		eventThHandleCreate.TargetTID = (int)(__int64)targetTid;

		// UM response structure
		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(
			AvThreadHandleCreate,
			&eventThHandleCreate,
			sizeof(AV_EVENT_THREAD_HANDLE_CREATE),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			if (UMResponse.Status == AvEventStatusBlock)
			{
				// TODO. Maybe get new access mask from UM.
				preOpInfo->Parameters->CreateHandleInformation.DesiredAccess = 0x1000; // limited information
			}
		}
	}
	else
	{
		desiredAccess = preOpInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
		HANDLE dublicateSourcePid = PsGetProcessId(
			preOpInfo->Parameters->DuplicateHandleInformation.SourceProcess);
		HANDLE dublicateTargetPid = PsGetProcessId(
			preOpInfo->Parameters->DuplicateHandleInformation.TargetProcess);

		// set event structure
		AV_EVENT_PROCESS_HANDLE_DUBLICATE eventThHandleDublicate = { 0 };
		eventThHandleDublicate.DesiredAccess = desiredAccess;
		eventThHandleDublicate.KernelHandle = kernelHandle;
		eventThHandleDublicate.RequestorPID = (int)(__int64)currentPid;
		eventThHandleDublicate.TargetPID = (int)(__int64)targetPid;
		eventThHandleDublicate.DublicateSourcePID = (int)(__int64)dublicateSourcePid;
		eventThHandleDublicate.DublicateTargetPID = (int)(__int64)dublicateTargetPid;

		// UM response structure
		AV_EVENT_RESPONSE UMResponse;
		ULONG replyLength = sizeof(AV_EVENT_RESPONSE);

		NTSTATUS status = AVCommSendEvent(
			AvThreadHandleDublicate,
			&eventThHandleDublicate,
			sizeof(AV_EVENT_PROCESS_HANDLE_DUBLICATE),
			&UMResponse,
			&replyLength);

		if (status == STATUS_SUCCESS) // check whether communication with UM was successfull.
		{
			if (UMResponse.Status == AvEventStatusBlock)
			{
				// TODO. Maybe get new access mask from UM.
				preOpInfo->Parameters->DuplicateHandleInformation.DesiredAccess = 0x1000; // limited information
			}
		}
	}

	return OB_PREOP_SUCCESS;

	*/
};
