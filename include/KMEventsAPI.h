/**
\file
\brief Declarations for AVCommDriver exports (KMEventsAPI).
*/
#pragma once

// Exports from AVCommDriver (KMEventsAPI)
#pragma region EventsAPI import

DECLSPEC_IMPORT NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, void** outUmBuffer, PSIZE_T outUmSize);
DECLSPEC_IMPORT VOID AVCommGetUmBuffer(PVOID umAddr, PVOID outKmBuffer, SIZE_T size);
DECLSPEC_IMPORT NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);
DECLSPEC_IMPORT NTSTATUS AVCommSendEvent(AV_EVENT_TYPE eventType, void* eventBuffer, int eventBufferSize, PAV_EVENT_RESPONSE UMResponse, PULONG UMResponseLength);
DECLSPEC_IMPORT UCHAR AVCommIsExcludedPID(HANDLE PID);
DECLSPEC_IMPORT UCHAR AVCommIsInitialized(VOID);

#pragma endregion EventsAPI import