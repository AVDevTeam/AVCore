/*
This file provides declarations for AVCommDriver exports.
*/

#if defined(_MSC_VER)
#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif
#endif

// Exports from AVCommDriver (KMEventsAPI)
#pragma region EventsAPI import

DECLSPEC_IMPORT NTSTATUS AVCommCreateBuffer(PVOID srcBuffer, SIZE_T srcSize, PVOID* outUmBuffer, PSIZE_T outUmSize);
DECLSPEC_IMPORT NTSTATUS AVCommFreeBuffer(PVOID UmBuffer, PSIZE_T UmBufferSize);
DECLSPEC_IMPORT NTSTATUS AVCommSendEvent(AV_EVENT_TYPE eventType, void* eventBuffer, int eventBufferSize, PAV_EVENT_RESPONSE UMResponse, PULONG UMResponseLength);
DECLSPEC_IMPORT UCHAR AVCommIsExcludedPID(HANDLE PID);
DECLSPEC_IMPORT UCHAR AVCommIsInitialized(VOID);

#pragma endregion EventsAPI import