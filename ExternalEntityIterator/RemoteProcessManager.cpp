#include <Windows.h>
#include <TlHelp32.h>
#include "Util.h"
#include "RemoteProcessManager.h"
#include <stdio.h>

RemoteProcessManager* RPM = NULL;

RemoteProcessManager::RemoteProcessManager(void)
{
	this->m_hProcessHandle = NULL;
	this->m_dwProcessId = NULL;
}

RemoteProcessManager::~RemoteProcessManager(void)
{
	CloseHandle( this->m_hProcessHandle );
}

DWORD RemoteProcessManager::GetProcessId( char* ProcessName )
{
	DWORD dwProcessID = NULL;
	PROCESSENTRY32 ProcEntry32 = {};
	ZeroMemory( &ProcEntry32, sizeof(PROCESSENTRY32) );
	ProcEntry32.dwSize = sizeof( PROCESSENTRY32 );

	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( hSnapshot == NULL || hSnapshot == INVALID_HANDLE_VALUE )
		return NULL;

	if ( Process32First( hSnapshot, &ProcEntry32 ) != TRUE )
	{
		CloseHandle( hSnapshot );
		return NULL;
	}
	while (true)
	{
		if (StrCmpToLower(ProcEntry32.szExeFile, ProcessName) == true)
		{
			dwProcessID = ProcEntry32.th32ProcessID;
			break;
		}
		if ( Process32Next( hSnapshot, &ProcEntry32 ) != TRUE ) break;
	}

	CloseHandle( hSnapshot );

	return dwProcessID;
}

bool RemoteProcessManager::OpenProcess( DWORD dwPid )
{
	if ( this->m_hProcessHandle != NULL && 
		 this->m_hProcessHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( this->m_hProcessHandle );
		this->m_hProcessHandle = NULL;
	}

	this->m_hProcessHandle = ::OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwPid );

	if ( this->m_hProcessHandle == NULL || 
		 this->m_hProcessHandle == INVALID_HANDLE_VALUE )
		return false;

	this->m_dwProcessId = dwPid;

	return true;
}

bool RemoteProcessManager::Read( DWORD_PTR dwAddress, void* out, SIZE_T size )
{
	ZeroMemory( out, size );
	SIZE_T numberOfBytesRead = 0;

	if ( this->m_hProcessHandle == NULL || 
		 this->m_hProcessHandle == INVALID_HANDLE_VALUE )
		return false;

	BOOL bReadProcMem = ReadProcessMemory( this->m_hProcessHandle, (PVOID)dwAddress, (PVOID)out, size, &numberOfBytesRead );
	if ( bReadProcMem != FALSE )
		return true;

	return false;
};

bool RemoteProcessManager::Write( DWORD_PTR dwAddress, void* Data, SIZE_T size )
{
	SIZE_T numberOfBytesRead = 0;

	if ( this->m_hProcessHandle == NULL || 
		 this->m_hProcessHandle == INVALID_HANDLE_VALUE )
		return false;

	BOOL bWriteProcMem = WriteProcessMemory( this->m_hProcessHandle, (PVOID)dwAddress, (PVOID)Data, size, &numberOfBytesRead );
	if ( bWriteProcMem != FALSE )
		return true;

	return false;
};

HANDLE RemoteProcessManager::GetProcessHandle()
{
	return this->m_hProcessHandle;
}

void RemoteProcessManager::WaitForProcessToExit()
{
	WaitForSingleObject( this->m_hProcessHandle, INFINITE );
}