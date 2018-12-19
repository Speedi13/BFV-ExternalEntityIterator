#pragma once

class RemoteProcessManager
{
public:
	RemoteProcessManager(void);
	~RemoteProcessManager(void);

	DWORD RemoteProcessManager::GetProcessId( char* ProcessName );

	bool RemoteProcessManager::OpenProcess( DWORD dwPid );

	bool RemoteProcessManager::Read(  DWORD_PTR dwAddress, void* out,  SIZE_T size );
	bool RemoteProcessManager::Write( DWORD_PTR dwAddress, void* Data, SIZE_T size );

	HANDLE RemoteProcessManager::GetProcessHandle();

	void RemoteProcessManager::WaitForProcessToExit();

private:

	HANDLE m_hProcessHandle;
	DWORD m_dwProcessId;
};

extern RemoteProcessManager* RPM;