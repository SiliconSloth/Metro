/*
 * This code describes the format of argument and command passing
 */

#pragma once

// An argument collection
struct Arguments {
    vector<string> positionals;     // The positionals of the command (the ones before flags or options).
    map<string, string> options;    // A map of found options and associated values if any.
    bool hasHelpFlag = false;       // Whether the help flag wss enabled in the arguments.
};

// An option on the command line.
struct Option {
    string name;                    // The long-form name of the option, e.g. --help
    string contraction;             // The short name of the option, e.g. -h
    bool needsValue = false;        // Whether this option needs and allows a value associated with it
    string description;             // Description of option listed in command help
};

// Command which can be run.
struct Command {
    string name;                                    // Command name
    string description;                             // Description of what the command does
    function<void(const Arguments&)> execute;       // Function to execute when function is called
    function<void(const Arguments&)> printHelp;     // Function to execute when function help is called
};

// List of all valid options
// Keep them in alphabetical order (by name) to make help messages easier to read
const Option ALL_OPTIONS[] = {
        {"force", "f", false, "Forces execution of the command ignoring warnings"},
        {"help", "h", false, "Explain how to use command"},
        {"pull", "d", false, "Only pull changes, without pushing changes to remote"},
        {"push", "u", false, "Only push changes, without pulling changes from remote. Requires no conflicts"},
        {"soft", "s", false, "Delete the last commit without reverting changes in the working directory"},
        {"version", "v", false, "Finds the version of Metro being used"}
};

