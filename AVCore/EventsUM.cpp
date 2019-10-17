#include "EventsUM.h"

AvFSEventCreate::AvFSEventCreate(PAV_EVENT_FILE_CREATE KMEvent)
{
	this->RequestorMode = KMEvent->RequestorMode;
	this->RequestorPID = KMEvent->RequestorPID;

	// copy wide char strings from KM supplied buffers (VolumeName might not be NULL-terminated).
	wchar_t* path = this->wcscpyZeroTerminate(KMEvent->FileName, KMEvent->FileNameSize);
	wchar_t* volume = this->wcscpyZeroTerminate(KMEvent->VolumeName, KMEvent->VolumeNameSize);

	// translate wchar_t* to std::string [https://stackoverflow.com/questions/27720553/conversion-of-wchar-t-to-string/27721137]
	std::wstring path_ws(path), volume_ws(volume);
	std::string path_std(path_ws.begin(), path_ws.end());
	std::string volume_std(volume_ws.begin(), volume_ws.end());

	this->FilePath = volume_std + path_std;

	// free temporary buffers.
	free(path);
	free(volume);
}

AvFSEventCreate::~AvFSEventCreate()
{
}

/*++
Routine Description:
	This function copies wide char string from KM buffer and makes 
	sure that it is NULL-terminated.
Arguments:
	srcBuffer - pointer to the wide char string buffer supplied by KM driver.
	srcSize - size of KM supplied buffer (in bytes).
Return Value:
	pointer to newly allocated NULL-terminated buffer with the copied wide char string.
--*/
wchar_t* AvEvent::wcscpyZeroTerminate(wchar_t* srcBuffer, int srcSize)
{
	int dstSize = srcSize;
	dstSize += 2; // two additional bytes for wide char NULL-terminator.
	wchar_t* dstBuffer = (wchar_t*)malloc(dstSize);
	if (dstBuffer == NULL)
	{
		throw "OutOfMemory"; // TODO! Error handling (Exceptions or RetVals ?!)
	}
	memset(dstBuffer, 0, dstSize); // zero out new buffer.
	memcpy_s(dstBuffer, dstSize, srcBuffer, srcSize);
	return dstBuffer;
}
