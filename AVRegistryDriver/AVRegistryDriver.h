#pragma once
#include <ntifs.h>
#include "KMUMcomm.h"
#include "EventsKMStructures.h"

// Exports from AVCommDriver
#pragma region EventsAPI import

DECLSPEC_IMPORT void AVCommStop(VOID);
DECLSPEC_IMPORT NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, PVOID* outUmBuffer, PSIZE_T outUmSize);
DECLSPEC_IMPORT NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);
DECLSPEC_IMPORT NTSTATUS AVCommSendEvent(void*, int, PAV_EVENT_RESPONSE, PULONG);
DECLSPEC_IMPORT HANDLE AVCommGetUmPID(VOID);

#pragma endregion EventsAPI import