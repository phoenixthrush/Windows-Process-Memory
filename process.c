#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char *targetProcessName = "main.exe";
    const char *searchString = "Hello";
    const char *replaceString = "Beans";
    DWORD processId = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("Failed to create process snapshot.\n");
        return 1;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (strcmp(entry.szExeFile, targetProcessName) == 0) {
                processId = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);

    if (processId == 0) {
        printf("Target process not found.\n");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    if (!hProcess) {
        printf("Failed to open target process.\n");
        return 1;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION memInfo;
    LPVOID address = 0;

    while (address < sysInfo.lpMaximumApplicationAddress) {
        if (VirtualQueryEx(hProcess, address, &memInfo, sizeof(memInfo)) == 0) {
            break;
        }

        if (memInfo.State == MEM_COMMIT && (memInfo.Protect & PAGE_READWRITE)) {
            char *buffer = (char *)malloc(memInfo.RegionSize);
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer, memInfo.RegionSize, &bytesRead)) {
                for (SIZE_T i = 0; i < bytesRead - strlen(searchString); i++) {
                    if (memcmp(buffer + i, searchString, strlen(searchString)) == 0) {
                        SIZE_T bytesWritten;
                        WriteProcessMemory(hProcess, (LPVOID)((char *)memInfo.BaseAddress + i), replaceString, strlen(replaceString), &bytesWritten);
                        printf("String replaced at: %p\n", (char *)memInfo.BaseAddress + i);
                    }
                }
            }
            free(buffer);
        }
        address = (LPVOID)((char *)memInfo.BaseAddress + memInfo.RegionSize);
    }

    CloseHandle(hProcess);
    return 0;
}
