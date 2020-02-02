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