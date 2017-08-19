// Process Injection Practice.cpp : Defines the entry point for the console application.
//

#include <Windows.h>

bool Inject(DWORD pID, char *dllName);

using namespace std;

int main()
{
	//inject into process of your choosing, using DLL with code you would like to inject
	//NOTE: Make sure you pass the entire path to the DLL
	Inject(9460, "C:\\Users\\MalcolmJefferson\\Documents\\Process Injection Practice\\x64\\Debug\\practiceDLL.dll");
    return 0;
}

bool Inject(DWORD pID, char *dllName)
{
	//open a handle to the process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	//if we are able to get a handle to the process
	if (hProcess)
	{
		//LPVOID type (parameter for ThreadProc which needs to be specified to call CreateRemoteThread or CreateThread or CreateRemoteThreadEx) 
		//is the same size as the parameters for LoadLibrary and FreeLibrary and the return values are the same size
		//so we are able to pass a pointer to LoadLibrary/FreeLibrary as the thread routine(kinda like a method specifically for operations involving threads)
		//to CreateRemoteThread.
		//We are using GetModuleHandleA to get a pointer to the kernel32.dll where the LoadLibrary/FreeLibrary functions are located
		//The locations of these functions and this dll never change so we are able to get a valid pointer to our remote process;
		//everytime. And the second argument 'LoadLibraryA' refers to the function we want to get a pointer to
		LPVOID LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
		
		//We are using VirtualAllocEx to allocate memory for the DLL name in the remote process.
		//Passing NULL for the lpAddress causes the function to determine where to allocate the region
		//Passing strlen(dllName) allows us to allocate space equal to the length of the dllname
		//Passing MEM_COMMIT | MEM_RESERVE sets the type of memory allocation to MEM_COMMIT and MEM_RESERVE (see MSDN for def.)
		//Passing PAGE_READWRITE sets the protection level for the memory region that we are allocating in a manner such that we can read and write to that location
		LPVOID allocDllName = VirtualAllocEx(hProcess, NULL, strlen(dllName), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		//write our dllName to the location in memory that we just reserved
		WriteProcessMemory(hProcess, allocDllName, dllName, strlen(dllName), NULL);

		//Create a remote thread(a thread that runs in the address space of another process)
		//Passing NULL causes the function to revert to the default values for those parameters
		//We are casting our LoadLibAddr as the LPTHREADSTARTROUTINE type to define the start address
		HANDLE injectedThreadHandle = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, allocDllName, 0, NULL);
		//get the error code, if 0 then success. Otherwise, there was a problem
		DWORD err = GetLastError();
		//Wait until the remote thread termintates. The thread will terminate as soon as our DllMain
		//called with reason DLL_PROCESS_ATTACH returns
		WaitForSingleObject(injectedThreadHandle, INFINITE);
		//Free the virtual memory that we were using and close our handle to the remote thread
		VirtualFreeEx(hProcess, allocDllName, strlen(dllName), MEM_RELEASE);
		CloseHandle(injectedThreadHandle);
		CloseHandle(hProcess);
		return true;
	}
	return false;
}

