#pragma once
#include <windows.h>

// Example usage of templated mem read/write:
// 
//	DWORD value = read_memory<DWORD>(process, addr);
//	write_memory<DWORD>(process, addr, 0);

template<typename T>
T read_memory(HANDLE proc, LPVOID addr)
{
	T val;
	ReadProcessMemory(proc, addr, &val, sizeof(T), NULL);
	return val;
}

template<typename T>
void write_memory(HANDLE proc, LPVOID addr, T val)
{
	WriteProcessMemory(proc, addr, &val, sizeof(T), NULL);
}

template<typename T>
DWORD protect_memory(HANDLE proc, LPVOID addr, DWORD prot)
{
	DWORD old_prot;
	VirtualProtectEx(proc, addr, sizeof(T), prot, &old_prot);
	return old_prot;
}

