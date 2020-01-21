#include "gitwrapper/types.h"

using namespace std;

int parse_pos_int(const string& str);

bool has_prefix(string const& str, string const& pre);

bool has_suffix(string const& str, string const& suff);

bool whitespace_only(const string& s);

void split_at_first(string const& str, char const& c, string & before, string & after);

string read_all(const string& path);

void write_all(const string& text, const string& path);

string time_to_string(git_time time);

void set_text_colour(string colour, void* handle);