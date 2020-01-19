/**
\file
\brief Declaration of AVComm global context

Header with KM-UM communication context declaration.
*/

#pragma once
#include <fltKernel.h>

/**
Structure for AVComm global context
*/
typedef struct _AV_COMM_GLOBAL_DATA
{
	PFLT_FILTER Filter; /*!<  The global FLT_FILTER pointer. Some API needs this, such as FltAllocateContext(...).*/

	PFLT_PORT EventsClientPort; /*!< Client communicate port connection handle.*/

	PFLT_PORT EventsServerPort; /*!< Server-side communicate port handle.*/

	HANDLE AVCoreServicePID; /*!< PID of AVCore service.*/
	HANDLE AVCoreServiceHandle; /*!< Opened KM handle to AVCore service process.*/
	PEPROCESS AVCoreServiceEprocess; /*!< Pointer to AVCore service process structure.*/

} AV_COMM_GLOBAL_DATA, * PAV_COMM_GLOBAL_DATA;