/*
 * This code allows easy creation of sub-processes.
 */

#pragma once

#ifdef _WIN32
#define Handle HANDLE
#define wait_for_terminate(procHandle) WaitForSingleObject(procHandle, INFINITE)
#define close_handle(handle) CloseHandle(handle)
#elif __unix__ || __APPLE__ || __MACH__
#define Handle int
#define wait_for_terminate(procHandle) waitpid(procHandle, nullptr, 0)
#define close_handle(handle) ::close(handle)
#endif

// Pipe to run external commands
struct Pipe {
    Handle readHandle;                  // Read handle for child process
    Handle writeHandle;                 // Write handle for child process

    /**
     * Creates a pipe to a child process.
     * @param isOutput Whether the pipe should receive output TODO Check this
     */
    Pipe(bool isOutput);

    /**
     * Writes the input to the pipe
     * @param input Input string to write
     */
    void write(const string& input);
    /**
     * Flushes the output of the command into the out variable.
     * @param out The output of the command between now and last flush.
     */
    void flush(string& out);
    /**
     * Closes the pipe
     */
    void close() const;
};

/**
 * Run the specified command, write the given string to its stdin, wait for the process to terminate
 * then return the full contents of its stdout and stderr.
  * @param cmd Command reference to run in child process.
  * @param input The input reference to provide to the command.
  * @param output The string reference to output the standard output of running to command to.
  * @param error The string reference to output the standard error of running the command to.
  */
void run_command(const string& cmd, const string& input, string& output, string& error);

/**
 * Start a command in a new process using the given pipes for stdin, stdout and stderr.
 * Returns a handle to the running process.
  * @param cmd Command reference to start in child process.
  * @param childIn The pipe to receive input from.
  * @param childOut The pipe to return standard output to.
  * @param childErr The pipe to return standard error to.
  * @return The handle of the child process started.
  */
Handle start_command(const string& cmd, const Pipe& childIn, const Pipe& childOut, const Pipe& childErr);