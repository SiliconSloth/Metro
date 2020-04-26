// Represents whether there is a progress bar to be cleared
bool progress_bar = false;

struct terminal_options {
    string term;                        // The terminal type, if any
    bool ansi_enabled = true;           // Whether to enable ANSI manually
    bool progress_enabled = true;       // Whether to enable progress bars
    bool ansi_colour_change = false;    // Whether to do colour switching via ANSI
} t_ops;

unsigned int parse_pos_int(const string& str) {
    try {
        int val = stoi(str);
        return val >= 0? val : -1;
    } catch (exception&) {
    }
    return -1;
}

bool has_prefix(string const& str, string pre) {
    if (pre.size() <= str.size()) {
        return str.compare(0, pre.size(), pre) == 0;
    } else {
        return false;
    }
}

bool has_suffix(string const& str, string suff) {
    if (suff.size() <= str.size()) {
        return str.compare(str.size() - suff.size(), suff.size(), suff) == 0;
    } else {
        return false;
    }
}

bool whitespace_only(const string& s) {
    return s.empty() || all_of(s.begin(), s.end(), [](char c){
        return isspace(static_cast<unsigned char>(c));
    });
}

void split_at_first(string const& str, char const& c, string & before, string & after) {
    size_t index = str.find(c);
    const string tempStr((string(str))); // Can't assume str != before
    if (index == -1) {
        before = tempStr;
        after = "";
    } else {
        before = tempStr.substr(0, index);
        after = tempStr.substr(index + 1, string::npos);
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

void print_options(const vector<string>& options) {
    cout << endl << "Options:" << endl;
    for (const auto& name : options) {
        // Find the option whose name matches the one given.
        bool found = false;
        for (const auto& opt : ALL_OPTIONS) {
            if (opt.name == name) {
                print_padded("--" + name, 8);
                print_padded("-" + opt.contraction, 5);
                cout << opt.description << endl;

                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Developer warning: Unknown option \"" << name << "\"" << endl;
        }
    }
}

void print_padded(const string& str, size_t len) {
    cout << str;
    for (size_t i = 0; i < len - str.length(); i++) {
        cout << " ";
    }
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

string time_to_string(git_time time) {
    char buf[80];
    struct tm ts = *localtime(reinterpret_cast<const time_t *>(&time.time));
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y ", &ts); // Format of time

    // Set offset manually via minutes
    int hour_offset = 0;
    int minute_offset = time.offset;
    while (minute_offset >= 60) {
        minute_offset -= 60;
        hour_offset++;
    }

    // Insert offset into string
    char buf2[5];
    sprintf(buf2, "%02d%02d", hour_offset, minute_offset);

    // Insert full time into string
    char buf3[100];
    sprintf(buf3, "%s%c%s", buf, '+', buf2);

    return string(buf3);
}

// Defines the original colour of the console being used before being changed.
static int defaultColour = -1;

void set_text_colour(const string colour, void* handle) {
#ifdef _WIN32
    // WINDOWS COLOUR SELECTION
    if (!t_ops.ansi_colour_change) {
        CONSOLE_SCREEN_BUFFER_INFO term;
        GetConsoleScreenBufferInfo(handle, &term);
        if (defaultColour == -1) defaultColour = term.wAttributes;

        unsigned int current = 0;
        if (colour[0] == 'r') current |= FOREGROUND_RED;
        if (colour[1] == 'g') current |= FOREGROUND_GREEN;
        if (colour[2] == 'b') current |= FOREGROUND_BLUE;
        if (colour[3] == 'i') current |= FOREGROUND_INTENSITY;
        if (colour[4] == 'r') current |= BACKGROUND_RED;
        if (colour[5] == 'g') current |= BACKGROUND_GREEN;
        if (colour[6] == 'b') current |= BACKGROUND_BLUE;
        if (colour[7] == 'i') current |= BACKGROUND_INTENSITY;
        if (colour[8] == 'f') current = (term.wAttributes & 0xF0) | (current & 0x0F);
        if (colour[8] == 'b') current = (current & 0xF0) | (term.wAttributes & 0x0F);
        if (colour[8] == 'r') current = defaultColour;

        SetConsoleTextAttribute(handle, current);
        return;
    }
#endif //_WIN32

    // ANSI COLOUR SELECTION
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
}

/**
 * Prints a progress bar of the given width.
 * @param progress A percentage between 0 and 1 of how far along the progress bar is.
 * @param width Width of the progress bar in characters.
 */
void print_progress_width(unsigned int progress, unsigned int width) {
    if (width > 250) width = 250; // Set max possible size limit for progress bar if massive console.

    // TODO Width isn't actually the final length
    cout << "\r" << "Progress: [" << flush;
    int i;
    if (width > 0) {
        for (i = 0; i < (progress * width) / 100; i++) {
            cout << "=" << flush;
        }
        for (; i < width; i++) {
            cout << "-" << flush;
        }
        cout << "] " << progress << "%" << flush;
    }
}

void print_progress(unsigned int progress) {
    if (!t_ops.progress_enabled) return;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#elif __unix__  || __APPLE__ || __MACH__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
#endif

    print_progress_width(progress, width - 17);
    progress_bar = true;
}

static unsigned int progress_count;

void print_progress(unsigned int progress, size_t bytes) {
    if (!t_ops.progress_enabled) return;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int width = csbi.srWindow.Right - csbi.srWindow.Left;

    chrono::time_point<chrono::steady_clock> time = std::chrono::high_resolution_clock::now();
    static chrono::time_point<chrono::steady_clock> last_time;
#elif __unix__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;

    chrono::system_clock::time_point time = std::chrono::high_resolution_clock::now();
    static chrono::system_clock::time_point last_time;
#elif __APPLE__ || __MACH__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;

    chrono::time_point<chrono::steady_clock> time = std::chrono::high_resolution_clock::now();
    static chrono::time_point<chrono::steady_clock> last_time;
#endif

    static size_t average_speed;
    static size_t last_bytes;
    size_t total_speed;

    if (last_bytes > bytes) progress_count = 0;

    // TODO Reset static variables at end of transfer.
    // Finds the total average speed over file transfer/
    // TODO Average speed should consider older speeds less strongly
    if (progress_count != 0) {
        float delta_time = std::chrono::duration_cast<std::chrono::microseconds>(time - last_time).count();
        // If time too small, assume minimum possible
        if (delta_time == 0) {
            delta_time = 1;
        }
        size_t current_speed = 1000000.f * (float) (bytes - last_bytes) / delta_time;
        total_speed = (current_speed + (average_speed * progress_count)) / (progress_count + 1);
    } else {
        total_speed = 0;
    }
    last_time = time;
    average_speed = total_speed;
    progress_count++;
    last_bytes = bytes;

    int max_width = 8; //xxx.xxYB
    string size = bytes_to_string(bytes);
    string speed = bytes_to_string(total_speed);
    std::stringstream size_space;
    std::stringstream speed_space;
    for (long i = size.length(); i < max_width; i++) {
        size_space << " ";
    }
    for (long i = speed.length(); i < max_width; i++) {
        speed_space << " ";
    }

    print_progress_width(progress, width - 40);
    cout << " | " << size << " | " << speed << "/s" << size_space.str() << speed_space.str() << flush;
    progress_bar = true;
}

void clear_progress_bar() {
    if (!t_ops.progress_enabled) return;
    enable_ansi();
    if (progress_bar) {
        cout << "\033[1A";
        clear_line();
        progress_bar = false;
        progress_count = 0;
    }
    disable_ansi();
}

void clear_line() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#elif __unix__ || __APPLE__ || __MACH__
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

string bytes_to_string(size_t bytes) {
    stringstream stream;
    stream << std::fixed << setprecision(2);
    if (bytes < 1000) {
        stream << bytes;
        return  stream.str() + "B";
    } else if (bytes < 1000000) {
        stream << (float) bytes / 1000.0f;
        return  stream.str() + "KB";
    } else if (bytes < 1000000000) {
        stream << (float) bytes / 1000000.0f;
        return  stream.str() + "MB";
    } else if (bytes < 1000000000000L) {
        stream << (float) bytes / 1000000000.0f;
        return  stream.str() + "GB";
    } else if (bytes < 1000000000000000L) {
        stream << (float) bytes / 1000000000000.0f;
        return  stream.str() + "TB";
    }
}

// Saves initial console mode
#ifdef _WIN32
static HANDLE sout;
static DWORD initial;
#endif //_WIN32

void enable_ansi() {
    if (!t_ops.ansi_enabled) return;
#ifdef _WIN32
    DWORD mode = 0;
    sout = GetStdHandle(STD_OUTPUT_HANDLE);

    if(sout == INVALID_HANDLE_VALUE) throw ANSIException();
    if(!GetConsoleMode(sout, &mode)) throw ANSIException();
    initial = mode;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if(!SetConsoleMode(sout, mode)) throw ANSIException();
#endif //_WIN32
}

void disable_ansi() {
    if (!t_ops.ansi_enabled) return;
#ifdef _WIN32
    printf("\x1b[0m"); // Reset all attributes
    if(!SetConsoleMode(sout, initial)) throw ANSIException();
#endif //_WIN32
}

string get_env(string name) {
#ifdef _WIN32
    char temp[50];
    temp[0] = '\0';
    const int actual = GetEnvironmentVariable(name.c_str(), temp, 51);
    if (actual <= 50) return string(temp);
    char *temp1 = new char[actual];
    temp1[0] = '\0';
    GetEnvironmentVariable(name.c_str(), temp, actual+1);
    string final(temp1);
    return final;
#elif __unix__ || __APPLE__ || __MACH__
    return string(getenv(name.c_str()));
#endif
}