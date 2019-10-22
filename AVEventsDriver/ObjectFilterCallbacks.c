#include "AVEventsDriver.h"

OB_PREOP_CALLBACK_STATUS AVObPreProcessCallback(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION pObPreOperationInfo)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(pObPreOperationInfo);
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
