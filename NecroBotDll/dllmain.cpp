// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

DWORD WINAPI runBot(LPVOID lpParam)
{
	MessageBoxA(NULL, "lol\n", "lol", MB_OK | MB_TOPMOST);
	return 1;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		MessageBoxA(NULL, "DLL Attached!\n", "leet h4x0r", MB_OK | MB_TOPMOST);
		CreateThread(NULL, 0, &runBot, NULL, 0, NULL);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

