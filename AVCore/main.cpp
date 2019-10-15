#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "avlib.h"
#include "utility.h"
#include "communication.h"

int _cdecl
main(
	_Unreferenced_parameter_ int argc,
	_Unreferenced_parameter_ char* argv[]
)
/*++

Routine Description:

	Entry main function of the user space program.

Arguments:

	argc - The number of arguments
	argv - The arguments

Return Value:

	0 - No error occurs.
	255 - Error occurs.

--*/
{

	UCHAR c;
	HRESULT hr = S_OK;
	USER_SCAN_CONTEXT userScanCtx = { 0 };

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);


	//
	//  Initialize scan listening threads.
	//

	hr = UserScanInit(&userScanCtx);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to initialize user scan data\n");
		DisplayError(hr);
		return 255;
	}

	//
	//  Read user's input until it reads 'q'
	//

	for (;;) {

		printf("press 'q' to quit: ");
		c = (unsigned char)getchar();
		if (c == 'q') {

			break;
		}
	}

	//
	//  Finalize the scan thread contexts.
	//

	hr = UserScanFinalize(&userScanCtx);
	if (FAILED(hr)) {
		fprintf(stderr, "Failed to finalize the user scan data.\n");
	}

	return 0;
}