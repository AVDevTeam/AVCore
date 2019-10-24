#include "AVEventsDriver.h"

OB_PREOP_CALLBACK_STATUS AVObPreProcessCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION preOpInfo)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
		
	BOOLEAN kernelHandle = (BOOLEAN)preOpInfo->KernelHandle;
	HANDLE targetPid = PsGetProcessId(preOpInfo->Object);
	HANDLE currentPid = PsGetCurrentProcessId();
	ACCESS_MASK desiredAccess;

	if (targetPid == currentPid)
	{
		// skip self-targeted handle operartions.
		return OB_PREOP_SUCCESS;
	}

	// TODO. Exclude processes.

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
};

OB_PREOP_CALLBACK_STATUS AVObPreThreadCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pObPreOperationInfo)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(pObPreOperationInfo);
	return OB_PREOP_SUCCESS;
};
