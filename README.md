# RemoteFreeLibrary
Unload Remote Library with FreeLibrary

The goal of this project is to demostrate a way to unload a remote DLL from a target process in Windows environment, and with same privilege(needed for PROCESS_VM_OPERATION and PROCESS_VM_WRITE).

All execution is (very) simple, we just need to find the base address of given module(HMODULE), and start a remote call of FreeLibrary function with that base address as a argument. 
