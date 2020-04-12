/**
 * This contains code regarding the way Metro exits.
 */

static struct {
    bool cloning = false;   // True if command used is clone
    string directory;       // Directory being cloned
    bool started = false;   // True if started cloning
    bool cancel = false;    // True if exit has been issued
    bool received = false;  // True if clone had told libgit to cancel
} exit_config;

#ifdef _WIN32
/**
 * Metro callback for Ctrl+C exit.
 * @param signal Signal being sent to callback.
 * @return If the function handles the control signal, it should return TRUE. If it returns FALSE, the next handler
 * function in the list of handlers for this process is used.
 *
 * See <a href="https://docs.microsoft.com/en-us/windows/console/handlerroutine">
 * https://docs.microsoft.com/en-us/windows/console/handlerroutine</a>
 */
BOOL WINAPI on_application_exit(DWORD signal);
#elif __unix__ || __APPLE__ || __MACH__
void on_application_exit(int sig_num);
#endif //_WIN32