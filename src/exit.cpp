#pragma once
#include "pch.h"

#ifdef _WIN32
#define EXIT_FUNC return FALSE
BOOL WINAPI on_application_exit(DWORD signal) {
    if (signal != CTRL_C_EVENT) EXIT_FUNC;
#elif __unix__ || __APPLE__ || __MACH__
#define EXIT_FUNC exit(0); return
void on_application_exit(int sig_num) {
    if (!exit_config.cloning) exit(0);
#endif //_WIN32

    if (!exit_config.cloning) EXIT_FUNC;
    if (!exit_config.started) EXIT_FUNC;
    exit_config.cancel = true;
    while (!exit_config.received);

#ifdef _WIN32
    Sleep(1000);
#elif __unix__
    sleep(1);
#elif __APPLE__ || __MACH__
    sleep(1);
#endif //_WIN32

    using namespace std_filesystem;
    path current = current_path();
    path full(current.string() + "\\" + exit_config.directory);
    remove_all(full); // Remove whether exists or not

    EXIT_FUNC;
}