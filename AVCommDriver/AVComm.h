/**
\file

Private header file for classpnp.sys modules.  This contains private
structure and function declarations as well as constant values which do
not need to be exported.
*/

#include <fltKernel.h>

//  Debugging level flags.
#define AVDBG_TRACE_ROUTINES            0x00000001
#define AVDBG_TRACE_OPERATION_STATUS    0x00000002
#define AVDBG_TRACE_DEBUG               0x00000004
#define AVDBG_TRACE_ERROR               0x00000008

#define AV_CONNECTION_CTX_TAG                'cCvA'

#define AV_DBG_PRINT( _dbgLevel, _string )          \
    if(FlagOn(Globals.DebugLevel,(_dbgLevel))) {    \
        DbgPrint _string;                           \
    }

// Prototypes
#pragma region Prototypes

NTSTATUS DllInitialize(
	_In_ PUNICODE_STRING RegistryPath
);

NTSTATUS DllUnload(VOID);

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
);

#pragma endregion Prototypes
