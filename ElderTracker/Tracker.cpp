#include <iostream>
#include <cstdio>
#include <thread>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

void scanProcessModules(DWORD dwPID);
void printError(TCHAR* msg);

int main( void ){
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	TCHAR toFind[] = _T("The Elder Scrolls Legends.exe");
	//Get a snapshot of all currently running processes
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	//If there is a process:
	if (Process32First(snapshot, &entry) == TRUE){
		//While there are more processes:
		while (Process32Next(snapshot, &entry) == TRUE){
			//If the exeFile name is equal to the correct file name:
			if (_tcscmp(entry.szExeFile, toFind) == 0){
				std::cout << "Found TES" << std::endl;
				HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, entry.th32ProcessID);
				if (hProcess == INVALID_HANDLE_VALUE) {
					std::cout << "Couldn't Open Process" << std::endl;
					return 1;
				}
				std::thread scan(scanProcessModules, entry.th32ProcessID);
				scan.join();
				CloseHandle(hProcess);
				break;
			}
		}
	}

	CloseHandle(snapshot);
	return 0;
}

void scanProcessModules(DWORD dwPID) {
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	//Take a snapshot of all modules in the specified process.
		hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of modules)"));
		return;
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{
		printError(TEXT("Module32First"));  // show cause of failure
		CloseHandle(hModuleSnap);           // clean the snapshot object
		return;
	}


	TCHAR toFind[] = _T("The Elder Scrolls Legends.exe");
	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		if (_tcscmp(me32.szModule, toFind) == 0) {
			_tprintf(TEXT("\nBase address = 0x%08X\n"), (DWORD)me32.modBaseAddr);
			break;
		}

	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
}

void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}
