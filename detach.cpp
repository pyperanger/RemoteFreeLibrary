#include <windows.h>
#include <iostream>
#include <psapi.h>

#define endl "\n";


DWORD UnloadDll(DWORD TargetId, std::wstring DLL_NAME)
{
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, TargetId);
    if (process == INVALID_HANDLE_VALUE)
        return 3;

    HMODULE modhandles[1024];
    DWORD nb;
    if (!EnumProcessModulesEx(process, modhandles, sizeof(modhandles), &nb, LIST_MODULES_ALL))
    {
        CloseHandle(process);
        return 4;
    }

    unsigned char* TARGET_REMOTE_BASE_ADDRESS = 0x0;

    for (size_t i = 0; i < (nb / sizeof(HMODULE)); i++)
    {
        TCHAR modname[MAX_PATH];
        if (GetModuleFileNameEx(process, modhandles[i], modname, sizeof(modname) / sizeof(TCHAR)))
        {
            if (wcsstr(modname, DLL_NAME.c_str()))
            {
                MEMORY_BASIC_INFORMATION modinfo;
                VirtualQueryEx(process, modhandles[i], &modinfo, sizeof(modinfo));
                std::wcout << DLL_NAME << " Base Address: " << modinfo.BaseAddress << endl;
                std::wcout << "Full Path: " << modname << endl;
                TARGET_REMOTE_BASE_ADDRESS = (unsigned char*)modinfo.BaseAddress;
            }
        }
    }   

    if (TARGET_REMOTE_BASE_ADDRESS == 0x0)
        return 5;
        
    // Create Remote Library Caller
    LPTHREAD_START_ROUTINE freelib_caller = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
    if (!freelib_caller)
        return 8;

    std::cout << "FreeLibrary Caller created at remote process: " << (PVOID)freelib_caller << endl;

    // Create and Start Remote freelib inside target process
    HANDLE thread = CreateRemoteThread(process, NULL, 0, freelib_caller, TARGET_REMOTE_BASE_ADDRESS, 0, NULL);
    if (!thread)
        return 9;

    WaitForSingleObject(thread, INFINITE);
        
    DWORD exitdw = 0;
    if (!GetExitCodeThread(thread, &exitdw))
        return 10;

    CloseHandle(thread);
    CloseHandle(process);

    return 1;
}

int
main(int argc, char**argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " (PID)1337 " << " (DLLNAME)lib.dll" << endl;
        return 1;
    }

    DWORD TargetId = atoi(argv[1]);

    std::cout << "Target PID: " << TargetId << endl;
    std::cout << "Target Lib: " << argv[2] << endl;
         
    DWORD code = UnloadDll(TargetId, std::wstring(argv[2], argv[2] + strlen(argv[2])));
    if (code)
    {
        std::cout << "Successful Unload!" << endl;
    }
    else {
        std::cout << "Unload Error: " << code << endl;
    }


    return 0;
}