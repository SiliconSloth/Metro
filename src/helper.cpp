#include "pch.h"

using namespace std;

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

string read_password() {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif //_WIN32

    string input;
    getline(cin, input);
    cout << endl;

#ifdef _WIN32
    SetConsoleMode(hStdin, mode);
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif //_WIN32

    return input;
}