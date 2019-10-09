using namespace std;

bool has_prefix(string const& str, string const& pre);

void split_at_first(string const& str, char const& c, string & before, string & after);

string read_all(const string& path);

void write_all(const string& text, const string& path);