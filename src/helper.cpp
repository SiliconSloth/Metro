#include "pch.h"

using namespace std;

// Convert a string to a non-negative integer, returning -1 on failure.
int parse_pos_int(const string& str) {
    try {
        int val = stoi(str);
        return val >= 0? val : -1;
    } catch (exception&) {
    }
    return -1;
}

bool has_prefix(string const& str, string const& pre) {
    if (pre.size() <= str.size()) {
        return str.compare(0, pre.size(), pre) == 0;
    } else {
        return false;
    }
}

bool has_suffix(string const& str, string const& suff) {
    if (suff.size() <= str.size()) {
        return str.compare(str.size() - suff.size(), suff.size(), suff) == 0;
    } else {
        return false;
    }
}

// True if the string contains no non-whitespace characters.
bool whitespace_only(const string& s) {
    return s.empty() || all_of(s.begin(), s.end(), [](char c){
        return isspace(static_cast<unsigned char>(c));
    });
}

// Split the given string around the fist occurrence of the given character.
// If the character is not found, the input string is returned as the first string and "" as the second.
// The outputs are stored in before and after.
void split_at_first(string const& str, char const& c, string & before, string & after) {
    size_t index = str.find(c);
    if (index == -1) {
        before = str;
        after = "";
    } else {
        before = str.substr(0, index);
        after = str.substr(index + 1, string::npos);
    }
}

vector<string> split_args(const string& command) {
    vector<string> args;
    stringstream arg;
    bool quoted = false;
    // Count the number of consecutive backslashes.
    int backslashes = 0;
    for (auto& c : command) {
        if (c == '\\') {
            backslashes++;
        } else if (c == '\"') {
            // Treat a series of 2n consecutive backslashes before a "
            // as n escaped backslashes.
            for (; backslashes > 0; backslashes -= 2) {
                arg << '\\';
            }
            // If there are no unescaped slashes then do not escape the ".
            if (backslashes == 0) {
                quoted = !quoted;
            } else {
                // If there was a backslash left over then the " was escaped.
                arg << '\"';
                backslashes = 0;
            }
        } else if (c == ' ') {
            // Inside quotes spaces are escaped, otherwise they denote the end of an argument.
            if (quoted) {
                arg << ' ';
            } else {
                string argStr = arg.str();
                if (!argStr.empty()) {
                    args.push_back(argStr);
                    arg = stringstream();
                }
            }
        } else {
            // Sequences of backslashes are not considered escaped if not followed by ".
            for (; backslashes > 0; backslashes--) {
                arg << '\\';
            }
            arg << c;
        }
    }

    // Add any remaining backslashes.
    for (; backslashes > 0; backslashes--) {
        arg << '\\';
    }

    // Add the last argument if non-empty.
    string argStr = arg.str();
    if (!argStr.empty()) {
        args.push_back(arg.str());
    }
    return args;
}

string read_all(const string& path) {
    //TODO: Exception if the file is not found/read fails
    ifstream file(path);
    string message((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return message;
}

void write_all(const string& text, const string& path) {
    //TODO: Exception if the file is not found/write fails
    ofstream file(path);
    file << text;
    file.close();
}

string time_to_string(Time time) {
    char buf[80];
    struct tm ts = *localtime(&time.time);
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y ", &ts);

    int hour_offset = 0;
    int minute_offset = time.offset;
    while (minute_offset >= 60) {
        minute_offset -= 60;
        hour_offset++;
    }

    char buf2[5];
    sprintf(buf2, "%02d%02d", hour_offset, minute_offset);

    char buf3[100];
    sprintf(buf3, "%s%c%s", buf, '+', buf2);

    return string(buf3);
}

// Should be in format like "rgbi-----" or "r--i-gb--"
// rgb is colour, i is intensity. The first 4 are the
// text, and the second 4 are the background. The last
// one is the mode: - for all, f for foreground, b for
// background and r for reset (UNIX only)
void set_text_colour(string colour, void* handle) {
#ifdef _WIN32
    int current = 0;
    if (colour[0] == 'r') current |= FOREGROUND_RED;
    if (colour[1] == 'g') current |= FOREGROUND_GREEN;
    if (colour[2] == 'b') current |= FOREGROUND_BLUE;
    if (colour[3] == 'i') current |= FOREGROUND_INTENSITY;
    if (colour[4] == 'r') current |= BACKGROUND_RED;
    if (colour[5] == 'g') current |= BACKGROUND_GREEN;
    if (colour[6] == 'b') current |= BACKGROUND_BLUE;
    if (colour[7] == 'i') current |= BACKGROUND_INTENSITY;

    SetConsoleTextAttribute(handle, current);
#elif __unix__
    if (colour[8] == 'r') {
        printf("\033[0m");
        return;
    }
    std::string col;
    std::string bri;
    std::string b_col;
    std::string b_bri;
    if (colour[0] == 'r') {
        if (colour[1] == 'g') {
            if (colour[2] == 'b') {
                col = "37";
            } else {
                col = "33";
            }
        } else {
            if (colour[2] == 'b') {
                col = "35";
            } else {
                col = "31";
            }
        }
    } else {
        if (colour[1] == 'g') {
            if (colour[2] == 'b') {
                col = "36";
            } else {
                col = "32";
            }
        } else {
            if (colour[2] == 'b') {
                col = "34";
            } else {
                col = "30";
            }
        }
    }
    if (colour[3] == 'i') bri = "1";
    else bri = "0";
    if (colour[4] == 'r') {
        if (colour[5] == 'g') {
            if (colour[6] == 'b') {
                b_col = "37";
            } else {
                b_col = "33";
            }
        } else {
            if (colour[6] == 'b') {
                b_col = "35";
            } else {
                b_col = "31";
            }
        }
    } else {
        if (colour[5] == 'g') {
            if (colour[6] == 'b') {
                b_col = "36";
            } else {
                b_col = "32";
            }
        } else {
            if (colour[6] == 'b') {
                b_col = "34";
            } else {
                b_col = "30";
            }
        }
    }
    if (colour[7] == 'i') b_bri = "1";
    else b_bri = "0";
    if (colour[8] == '-' || colour[8] == 'f') printf("\033[%s;%sm", bri.c_str(), col.c_str());
    if (colour[8] == '-' || colour[8] == 'b') printf("\033[%s;%sm", b_bri.c_str(), b_col.c_str());
#endif //_WIN32
}

void print_progress(int progress) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int width = csbi.srWindow.Right - csbi.srWindow.Left - 16;
#elif __unix__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col - 17;
#endif

    string bar;
    int i;
    if (width > 0) {
        for (i = 0; i < (progress * width) / 100; i++) {
            bar.append("=");
        }
        for (; i < width; i++) {
            bar.append("-");
        }
        cout << "\r" << "Progress: [" << bar << "] " << progress << "%" << flush;
    }
}

void clear_line() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#elif __unix__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
#endif

    cout << "\r";
    for (int i = 0; i < width; i++) {
        cout << " ";
    }
    cout << "\r";
}