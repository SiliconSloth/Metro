#include "pch.h"

using namespace std;

bool has_prefix(string const& str, string const& pre) {
    return str.compare(0, pre.size(), pre) == 0;
}

// Split the given string around the fist occurrence of the given character.
// If the character is not found, the input string is returned as the first string and "" as the second.
// The outputs are stored in before and after.
void split_at_first(string const& str, char const& c, string & before, string & after) {
    int index = str.find(c);
    if (index == -1) {
        before = str;
        after = "";
    } else {
        before = str.substr(0, index);
        after = str.substr(index + 1, string::npos);
    }
}