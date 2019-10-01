#include "pch.h"

using namespace std;

// An option on the command line.
// name: The long-form name of the option, e.g. --help
// contraction: The short name of the option, e.g. -h
// needsValue: Whether this option needs and allows a value associated with it
struct Option {
    string name;
    string contraction;
    bool needsValue = false;
};

const Option ALL_OPTIONS[] = {
        {"help", "h", false},
        {"timeout", "t", true},
        {"force", "f", false}
};
