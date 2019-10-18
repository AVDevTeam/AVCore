#include <windows.h>
#include <stdio.h>
#include <fltUser.h>
#include "KMUMcomm.h"
#include "KMcommunication.h"
#include "PipeServer.h"

/*
int _cdecl
main(
	_Unreferenced_parameter_ int argc,
	_Unreferenced_parameter_ char* argv[]
)
{
	const std::string pipeName = "\\\\.\\pipe\\AVCorePipe";

	PipeServer pipe(pipeName);

	int status = pipe.createNamedPipe();
	std::cout << "Pipe was created" << std::endl;

	pipe.waitForClient();
	std::cout << "Pipe is waiting" << std::endl;
	
	std::string message;
	pipe.receiveMessage(message);

	std::cout << "Recived: " << message;

	getchar();
}
*/


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
