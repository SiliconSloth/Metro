#include "pch.h"

Command sinkCmd{
        "sink",
        "Sinks Metro",

        // execute
        [](const Arguments& args) {
#ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#elif __unix__
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            int height = w.ws_row;
#endif

            u_int wait_time = 70000;
            int length = height - 6;
//            cout << "\033[2J\033[;H" << flush;
            for (int i = 0; i < length + 4; i++) cout << endl;
            cout << "\033[" + to_string(length + 2) + "B";
            print_progress(0);
            cout << "\033[1000D\033[" + to_string(length + 2) + "A" << flush;
            cout << "           metro sink" << flush;
            usleep(wait_time);
            cout << "\r                      " << endl;
            cout << "           metro sink" << flush;

            for (int i = 0; i < length; i++) {
                usleep(wait_time);
                cout << "\r                       " << endl;
                cout << "           metro sink" << flush;
                cout << "\033[100D\033[" + to_string(length - i) + "B";
                print_progress(((i + 1) * 100) / length);
                cout << "\033[100D\033[" + to_string(length - i) + "A" << flush;
            }
            usleep(wait_time);
            cout << endl << endl << "Successfully sank Metro." << endl << "Did you mean \"metro sync\"?" << endl;
//            cout << "\033[2J\033[;H" << flush;
        },

    // printHelp
    [](const Arguments& args) {
        cout << "Usage: metro sink\n";
    }
};

