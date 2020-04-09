using namespace std;

struct Arguments {
    vector<string> positionals;
    map<string, string> options;
    bool hasHelpFlag = false;
};

// An option on the command line.
// name: The long-form name of the option, e.g. --help
// contraction: The short name of the option, e.g. -h
// needsValue: Whether this option needs and allows a value associated with it
// description: Description of option listed in command help
struct Option {
    string name;
    string contraction;
    bool needsValue = false;
    string description;
};

struct Command {
    string name;
    string description;
    function<void(const Arguments&)> execute;
    function<void(const Arguments&)> printHelp;
};

// List of all valid options
// Keep them in alphabetical order to make help messages easier to read
const Option ALL_OPTIONS[] = {
        {"force", "f", false, "Not yet implemented"},
        {"help", "h", false, "Explain how to use command"},
        {"pull", "d", false, "Only pull changes, without pushing changes to remote"},
        {"push", "u", false, "Only push changes, without pulling changes from remote. Requires no conflicts"},
        {"soft", "s", false, "Delete the last commit without reverting changes in the working directory"}
};

