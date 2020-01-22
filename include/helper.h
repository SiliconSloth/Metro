using namespace std;

/*
 * Check if a non-terminated char array is equal to a null-terminated char array.
 * The third argument is the length of the first string.
 *
 * Works by checking both strings are the same length (ignoring the terminator)
 * and then comparing the first n characters of each.
 */
#define strnequal(str1, str2, n) ((n) == sizeof(str2)-1 && !strncmp(str1, str2, n))

int parse_pos_int(const string& str);

bool has_prefix(string const& str, string const& pre);

bool has_suffix(string const& str, string const& suff);

bool whitespace_only(const string& s);

void split_at_first(string const& str, char const& c, string & before, string & after);

string read_all(const string& path);

void write_all(const string& text, const string& path);