#include <ws2tcpip.h>
#include "EventsParser.h"

AvFSEventCreate::AvFSEventCreate(char requestorMode, int requestorPID, std::string FilePath)
{
	this->RequestorMode = requestorMode;
	this->RequestorPID = requestorPID;
	this->FilePath = FilePath;
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

AvFSEventCreate::~AvFSEventCreate()
{
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
	wchar_t* dstBuffer = new wchar_t[dstSize];
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

AvObEventProcessHandleCreate::~AvObEventProcessHandleCreate()
{
}

int AvObEventProcessHandleCreate::getRequestorPID()
{
	return this->requestorPID;
}

boolean AvObEventProcessHandleCreate::getIsKernelHandle()
{
	return this->isKernelHandle;
}

int AvObEventProcessHandleCreate::getTargetPID()
{
	return this->targetPID;
}

ACCESS_MASK AvObEventProcessHandleCreate::getDesiredAccess()
{
	return this->desiredAccess;
}

AvEvent* AvObEventProcessHandleCreateParser::parse(PVOID event)
{
	PAV_EVENT_PROCESS_HANDLE_CREATE eventPrHandleCreate = (PAV_EVENT_PROCESS_HANDLE_CREATE)event;
	AvEvent* eventInstanse = reinterpret_cast<AvEvent*>(new AvObEventProcessHandleCreate(
		eventPrHandleCreate->RequestorPID,
		eventPrHandleCreate->KernelHandle == TRUE,
		eventPrHandleCreate->TargetPID,
		eventPrHandleCreate->DesiredAccess));
	return eventInstanse;
}

AvObEventProcessHandleDublicate::~AvObEventProcessHandleDublicate()
{
}

int AvObEventProcessHandleDublicate::getDublicateSourcePID()
{
	return this->dublicateSourcePID;
}

int AvObEventProcessHandleDublicate::getDublicateTargetPID()
{
	return this->dublicateTargetPID;
}

int AvObEventProcessHandleDublicate::getRequestorPID()
{
	return this->requestorPID;
}

unsigned char AvObEventProcessHandleDublicate::getIsKernelHandle()
{
	return this->isKernelHandle;
}

int AvObEventProcessHandleDublicate::getTargetPID()
{
	return this->targetPID;
}

unsigned long AvObEventProcessHandleDublicate::getDesiredAccess()
{
	return this->desiredAccess;
}

AvEvent* AvObEventProcessHandleDublicateParser::parse(PVOID event)
{
	PAV_EVENT_PROCESS_HANDLE_DUBLICATE eventPrHandleDublicate = (PAV_EVENT_PROCESS_HANDLE_DUBLICATE)event;
	AvEvent* eventInstanse = reinterpret_cast<AvEvent*>(new AvObEventProcessHandleDublicate(
		eventPrHandleDublicate->RequestorPID,
		eventPrHandleDublicate->KernelHandle == TRUE,
		eventPrHandleDublicate->TargetPID,
		eventPrHandleDublicate->DesiredAccess,
		eventPrHandleDublicate->DublicateSourcePID,
		eventPrHandleDublicate->DublicateTargetPID));
	return eventInstanse;
}

AvObEventThreadHandleCreate::~AvObEventThreadHandleCreate()
{
}

int AvObEventThreadHandleCreate::getRequestorTID()
{
	return this->requestorTID;
}

int AvObEventThreadHandleCreate::getTargetTID()
{
	return this->targetTID;
}

int AvObEventThreadHandleCreate::getRequestorPID()
{
	return this->requestorPID;
}

unsigned char AvObEventThreadHandleCreate::getIsKernelHandle()
{
	return this->isKernelHandle;
}

int AvObEventThreadHandleCreate::getTargetPID()
{
	return this->targetPID;
}

unsigned long AvObEventThreadHandleCreate::getDesiredAccess()
{
	return this->desiredAccess;
}

AvEvent* AvObEventThreadHandleCreateParser::parse(PVOID event)
{
	PAV_EVENT_THREAD_HANDLE_CREATE eventThHandleCreate = (PAV_EVENT_THREAD_HANDLE_CREATE)event;
	AvEvent* eventInstanse = reinterpret_cast<AvEvent*>(new AvObEventThreadHandleCreate(
		eventThHandleCreate->RequestorPID,
		eventThHandleCreate->RequestorTID,
		eventThHandleCreate->KernelHandle == TRUE,
		eventThHandleCreate->TargetPID,
		eventThHandleCreate->TargetTID,
		eventThHandleCreate->DesiredAccess));
	return eventInstanse;
}

AvObEventThreadHandleDublicate::~AvObEventThreadHandleDublicate()
{
}

int AvObEventThreadHandleDublicate::getRequestorTID()
{
	return this->requestorTID;
}

int AvObEventThreadHandleDublicate::getTargetTID()
{
	return this->targetTID;
}

int AvObEventThreadHandleDublicate::getDublicateSourcePID()
{
	return this->dublicateSourcePID;
}

int AvObEventThreadHandleDublicate::getDublicateTargetPID()
{
	return this->dublicateTargetPID;
}

int AvObEventThreadHandleDublicate::getRequestorPID()
{
	return this->requestorPID;
}

unsigned char AvObEventThreadHandleDublicate::getIsKernelHandle()
{
	return this->isKernelHandle;
}

int AvObEventThreadHandleDublicate::getTargetPID()
{
	return this->targetPID;
}

unsigned long AvObEventThreadHandleDublicate::getDesiredAccess()
{
	return this->desiredAccess;
}

AvEvent* AvObEventThreadHandleDublicateParser::parse(PVOID event)
{
	PAV_EVENT_THREAD_HANDLE_DUBLICATE eventThHandleDublicate = (PAV_EVENT_THREAD_HANDLE_DUBLICATE)event;
	AvEvent* eventInstanse = reinterpret_cast<AvEvent*>(new AvObEventThreadHandleDublicate(
		eventThHandleDublicate->RequestorPID,
		eventThHandleDublicate->RequestorTID,
		eventThHandleDublicate->KernelHandle == TRUE,
		eventThHandleDublicate->TargetPID,
		eventThHandleDublicate->TargetTID,
		eventThHandleDublicate->DesiredAccess,
		eventThHandleDublicate->DublicateSourcePID,
		eventThHandleDublicate->DublicateTargetPID));
	return eventInstanse;
}

AvEventProcessCreate::~AvEventProcessCreate()
{
}

int AvEventProcessCreate::getPID()
{
	return this->PID;
}

int AvEventProcessCreate::getParentPID()
{
	return this->parentPID;
}

int AvEventProcessCreate::getCreatingPID()
{
	return this->creatingPID;
}

int AvEventProcessCreate::getCreatingTID()
{
	return this->creatingTID;
}

std::string& AvEventProcessCreate::getImageFileName()
{
	return this->imageFileName;
}

std::string& AvEventProcessCreate::getCommandLine()
{
	return this->commandLine;
}

AvEvent* AvEventProcessCreateParser::parse(PVOID event)
{
	PAV_EVENT_PROCESS_CREATE eventProcessCreate = (PAV_EVENT_PROCESS_CREATE)event;
	// copy wide char strings from KM supplied buffers.
	wchar_t* imageFileName = this->wcscpyZeroTerminate(eventProcessCreate->imageFileName, eventProcessCreate->imageFileNameSize);
	wchar_t* commandLine = this->wcscpyZeroTerminate(eventProcessCreate->commandLine, eventProcessCreate->commandLineSize);

	std::wstring imageFileName_ws(imageFileName), commandLine_ws(commandLine);
	std::string imageFileName_std(imageFileName_ws.begin(), imageFileName_ws.end());
	std::string commandLine_std(commandLine_ws.begin(), commandLine_ws.end());
	delete imageFileName;
	delete commandLine;

	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventProcessCreate(
		eventProcessCreate->PID,
		eventProcessCreate->parentPID,
		eventProcessCreate->creatingPID,
		eventProcessCreate->creatingTID,
		imageFileName_std,
		commandLine_std
	));
	return instance;
}

AvEventProcessEixt::~AvEventProcessEixt()
{
}

int AvEventProcessEixt::getPID()
{
	return this->PID;
}

AvEvent* AvEventProcessEixtParser::parse(PVOID event)
{
	PAV_EVENT_PROCESS_EXIT eventProcessExit = (PAV_EVENT_PROCESS_EXIT)event;
	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventProcessEixt(
		eventProcessExit->PID
	));
	return instance;
}

AvEventThreadCreate::~AvEventThreadCreate()
{
}

int AvEventThreadCreate::getPID()
{
	return this->PID;
}

int AvEventThreadCreate::getTID()
{
	return this->TID;
}

AvEvent* AvEventThreadCreateParser::parse(PVOID event)
{
	PAV_EVENT_THREAD_CREATE eventThreadCreate = (PAV_EVENT_THREAD_CREATE)event;
	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventThreadCreate(
		eventThreadCreate->PID,
		eventThreadCreate->TID));
	return instance;
}

AvEventThreadExit::~AvEventThreadExit()
{
}

int AvEventThreadExit::getPID()
{
	return this->PID;
}

int AvEventThreadExit::getTID()
{
	return this->TID;
}

AvEvent* AvEventThreadExitParser::parse(PVOID event)
{
	PAV_EVENT_THREAD_EXIT eventThreadExit = (PAV_EVENT_THREAD_EXIT)event;
	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventThreadExit(
		eventThreadExit->PID,
		eventThreadExit->TID));
	return instance;
}

AvEventImageLoad::~AvEventImageLoad()
{
}

int AvEventImageLoad::getPID()
{
	return this->PID;
}

std::string& AvEventImageLoad::getImageName()
{
	return this->imageName;
}

unsigned char AvEventImageLoad::getIsSystemModule()
{
	return this->isSystemModule;
}

AvEvent* AvEventImageLoadParser::parse(PVOID event)
{
	PAV_EVENT_IMAGE_LOAD eventImageLoad = (PAV_EVENT_IMAGE_LOAD)event;
	// copy wide char strings from KM supplied buffers.
	wchar_t* imageName = this->wcscpyZeroTerminate(eventImageLoad->imageName, eventImageLoad->imageNameSize);

	std::wstring imageName_ws(imageName);
	std::string imageName_std(imageName_ws.begin(), imageName_ws.end());
	free(imageName);

	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventImageLoad(
		eventImageLoad->PID,
		imageName_std,
		eventImageLoad->systemModeImage
	));
	return instance;
}

AvEventRegCreateKey::~AvEventRegCreateKey()
{
}

int AvEventRegCreateKey::getRequestorPID()
{
	return this->requestorPID;
}

std::string& AvEventRegCreateKey::getKeyPath()
{
	return this->keyPath;
}

AvEventRegOpenKey::~AvEventRegOpenKey()
{
}

int AvEventRegOpenKey::getRequestorPID()
{
	return this->requestorPID;
}

std::string& AvEventRegOpenKey::getKeyPath()
{
	return this->keyPath;
}

AvEvent* AvEventRegCreateKeyParser::parse(PVOID event)
{
	PAV_EVENT_REG_CREATE_KEY eventRegCreateKey = (PAV_EVENT_REG_CREATE_KEY)event;
	// copy wide char strings from KM supplied buffers.
	wchar_t* keyPath = this->wcscpyZeroTerminate(eventRegCreateKey->keyPath, eventRegCreateKey->keyPathSize);

	std::wstring keyPath_ws(keyPath);
	std::string keyPath_std(keyPath_ws.begin(), keyPath_ws.end());
	free(keyPath);
	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventRegCreateKey(
		eventRegCreateKey->requestorPID,
		keyPath_std
	));
	return instance;
}

AvEvent* AvEventRegOpenKeyParser::parse(PVOID event)
{
	PAV_EVENT_REG_OPEN_KEY eventRegOpenKey = (PAV_EVENT_REG_OPEN_KEY)event;
	// copy wide char strings from KM supplied buffers.
	wchar_t* keyPath = this->wcscpyZeroTerminate(eventRegOpenKey->keyPath, eventRegOpenKey->keyPathSize);

	std::wstring keyPath_ws(keyPath);
	std::string keyPath_std(keyPath_ws.begin(), keyPath_ws.end());
	free(keyPath);
	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventRegCreateKey(
		eventRegOpenKey->requestorPID,
		keyPath_std
	));
	return instance;
}

int AvEventWinApiCall::getPID()
{
	return this->PID;
}

std::string AvEventWinApiCall::getFunctionName()
{
	return this->functionName;
}

std::list<std::string> AvEventWinApiCall::getFunctionArgs()
{
	return this->argumetns;
}

AvEvent* AvEventNetworkParser::parse(PVOID event)
{
	PAV_EVENT_NETWORK eventnetwork = (PAV_EVENT_NETWORK)event;
	char* data = NULL;
	if (eventnetwork->data != NULL && eventnetwork->dataLength != 0) {
		data = new char[eventnetwork->dataLength];
		if (data != NULL) {
			memcpy_s(data, eventnetwork->dataLength, eventnetwork->data, eventnetwork->dataLength);
		}
	}
	
	AvEvent* instance = reinterpret_cast<AvEvent*>(new AvEventNetwork(
		(uint8_t*)eventnetwork->localAddress, (uint8_t*)eventnetwork->remoteAddress,
		eventnetwork->localPort, eventnetwork->remotePort,
		eventnetwork->isIPV6 ? AF_INET6 : AF_INET,
		data, eventnetwork->dataLength
	));
	return instance;
}

AvEventNetwork::~AvEventNetwork() {
	delete this->data;
}

void *AvEventNetwork::getLocalAddress()
{
	return (void*)this->localAddress;
}

void *AvEventNetwork::getRemoteAddress()
{
	return (void*)this->remoteAddress;
}

char *AvEventNetwork::getLocalAddressStr()
{
	char *buffer = new char[64];
	memset(buffer, 0, 64);
	//InetNtopA(this->family, this->localAddress, buffer, 64);
	return buffer;
}

char *AvEventNetwork::getRemoteAddressStr()
{
	char* buffer = new char[64];
	memset(buffer, 0, 64);
	//InetNtopA(this->family, this->remoteAddress, buffer, 64);
	return buffer;
}

int AvEventNetwork::getLocalPort()
{
	return this->localPort;
}

int AvEventNetwork::getRemotePort()
{
	return this->remotePort;
}

int AvEventNetwork::getFamily()
{
	return this->family;
}

char* AvEventNetwork::getData()
{
	return this->data;
}
unsigned long long AvEventNetwork::getDataLength()
{
	return this->dataLength;
}
