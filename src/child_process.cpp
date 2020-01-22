#include "pch.h"

void run_command(const string& cmd, const string& input, string& output, string& error) {
    Pipe childIn(false);
    Pipe childOut(true);
    Pipe childErr(true);

    HANDLE procHandle = start_command(cmd, childIn, childOut, childErr);

    // Write the entire input to the started process' stdin.
    bool success = WriteFile(childIn.write, input.c_str(), input.size(), nullptr, nullptr);
    childIn.close();
    if (!success) {
        throw MetroException("Writing process input failed");
    }

    // Wait for the process to terminate before we read the outputs.
    WaitForSingleObject(procHandle, INFINITE);
    CloseHandle(procHandle);

    // Read the entire stdout and stderr of the child process.
    childOut.flush(output);
    childErr.flush(error);

    childOut.close();
    childErr.close();
}

Pipe::Pipe(bool isOutput) {
    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    attributes.bInheritHandle = true;
    attributes.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&read, &write, &attributes, 0)) {
        throw MetroException("Couldn't create pipe");
    }

    if (!SetHandleInformation(isOutput? read : write, HANDLE_FLAG_INHERIT, 0)) {
        throw MetroException("Couldn't set pipe handle information");
    }
}

void Pipe::flush(string& out) {
    // Find out how many bytes there are to be read.
    DWORD numBytes;
    if (!PeekNamedPipe(read, nullptr, 0, nullptr, &numBytes, nullptr)) {
        throw MetroException("Pipe peek failed");
    }

    if (numBytes == 0) {
        out = "";
        return;
    }

    // Allocate a buffer large enough to hold the bytes and null terminator.
    char *buffer = new char[numBytes + 1];
    if (!ReadFile(read, buffer, numBytes, nullptr, nullptr)) {
        delete[] buffer;
        throw MetroException("Output read failed");
    }

    // Add a null terminator character.
    buffer[numBytes] = 0;
    out = string(buffer);
    // The command output might contain sensitive information, so erase it from memory after the above copy.
    SecureZeroMemory(buffer, numBytes + 1);
    delete[] buffer;
}

void Pipe::close() {
    CloseHandle(read);
    CloseHandle(write);
}

HANDLE start_command(const string& cmd, const Pipe& childIn, const Pipe& childOut, const Pipe& childErr) {
    PROCESS_INFORMATION procInfo;
    ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdError = childErr.write;
    startupInfo.hStdOutput = childOut.write;
    startupInfo.hStdInput = childIn.read;
    startupInfo.dwFlags = STARTF_USESTDHANDLES;

    bool success = CreateProcess(nullptr, const_cast<char*>(cmd.c_str()), nullptr, nullptr, true,
                                 0, nullptr, nullptr, &startupInfo, &procInfo);

    if (!success) {
        throw MetroException("Couldn't create process");
    }

    CloseHandle(procInfo.hThread);
    return procInfo.hProcess;
}