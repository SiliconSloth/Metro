#include "pch.h"

Command info {
        "info",
        "Show the state of the repo",

        // execute
        [](const Arguments &args) {
            Repository repo = git::Repository::open(".");
            cout << "Current branch: " << metro::current_branch_name(repo) << endl;
            cout << "Merging: " << (metro::merge_ongoing(repo)? "yes" : "no") << endl;
        },

        // printHelp
        [](const Arguments &args) {
            std::cout << "Usage: metro info\n";
        }
};
