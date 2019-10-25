#include <aclapi.h>
#include <tchar.h>
#include <Windows.h>

#include "PipeServer.h"

int PipeServer::createSecurityAttributes(SECURITY_ATTRIBUTES * sa)
{
	DWORD dwRes;
	PSID pEveryoneSID = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

	// Create a well-known SID for the Everyone group
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
	{
		std::cout << "AllocateAndInitializeSid failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow Everyone full control to object
	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = GENERIC_ALL;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	// Create a new ACL that contains the new ACEs.
	dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes)
	{
		std::cout << "SetEntriesInAcl failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (NULL == pSD) {
		std::cout << "LocalAlloc failed : " << GetLastError() << std::endl;
		return -1;
	}

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
		std::cout << "InitializeSecurityDescriptor failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
		std::cout << "SetSecurityDescriptorDacl failed : " << GetLastError() << std::endl;
		return -1;
	}

	// Initialize a security attributes structure.
	sa->nLength = sizeof(SECURITY_ATTRIBUTES);
	sa->lpSecurityDescriptor = pSD;
	sa->bInheritHandle = FALSE;

	return 0;
}

int PipeServer::createNamedPipe()
{
	const int bufsize = 512;

	SECURITY_ATTRIBUTES sa;
	if (this->createSecurityAttributes(&sa) < 0)
	{
		std::cout << "GetSecurityDescriptor failed: " << GetLastError() << std::endl;
		return -1;
	}

	this->hPipe = CreateNamedPipe(
		this->pipeName.c_str(),   // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_NOWAIT |             // NOT blocking mode 
		PIPE_ACCEPT_REMOTE_CLIENTS,
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		bufsize,                  // output buffer size 
		bufsize,                  // input buffer size 
		100,                      // client time-out 
		&sa);					  // security attribute 

	if (this->hPipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "CreateNamedPipe failed: " << GetLastError() << std::endl;
		return -1;
	}
}

int PipeServer::waitForClient(int &_stopSignal)
{
	BOOL fConnected = FALSE;

	int hj = 0;
	while (!_stopSignal)
	{
		//std::cout << hj++ << std::endl;
		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED
		fConnected = ConnectNamedPipe(this->hPipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			return 1;
		}

		Sleep(300);
	}
}

int PipeServer::receiveMessage(std::string & _message)
{
	BOOL fSuccess = FALSE;
	const int bufsize = 512;
	char buffer[bufsize];
	DWORD cbRead;

	fSuccess = ReadFile(
		this->hPipe,	// pipe handle 
		buffer,			// buffer to receive reply 
		bufsize,		// size of buffer 
		&cbRead,		// number of bytes read 
		nullptr);		// not overlapped 

	if (!fSuccess)
	{
		//std::cout << "ReadFile failed: " << GetLastError() << std::endl;
		if (GetLastError() == ERROR_BROKEN_PIPE)
		{
			// Если клиент отключился 
			// Очистить pipe от него
			DisconnectNamedPipe(hPipe);
			return -1;
		}
		else
		{
			// Если клиент ничего не прислал 
			return 0;
		}
	}

	if (cbRead != 0)
	{
		_message = "";
		_message.append(buffer, cbRead);
	}

	return cbRead;
}

int PipeServer::sendMessage(const std::string & _message)
{
	BOOL fSuccess = FALSE;
	DWORD cbWritten;

	fSuccess = WriteFile(
		this->hPipe,				// handle to pipe 
		(_message + "\n").c_str(),	// buffer to write from 
		_message.length() + 1,		// number of bytes to write 
		&cbWritten,					// number of bytes written 
		nullptr);					// not overlapped I/O 

	if (!fSuccess || _message.length() + 1 != cbWritten)
	{
		std::cout << "WriteFile failed: " << GetLastError() << std::endl;
		return -1;
	}
}


PipeServer::PipeServer(const std::string & pipename_)
{
	this->hPipe = nullptr;
	this->pipeName = pipename_;
}

PipeServer::~PipeServer()
{
	CloseHandle(this->hPipe);
	this->hPipe = nullptr;
	this->pipeName = "";
}
