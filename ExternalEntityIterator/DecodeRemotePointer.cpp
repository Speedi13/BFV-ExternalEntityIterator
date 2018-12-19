#include <Windows.h>
#include "DecodeRemotePointer.h"
typedef 
enum _PROCESSINFOCLASS
{
    ProcessBasicInformation,
	//...
    ProcessCookie = 36
} PROCESSINFOCLASS;
 

#pragma comment(lib, "ntdll.lib")
extern "C" NTSYSCALLAPI NTSTATUS NTAPI NtQueryInformationProcess( IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL );
 
#pragma code_seg(push, ".text")
__declspec(allocate(".text"))
    UCHAR ___ROR8__[] = { 
 0x53,				//- push rbx
 0x48, 0x8B, 0xDA,	//- mov rbx,rdx
 0x48, 0xD3, 0xCB,	//- ror rbx,cl
 0x48, 0x8B, 0xC3,	//- mov rax,rbx
 0x5B,				//- pop rbx
 0xC3,				//- ret 
};
#pragma code_seg()
typedef DWORD64 ( __fastcall* t_ROR8 )( DWORD64, DWORD64 );
t_ROR8 __ROR8__ = (t_ROR8)&___ROR8__;
 
ULONG GetProcessCookie( HANDLE hProcessHandle )
{
	ULONG dwProcessCookie = NULL;
	NTSTATUS NtStatus = NtQueryInformationProcess( hProcessHandle, ProcessCookie, &dwProcessCookie, 4, NULL );
	if ( NtStatus > NULL )
		return NULL;
 
	return dwProcessCookie;
}
DWORD64 DecodePointerExternal( HANDLE hProcessHandle, DWORD64 EncodedPointer )
{
	static ULONG dwProcessCookie = NULL;
	if (dwProcessCookie == NULL)
		dwProcessCookie = GetProcessCookie( hProcessHandle );
 
	return __ROR8__(0x40 - (dwProcessCookie & 0x3F),EncodedPointer) ^ dwProcessCookie;
}
