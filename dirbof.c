// dir_bof.c
#include <windows.h>
#include "beacon.h"

// Main function for the BOF
void go(char *args, int len) {
    char *command = "cmd.exe /c dir";
    char buffer[4096];
    DWORD read, written;
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa;
    
    // Set up security attributes for inheritable handles
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create a pipe for the output
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to create pipe.");
        return;
    }

    // Set up the process information structures
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    ZeroMemory(&pi, sizeof(pi));

    // Create the process to run the command
    if (!CreateProcessA(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to create process.");
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return;
    }

    // Close the write end of the pipe in the current process
    CloseHandle(hWrite);

    // Read from the pipe and output to Beacon
    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &read, NULL) && read != 0) {
        buffer[read] = '\0';
        BeaconOutput(CALLBACK_OUTPUT, buffer, read);
    }

    // Clean up handles
    CloseHandle(hRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
