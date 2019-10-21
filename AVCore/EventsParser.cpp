#include "EventsParser.h"

AvFSEventCreate::AvFSEventCreate(char requestorMode, int requestorPID, std::string FilePath)
{
	this->RequestorMode = requestorMode;
	this->RequestorPID = requestorPID;
	this->FilePath = FilePath;
}

AvFSEventCreate::~AvFSEventCreate()
{
}

wchar_t* AvFSEventCreateParser::getVoluemLetter(wchar_t* deviceName)
{
	wchar_t volumeNames[512] = { 0 }; // todo dynamic allocation
	wchar_t curDeviceName[512] = { 0 };

	GetLogicalDriveStringsW(512, volumeNames);

	wchar_t* volumeName = volumeNames;
	while (*volumeName != NULL)
	{
		int curVolumeNameSize = wcslen(volumeName);
		volumeName[curVolumeNameSize - 1] = 0; // trim slash
		DWORD out = QueryDosDeviceW(volumeName, curDeviceName, sizeof(curDeviceName) >> 2);
		if (wcscmp(deviceName, curDeviceName) == 0)
		{
			wchar_t* resultVolumeName = (wchar_t*)malloc(sizeof(wchar_t) * (curVolumeNameSize + 1));
			if (resultVolumeName == NULL)
			{
				throw std::bad_alloc();
			}
			wcscpy_s(resultVolumeName, curVolumeNameSize + 1, volumeName);
			return resultVolumeName;
		}
		volumeName += wcslen(volumeName) + 1; // go to next volume name
	}
	return nullptr;
}

std::string& AvFSEventCreate::getFilePath()
{
	return this->FilePath;
}

int AvFSEventCreate::getRequestorPID()
{
	return this->RequestorPID;
}

char AvFSEventCreate::getRequestorMode()
{
	return this->RequestorMode;
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
wchar_t* EventParser::wcscpyZeroTerminate(wchar_t* srcBuffer, int srcSize)
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

AvEvent* AvFSEventCreateParser::parse(PVOID event)
{
	PAV_EVENT_FILE_CREATE KMEvent = (PAV_EVENT_FILE_CREATE)event;
	char RequestorMode = KMEvent->RequestorMode;
	int RequestorPID = KMEvent->RequestorPID;

	// copy wide char strings from KM supplied buffers (VolumeName might not be NULL-terminated).
	wchar_t* path = this->wcscpyZeroTerminate(KMEvent->FileName, KMEvent->FileNameSize);
	wchar_t* volume = this->wcscpyZeroTerminate(KMEvent->VolumeName, KMEvent->VolumeNameSize);
	wchar_t* volumeLetter = this->getVoluemLetter(volume);
	bool volumeLetterPresent = true;

	if (volumeLetter == nullptr)
	{
		volumeLetterPresent = false;
		volumeLetter = volume;
	}

	// translate wchar_t* to std::string [https://stackoverflow.com/questions/27720553/conversion-of-wchar-t-to-string/27721137]
	std::wstring path_ws(path), volumeLetter_ws(volumeLetter);
	std::string path_std(path_ws.begin(), path_ws.end());
	std::string volumeLetter_std(volumeLetter_ws.begin(), volumeLetter_ws.end());

	std::string FilePath = volumeLetter_std + path_std;

	// free temporary buffers.
	free(path);
	free(volume);
	if (volumeLetterPresent)
		free(volumeLetter);

	AvEvent* eventInstanse = reinterpret_cast<AvEvent*>(new AvFSEventCreate(RequestorMode, RequestorPID, FilePath));
	return eventInstanse;
}
