#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "KMcommunication.h"

int _cdecl
main(
	_Unreferenced_parameter_ int argc,
	_Unreferenced_parameter_ char* argv[]
)
{

	printf("press any key to start\n");
	getchar();

	UCHAR c;
	HRESULT hr = S_OK;
	AV_CORE_CONTEXT userScanCtx = { 0 };

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	hr = KMCommInit(&userScanCtx);
	if (FAILED(hr)) 
	{
		fprintf(stderr, "Failed to initialize user scan data\n");
		return 255;
	}

	for (;;) 
	{
		printf("press 'q' to quit: ");
		c = (unsigned char)getchar();
		if (c == 'q') 
		{
			break;
		}
	}

	hr = KMCommFinalize(&userScanCtx);
	if (FAILED(hr)) 
	{
		fprintf(stderr, "Failed to finalize the user scan data.\n");
	}

	return 0;
}