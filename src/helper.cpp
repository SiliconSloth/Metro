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
    sprintf(buf3, "%s%c%s", buf, time.sign, buf2);

    return string(buf3);
}

// Should be in format like "rgbi----" or "r--i-gb-"
// rgb is colour, i is intensity. The first 4 are the
// text, and the second 4 are the background
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
#endif //_WIN32
}