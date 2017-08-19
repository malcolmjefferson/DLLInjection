// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	//if the DLL is able to inject into it's process, then we will create a message box
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		MessageBoxA(0, "Hello from injected DLL!", 0, 0);
		return true;
	}
	return false;
}

