static struct {
    bool cloning = false; // True if command used is clone
    string directory; // Directory being cloned
    bool started = false; // True if started cloning
    bool cancel = false; // True if exit has been issued
    bool received = false; // True if clone had told libgit to cancel
} exit_config;

#ifdef _WIN32
BOOL WINAPI on_application_exit(DWORD signal);
#elif __unix__ || __APPLE__ || __MACH__
void on_application_exit(int sig_num);
#endif //_WIN32