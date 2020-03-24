#ifdef _WIN32
#define Handle HANDLE
#define wait_for_terminate(procHandle) WaitForSingleObject(procHandle, INFINITE)
#define close_handle(handle) CloseHandle(handle)
#elif __unix__ || __APPLE__ || __MACH__
#define Handle int
#define wait_for_terminate(procHandle) waitpid(procHandle, nullptr, 0)
#define close_handle(handle) ::close(handle)
#endif

struct Pipe {
    Handle readHandle;
    Handle writeHandle;

    Pipe(bool isOutput);

    void write(const string& input);
    void flush(string& out);
    void close() const;
};

/*
 * Run the specified command, write the given string to its stdin, wait for the process to terminate
 * then return the full contents of its stdout and stderr.
 */
void run_command(const string& cmd, const string& input, string& output, string& error);

/*
 * Start a command in a new process using the given pipes for stdin, stdout and stderr.
 * Returns a handle to the running process.
 */
Handle start_command(const string& cmd, const Pipe& childIn, const Pipe& childOut, const Pipe& childErr);