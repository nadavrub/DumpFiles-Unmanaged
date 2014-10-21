#include "stdafx.h"
#include "windows.h"
#include "Psapi.h"
#include "imagehlp.h"

#pragma comment( lib, "Psapi.lib" )
#pragma comment( lib, "Dbghelp.lib" )

namespace Dumps {
	const MINIDUMP_TYPE g_eDumpType	= (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithProcessThreadData);

	DWORD WINAPI __GenerateDumpFile(IN LPEXCEPTION_POINTERS pExeption) {
		HANDLE		hProcess				= GetCurrentProcess();
		DWORD		dwProcessId				= GetCurrentProcessId();
		DWORD		hrDump					= S_OK;
		SYSTEMTIME	st;
		WCHAR		pDumpName[MAX_PATH*2]	= { 0 };
		WCHAR		pModulePath[MAX_PATH]	= { 0 };
		PWCHAR		pPtr					= pModulePath;
		PWCHAR		pModuleName				= pModulePath;
		LONGLONG	llNow;

		GetSystemTime(&st);
		SystemTimeToFileTime(&st, (LPFILETIME)&llNow);
		if(0 == GetModuleFileNameEx(hProcess, 0, pModulePath, sizeof(pModulePath)/sizeof(pModulePath[0])))
			return HRESULT_FROM_WIN32(GetLastError());
		while(0 != *pPtr) {
			if('\\' == *pPtr)
				pModuleName = pPtr + 1;
			pPtr++;

		}
		pModuleName[-1] = 0;// Terminate the 'pModulePath'.
		swprintf_s(pDumpName, L"%ls\\%.2i-%.2i-%.4i-%X-0x%.8X-%ls.dmp", pModulePath, st.wMonth, st.wDay, st.wYear
			, (LONG)(llNow % 10000000I64*60*60*24), pExeption ? pExeption->ExceptionRecord->ExceptionCode : 0, pModuleName);
		HANDLE hFile = CreateFile(pDumpName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if(INVALID_HANDLE_VALUE == hFile)
			return HRESULT_FROM_WIN32(GetLastError());
		MINIDUMP_EXCEPTION_INFORMATION infoException = { GetCurrentThreadId(), pExeption, TRUE };
		if(FALSE == MiniDumpWriteDump(hProcess, dwProcessId, hFile, g_eDumpType, pExeption ? &infoException : 0, 0, 0)) {
			hrDump = HRESULT_FROM_WIN32(GetLastError());
			DeleteFile(pDumpName);
		}
		CloseHandle(hFile);
		return hrDump;
	}

	LONG WINAPI __TopLevelExceptionHandler(EXCEPTION_POINTERS *pException) {
		if( (FALSE == IsDebuggerPresent()) && 
			// Avoid generating dumps on _ASSERTs
			(EXCEPTION_BREAKPOINT != pException->ExceptionRecord->ExceptionCode) && 
			(EXCEPTION_SINGLE_STEP != pException->ExceptionRecord->ExceptionCode) ) 
		{
			// Must create a specialized thread to support edge case stack overflow scenarios
			HANDLE hDumpThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)__GenerateDumpFile, pException, 0, 0);
			if(0 != hDumpThread) {
				WaitForSingleObject(hDumpThread, INFINITE);
				CloseHandle(hDumpThread);
			}
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void GenerateDumpOnUnhandledExceptions(IN BOOL bGenerate) {
		SetUnhandledExceptionFilter((TRUE == bGenerate) ? __TopLevelExceptionHandler : NULL);
	}
}

#pragma warning( disable : 4717 )
void StackOverflow() {
	StackOverflow();
}
#pragma warning( default : 4717 )

void AccessViolation() {
	*((PDWORD)0) = 8;
}

int _tmain(int argc, _TCHAR* argv[])
{
	Dumps::GenerateDumpOnUnhandledExceptions(TRUE);
//	AccessViolation();
	StackOverflow();
	return 0;
}

