#include "pch.h"

void run_command(const string& cmd, const string& input, string& output, string& error) {
    Pipe childIn(false);
    Pipe childOut(true);
    Pipe childErr(true);

    Handle procHandle = start_command(cmd, childIn, childOut, childErr);

    // Write the entire input to the started process' stdin.
    if (!input.empty()) {
        childOut.write(input);
    }
    childIn.close();

    // Wait for the process to terminate before we read the outputs.
    wait_for_terminate(procHandle);
    close_handle(procHandle);

    // Read the entire stdout and stderr of the child process.
    childOut.flush(output);
    childErr.flush(error);

    childOut.close();
    childErr.close();
}

#ifdef _WIN32
Pipe::Pipe(bool isOutput) {
    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    attributes.bInheritHandle = true;
    attributes.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&readHandle, &writeHandle, &attributes, 0)) {
        throw MetroException("Couldn't create pipe");
    }

    if (!SetHandleInformation(isOutput? readHandle : writeHandle, HANDLE_FLAG_INHERIT, 0)) {
        throw MetroException("Couldn't set pipe handle information");
    }
}

void Pipe::write(const string& input) {
    if (!WriteFile(writeHandle, input.c_str(), input.size(), nullptr, nullptr)) {
        throw MetroException("Couldn't write to pipe");
    }
}

void Pipe::flush(string& out) {
    // Find out how many bytes there are to be read.
    DWORD numBytes;
    if (!PeekNamedPipe(readHandle, nullptr, 0, nullptr, &numBytes, nullptr)) {
        throw MetroException("Pipe peek failed");
    }

    if (numBytes == 0) {
        out = "";
        return;
    }

    // Allocate a buffer large enough to hold the bytes and null terminator.
    char *buffer = new char[numBytes + 1];
    if (!ReadFile(readHandle, buffer, numBytes, nullptr, nullptr)) {
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

Handle start_command(const string& cmd, const Pipe& childIn, const Pipe& childOut, const Pipe& childErr) {
    PROCESS_INFORMATION procInfo;
    ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdError = childErr.writeHandle;
    startupInfo.hStdOutput = childOut.writeHandle;
    startupInfo.hStdInput = childIn.readHandle;
    startupInfo.dwFlags = STARTF_USESTDHANDLES;

    bool success = CreateProcess(nullptr, const_cast<char*>(cmd.c_str()), nullptr, nullptr, true,
                                 0, nullptr, nullptr, &startupInfo, &procInfo);

    if (!success) {
        throw MetroException("Couldn't create process");
    }

    CloseHandle(procInfo.hThread);
    return procInfo.hProcess;
}

#elif __unix__
Pipe::Pipe(bool isOutput) {
    Handle handles[2];
    if (pipe(handles) < 0) {
        throw MetroException("Couldn't create pipe");
    }
    readHandle = handles[0];
    writeHandle = handles[1];

    // Allow non-blocking reading from output pipes.
    if (isOutput) {
        fcntl(readHandle, F_SETFL, O_NONBLOCK);
    }
}

void Pipe::write(const string& input) {
    if (::write(writeHandle, input.c_str(), input.size()) < 0) {
        throw MetroException("Couldn't write to pipe");
    }
}

#define BUFFER_SIZE 4096

void Pipe::flush(string& out) {
    stringstream outStream;
    char buffer[BUFFER_SIZE];
    while (true) {
        int numRead = read(readHandle, buffer, BUFFER_SIZE-1);
        if (numRead <= 0) {
            break;
        }
        // Null-terminate the buffer.
        buffer[numRead] = 0;
        outStream << buffer;
        // If the buffer wasn't filled, then there is no more left to read.
        if (numRead < BUFFER_SIZE-1) {
            break;
        }
    }

    // Securely erase the buffer as it might have held sensitive information.
    memset_volatile(buffer, 0, BUFFER_SIZE);
    out = outStream.str();
}

Handle start_command(const string& cmd, const Pipe& childIn, const Pipe& childOut, const Pipe& childErr) {
    Handle pid = fork();
    if (pid < 0) {
        throw MetroException("Couldn't fork process");
    }

    // Runs in the child process.
    if (!pid) {
        // Set the stdin, stdout, and stderr of the child to our pipes.
        if (dup2(childIn.readHandle, STDIN_FILENO) < 0) {
            cerr << "Couldn't set stdin" << endl;
        }
        if (dup2(childOut.writeHandle, STDOUT_FILENO) < 0) {
            cerr << "Couldn't set stdout" << endl;
        }
        if (dup2(childErr.writeHandle, STDERR_FILENO) < 0) {
            cerr << "Couldn't set stderr" << endl;
        }

        childIn.close();
        childOut.close();
        childErr.close();

        // Split the command string into separate arguments.
        vector<string> args = split_args(cmd);

        // Convert the list of arguments into a C string array.
        // The strings themselves will be copied into a contiguous underlying array.
        unsigned long totalChars = 0;
        for (auto& arg : args) {
            totalChars += arg.size() + 1;
        }
        char underlying[totalChars];

        // A null-terminated array to store the string pointers.
        char *argPtrs[args.size() + 1];
        argPtrs[args.size()] = nullptr;

        // Copy the strings into the underlying array,
        // and set the pointers to the start of each string.
        char *argStart = underlying;
        for (int i = 0; i < args.size(); i++) {
            strcpy(argStart, args[i].c_str());
            argPtrs[i] = argStart;
            argStart += args[i].size() + 1;
        }

        execvp(argPtrs[0], argPtrs);
        // This only runs if the execution failed.
        cerr << "Couldn't execute command" << endl;
        exit(0);
    }

    return pid;
}
#endif

void Pipe::close() const {
    close_handle(readHandle);
    close_handle(writeHandle);
}