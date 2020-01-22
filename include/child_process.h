struct Pipe {
    HANDLE read;
    HANDLE write;

    Pipe(bool isOutput);

    void flush(string& out);
    void close();
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
HANDLE start_command(const string& cmd, const Pipe& childIn, const Pipe& childOut, const Pipe& childErr);