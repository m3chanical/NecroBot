// NecroBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "memory.h"

constexpr auto NECRODANCER = L"NecroDancer.exe";

DWORD get_game_pid(void);
DWORD get_game_base(HANDLE process);
DWORD get_process_thread_id(HANDLE process);
void load_dll(HANDLE process, const wchar_t* dll_path);

int main()
{
	DWORD pid, new_base, thread_id;
	HANDLE process;
	
	if((pid = get_game_pid()) == 0)
	{
		printf("Could not find NecroDancer pid. Exiting.\n");
		return -1;
	}


	process = OpenProcess(
				PROCESS_VM_OPERATION | // memory access
				PROCESS_VM_READ |
				PROCESS_VM_WRITE,
				FALSE,
				pid
				);
	if(process == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open PID %d, error code %d", pid, GetLastError());
		return -1;
	}

	new_base = get_game_base(process);
	thread_id = get_process_thread_id(process);
	
	printf("NecroDancer PID:\t%d\n", pid);
	printf("The base of the game is:\t%x\n", new_base);
	printf("The thread id is:\t%d\n", thread_id);

	unsigned long is_debug = new_base + 0x435c1e;
	byte value = read_memory<byte>(process, LPVOID(is_debug));

	printf("necrodancer is_debug:\t%x\n", value);

	// Right now this is taken care of by the dll injection:
	
	//printf("engaging necrodancer debug...\n");
	//write_memory<byte>(process, LPVOID(is_debug), 1);

	printf("injecting dll...\n");
	load_dll(process, L"C:\\Users\\ad0ra\\source\\repos\\NecroBot\\Debug\\NecroBotDll.dll"); // need full path for necrobotdll?

	CloseHandle(process);
	return 0;
}

DWORD get_game_pid()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot =
		CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			std::wstring bin_path = entry.szExeFile;
			if(bin_path.find(NECRODANCER) != std::wstring::npos)
			{
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		}
	}
	// if we don't find it
	CloseHandle(snapshot);
	return NULL;
}

DWORD get_game_base(HANDLE process)
{
	DWORD new_base;
	HMODULE k32 = GetModuleHandle(L"kernel32.dll");

	if (k32 == nullptr)
	{
		printf("could not get handle to kernel32\n");
		return -1;
	}

	LPVOID func_addr = GetProcAddress(k32, "GetModuleHandleA");
	if (!func_addr)
		func_addr = GetProcAddress(k32, "GetModuleHandleW");

	HANDLE thread =
		CreateRemoteThread(process, NULL, NULL,
		(LPTHREAD_START_ROUTINE)func_addr,
			NULL, NULL, NULL);
	if (thread == nullptr)
	{
		printf("could not create remote thread\n");
		return -1;
	}
	WaitForSingleObject(thread, INFINITE);
	GetExitCodeThread(thread, &new_base);
	CloseHandle(thread);
	return new_base;
}

DWORD get_process_thread_id(HANDLE process)
{
	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (Thread32First(snapshot, &entry) == TRUE)
	{
		DWORD pid = GetProcessId(process);
		while(Thread32Next(snapshot, &entry) == TRUE)
		{
			if(entry.th32OwnerProcessID == pid)
			{
				CloseHandle(snapshot);
				return entry.th32ThreadID;
			}
		}
	}
	CloseHandle(snapshot);
	return NULL;
}

void load_dll(HANDLE process, const wchar_t* dll_path)
{
	int namelen = wcslen(dll_path) + 1;
	LPVOID remote_str = VirtualAllocEx(process, NULL, namelen * 2, MEM_COMMIT, PAGE_EXECUTE);

	if(remote_str == nullptr)
	{
		return;
	}

	WriteProcessMemory(process, remote_str, dll_path, namelen * 2, NULL);

	HMODULE k32 = GetModuleHandleA("kernel32.dll");
	LPVOID func_addr = GetProcAddress(k32, "LoadLibraryW");

	if(func_addr == nullptr)
	{
		return;
	}

	HANDLE thread =
		CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)func_addr, remote_str, NULL, NULL);

	if(thread == nullptr)
	{
		return;
	}

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
}
